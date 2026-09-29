# TITLE

## Recommended priorities

- SyncedVarmapService* (highest)
- NetworkingService*
- LobbyBackendService*
- LobbyFrontendService* (user-provided)
- AuthorizationService*

- GameplayService (user-provided)
- InputSyncService
- Various gameplay objects (user-provided)

- WindowService (lowest)

## Breakdown by event

### PRE_UPDATE
- NetworkingService prepares itself to record telemetry
- USER WINDOW

### BEGIN_UPDATE
- NetworkingService receives messages, calls handlers
- LobbyBackendService ... ???
- LobbyFrontendService ... ???
- AuthorizationService ... ???
- USER WINDOW

### UPDATE_1
- LobbyFrontendService ... ???
- << game objects delete selves >>
- USER WINDOW

### UPDATE_2
- USER WINDOW

### END_UPDATE
- SyncedVarmapService composes messages
- NetworkingService sends messages
- USER WINDOW

### POST_UPDATE
- LobbyBackendService ... ???
- USER WINDOW

### PRE_DRAW
- USER WINDOW
- WindowService clears MRT (main render texture)

### DRAW_1
- USER WINDOW

### DRAW_2
- USER WINDOW
- WindowService clears the window, draws MRT in it

### DRAW_GUI
- USER WINDOW

### POST_DRAW
- USER WINDOW

### DISPLAY
- USER WINDOW
- WindowService displays window
