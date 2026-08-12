// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Math.hpp>

#include <cstdint>
#include <functional>

namespace cinnabar {

class InteractivityManagerInterface : public spe::ContextComponent {
public:
    SPEMPE_CTXCOMP_TAG("cinnabar::InteractivityManager");

    virtual void pushClickableObject(QAO_GenericId                           aClickableId,
                                     int                                     aFinegrainedPriority,
                                     std::intptr_t                           aUserData,
                                     std::function<bool(hg::math::Vector2d)> aQuickMouseOverCheck,
                                     std::function<bool(hg::math::Vector2d)> aFullMouseOverCheck) = 0;

    //! \brief push an object to the top of the clickable object stack.
    //!
    //! During the `QAO_Event::BEGIN_UPDATE` event, all clickable objects that detect that they intersect
    //! with the mouse cursor should call this function. Because objects with lower execution priority
    //! are updated (and drawn) after those with higher priority, the "foreground-most" object that the
    //! player sees will end up on top of the stack. At the end of the `BEGIN_UPDATE` event, this object
    //! (if any) will receive the `HandleLeftClick` and/or `HandleRightClick` QAO messages if the left
    //! and right mouse buttons, respectively, have been pressed.
    //!
    //! \param aClickableId QAO ID of the clickable object.
    //! \param aUserData arbitrary user data to be passed back through the `Handle*Click` message
    //!                  (if the message is sent to this object).
    //!
    //! \warning DO NOT call this outside of the `QAO_Event::PRE_UPDATE` event!
    // virtual void pushClickableObject(QAO_GenericId aClickableId, std::intptr_t aUserData = 0) = 0;
};

using MInteractivity = InteractivityManagerInterface;

} // namespace cinnabar
