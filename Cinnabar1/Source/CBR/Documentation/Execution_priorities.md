# Execution priorities

This document gives an overview of the order in which the important things happen during a single
iteration of Cinnabar1's main loop - per QAO step and per QAO event.

Exact priority values live in `Engine.hpp` (`PRIORITY_*` macros) and are subject to change, so they
are not quoted here; only the **relative ordering** matters. Remember: in QAO, events of instances
with **higher** execution priority are executed **before** those with lower priority.

## Overall ordering of objects

From first to last (within every event):

| Block                | Object                                     | Notes                                           |
|----------------------|--------------------------------------------|-------------------------------------------------|
| First block managers | `SyncedVarmapManager`                      | Just before `NetworkingManager`.                |
|                      | `NetworkingManager`                        | Receiving only; sending is done by its send helper (see below). |
|                      | `InputSyncManager`                         | Not present in DEV mode.                        |
|                      | `LobbyBackendManager`                      |                                                 |
|                      | `LobbyFrontendManager`                     |                                                 |
|                      | `AuthorizationManager`                     | Last of the 'first block'.                      |
| Gameplay             | `AuxiliaryGameplayManager`                 | Has no events.                                  |
|                      | **Gameplay entities**                      | Ships, ship controllers, attachables, asteroids etc. |
|                      | `OverworldManager`                         | After the entities. Not present in CLIENT mode (for now). |
| Last block managers  | `InteractivityManager`                     | First of the 'last block'.                      |
|                      | `NetworkingManager`'s send helper          | Separate QAO object owned by `NetworkingManager`, with its own priority. Only implements `_eventEndUpdate()`. |
|                      | `WindowManager`                            |                                                 |
|                      | `MainGameFlowManager`, `EventLoopTimingReporter` | Same priority (relative order unspecified); last. |

**Gameplay entities** always execute between the last 'first block' manager (`AuthorizationManager`)
and the first 'last block' manager (`InteractivityManager`). This means that, in every event, all
networking/lobby/auth bookkeeping has already been done before entities run, and all UI/window
bookkeeping is done after them.

---

## Step: Update

### 1. PRE_UPDATE (`_eventPreUpdate()`)

> Event for code that absolutely must execute before anything else. You should rarely need this event.

1. `NetworkingManager` - adjusts telemetry data (starts a new telemetry cycle).
2. *Gameplay entities* - (if they implement it).
3. `InteractivityManager` - clears PNCS (Point n' Click System) data (registered clickables and
   the cached mouse position).
4. `WindowManager` - **(Normal mode only)** processes the window events that were polled during the
   previous `DISPLAY` event:
   - Every event is first fed to RmlUi. **RmlUi event callbacks are executed right here**
     (synchronously) - this is where e.g. `LobbyFrontendManager`'s button handlers (lock in, reset,
     move up/down, kick) run, even though that manager doesn't implement `_eventPreUpdate()` itself.
   - Events not consumed by RmlUi are recorded into the input tracker; this becomes the input that
     `WindowManager::getInput()` returns for the rest of the iteration.
   - Handles window close (stops the context) and resize.
5. `EventLoopTimingReporter` - accumulates timing info and periodically logs a performance report.

**Consequence:** anything that reads `WindowManager::getInput()` during `PRE_UPDATE` with a higher
priority than `WindowManager` (i.e. everything) sees **stale** input from the previous iteration.
Read input in `BEGIN_UPDATE` or later.

### 2. BEGIN_UPDATE (`_eventBeginUpdate()`)

> Objects prepare for the upcoming Update(s) - actions like gathering user input, reading network
> messages and similar are appropriate for this event. Objects should interact as little as possible here.

1. `NetworkingManager` - receives and processes packets and messages (`RN_UpdateMode::Receive`).
   All RigelNet handlers run here. On clients, this is when synchronized (dummy) objects are created,
   updated and destroyed by the incoming sync messages. Networking events are dispatched to the
   listeners (`SyncedVarmapManager`, `LobbyBackendManager`, `MainGameFlowManager`...).
2. `InputSyncManager` - **(Host only)** applies the received client input.
3. `LobbyBackendManager` - adjusts the lobby based on the received data (on the host, this includes
   scanning for new or disconnected players); enqueues lobby events.
4. `LobbyFrontendManager` - polls the events from `LobbyBackendManager` and adjusts the frontend
   (shows the lobby GUI when the lobby changes or gets locked in).
5. `AuthorizationManager` - **(Host only)** checks whether the authorized player is still connected
   and authorizes another one if not.
6. *Gameplay entities* - prepare for the update. **Clickable entities must register themselves with
   `InteractivityManager::pushClickableObject()` here** (this is the only event where that is allowed).
7. `InteractivityManager` -
   - **(Host only)** gathers the remote players' controls from `InputSyncManager`;
   - **(Client only)** dispatches PNCS events (to the entities registered in step 6), then puts the
     local player's controls into `InputSyncManager` (to be uploaded in `UPDATE_1`);
   - **(Dev only)** dispatches PNCS events.

### 3. UPDATE_1 (`_eventUpdate1()`)

> These are the "main" events where most of the work happens. Ideally, all objects prepared themselves
> during `PRE_UPDATE` and `BEGIN_UPDATE`, so they are free to interact with each other (including
> creating new objects or deleting existing ones).

1. `InputSyncManager` - **(Client only)** composes the upload of the recorded input (actually sent
   in `END_UPDATE`).
2. `LobbyFrontendManager` - **(Client only)** shows/hides the lobby GUI (on the `L` key).
3. *Gameplay entities* - main gameplay logic. Synchronized dummy objects (clients) first advance their
   state schedulers here (and die if their death counter expired).
4. `OverworldManager` - simulates the Chipmunk (Alvin) physics space. Because this runs **after** the
   entities' `UPDATE_1`, forces/velocities that entities set in `UPDATE_1` take effect in the same
   iteration, and the resulting positions are only visible to entities from `UPDATE_2` onward.

### 4. UPDATE_2 (`_eventUpdate2()`)

> (Same as `UPDATE_1`.)

1. *Gameplay entities* - can react to the results of the physics simulation.

No managers implement this event.

### 5. END_UPDATE (`_eventEndUpdate()`)

> In this event, we process the results of the preceding `Update1/2` events. In a multiplayer game,
> this would be where the server uploads updates to clients. You can also view it as a preparation for
> the `Draw*` events. Like in `BEGIN_UPDATE`, interactions between objects should be minimal.

1. `SyncedVarmapManager` - **(Host only)** composes the state updates of the varmap.
2. `InputSyncManager` - **(Host only)** advances all state schedulers (of the received input).
3. *Gameplay entities* - (if they implement it).
4. `NetworkingManager`'s send helper - sends all composed messages (`RN_UpdateMode::Send`).
   **(Host only)** Before that, it updates the synchronized object registry and composes the state
   updates (sync create/update/destroy) of all synchronized objects.

**Consequence:** because sending happens after the gameplay entities' `END_UPDATE`, everything
composed during the whole Update step (including the client input composed by `InputSyncManager` in
`UPDATE_1` and any messages composed by entities in `END_UPDATE`) is sent in the **same** iteration.
Likewise, the state of synchronized objects is synced as it is at the end of their `END_UPDATE`.
Only messages composed after the send helper (e.g. in `POST_UPDATE` or in the Draw/Display steps) are
deferred to the next iteration.

### 6. POST_UPDATE (`_eventPostUpdate()`)

> This is another rarely needed event, sometimes used to 'clean up' the results of the whole Update
> step. Again, no object interactions are expected here.

1. `LobbyBackendManager` - clears its event queue (events not polled during this Update step are lost).
2. *Gameplay entities* - (if they implement it).

---

## Step: Draw

**NOTE:** The Draw step is not executed at all on the headless server (its Draw EXECON level is set to
`META_EXECUTE_NONE`), and in general it isn't guaranteed to run after every Update step.

### 7. PRE_DRAW (`_eventPreDraw()`)

> This is the analogue to `PRE_UPDATE` but for the drawing step; this event is used to prepare the game
> state for the drawing step, if needed (for example: clear the target window). No actual drawing should
> be performed in this event.

1. *Gameplay entities* - (if they implement it).
2. `InteractivityManager` - dispatches mouseover/hover PNCS events (to the foreground-most clickable
   under the cursor).
3. `WindowManager` - **(Normal only)** clears the MRT (main render texture).

**NOTE:** the MRT is cleared **after** the entities' `PRE_DRAW`, so nothing drawn in `PRE_DRAW` will
survive.

### 8. DRAW_1 (`_eventDraw1()`)

> In this event the game world is drawn. You can have each object resposible for drawing its own state,
> or have one object that draws everything. Both (`DRAW_1` and `DRAW_2`) events are functionally the
> same, but because things drawn later appear on top of those drawn before, it can sometimes be useful
> to have 2 separate drawing events so it's easier to order everything properly.

1. *Gameplay entities* - draw the game world.

No managers implement this event.

### 9. DRAW_2 (`_eventDraw2()`)

> (Same as `DRAW_1`.)

1. *Gameplay entities* - draw the game world (on top of `DRAW_1`).
2. `WindowManager` - **(Normal only)** clears the window and draws the MRT onto it.

**NOTE:** since the MRT is drawn onto the window at the end of `DRAW_2`, anything drawn onto the MRT
later than that won't appear in this frame.

### 10. DRAW_GUI (`_eventDrawGUI()`)

> A GUI always goes on top of everything else, so it's natural to draw it last. (Also, it's worth noting
> that a GUI is usually drawn relative to the 'window' coordinate system and not the 'game world'
> coordinate system.)

1. `LobbyFrontendManager` - **(Client only)** updates the lobby's RmlUi data model with the current
   lobby state (it doesn't draw anything directly).
2. *Gameplay entities* - (if they implement it) draw their GUI elements / update RmlUi data models.
3. `WindowManager` - **(Normal only)** updates and renders the RmlUi context (all RmlUi documents);
   marks the frame as ready to be displayed.

### 11. POST_DRAW (`_eventPostDraw()`)

> No more actual drawing should be performed after `DRAW_GUI`, but `POST_DRAW` can be used to clean up
> after the drawing step if needed.

1. *Gameplay entities* - (if they implement it).

No managers implement this event.

---

## Step: Display

### 12. DISPLAY (`_eventDisplay()`)

> Usually you'll only have two predefined actions in this event: 1) display everything that was drawn
> onto the screen; 2) if there is still time until the expected start of the next iteration, the program
> can sleep or wait for V-Sync. (Point 2 can be skipped in games with a variable timestep, though QAO was
> really made with a constant timestep in mind.)

1. *Gameplay entities* - (if they implement it).
2. `WindowManager` -
   - **(Normal only)** displays the window (if the frame is ready and the framerate limit allows it),
     then **polls window events** and stores them to be processed in the next `PRE_UPDATE`;
   - sleeps until the next step (if busy wait prevention is enabled) - also in headless mode.
3. `MainGameFlowManager` - adjusts the global EXECON levels: once per iteration, raises the Update
   (and, if not headless, Draw) EXECON level by one, until `META_EXECUTE_ALL` is reached. The new
   levels take effect from the next iteration on.
