// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Math.hpp>

#include <cstdint>
#include <functional>

namespace cinnabar {

class InteractivityManager : public spe::ContextComponent {
public:
    SPEMPE_CTXCOMP_TAG("cinnabar::InteractivityManager");

    //! \brief Register an object as a candidate to receive mouse-over and click events this frame.
    //!
    //! During the `QAO_Event::BEGIN_UPDATE` event, every clickable object that's in view's frustum 
    //! should call this function to enter itself into the interactivity manager's per-frame candidate
    //! list. Once all candidates have registered, the manager selects a single "foreground-most" object
    //! and forwards a `HandlePNCSEvent` message to it, carrying the current mouse button state
    //! (down/up for the left and right buttons) alongside its `aUserData`.
    //!
    //! Selection works in two stages:
    //! -# Candidates are ranked by `aFinegrainedPriority`: the object with the lowest priority value is
    //!    considered to be drawn on top (least obscured) and is therefore preferred.
    //! -# Starting from the top candidate, the manager evaluates `aFullMouseOverCheck` and picks the
    //!    first object for which it returns `true`. This lets objects supply a cheap approximate test up
    //!    front and defer the exact (and possibly expensive) hit test until it is actually needed.
    //!
    //! The candidate list is rebuilt from scratch every frame, so this must be called on every frame in
    //! which the object wants to be interactive.
    //!
    //! \param aClickableId QAO ID of the clickable object; this is the object that will receive the
    //!                     `HandlePNCSEvent` message if it is selected.
    //! \param aFinegrainedPriority tie-breaking priority used to determine which overlapping object is in
    //!                             the foreground. Lower values are treated as being closer to the front.
    //! \param aUserData arbitrary user data passed back verbatim through the `HandlePNCSEvent` message
    //!                  (if the message is sent to this object).
    //! \param aQuickMouseOverCheck cheap, possibly approximate hit test evaluated immediately; if it
    //!                             returns `false` for the current mouse position the call is a no-op
    //!                             and the object is not registered. May report false positives.
    //! \param aFullMouseOverCheck exact hit test evaluated lazily during selection; only the candidate
    //!                            ultimately chosen is required to pass this check. Both callbacks receive
    //!                            the mouse position in world coordinates.
    //!
    //! \warning DO NOT call this outside of the `QAO_Event::BEGIN_UPDATE` event!
    virtual void pushClickableObject(QAO_GenericId                           aClickableId,
                                     int                                     aFinegrainedPriority,
                                     std::intptr_t                           aUserData,
                                     std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
                                     std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) = 0;
};

using MInteractivity = InteractivityManager;

} // namespace cinnabar
