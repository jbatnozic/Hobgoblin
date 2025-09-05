// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <SPeMPE/Utility/Window_frame_input_view.hpp>

#include <SPeMPE/Utility/Window_input_tracker.hpp>

namespace jbatnozic {
namespace spempe {

WindowFrameInputView::WindowFrameInputView(hg::NeverNull<const detail::WindowInputTracker*> aTracker)
    : _tracker{*aTracker} {}

///////////////////////////////////////////////////////////////////////////
// UNIVERSAL                                                             //
///////////////////////////////////////////////////////////////////////////

bool WindowFrameInputView::checkPressed(hg::in::UniversalInputEnum aKeyOrButton, Mode aMode) const {
    return _tracker.checkPressed(aKeyOrButton, aMode);
}

bool WindowFrameInputView::checkReleased(hg::in::UniversalInputEnum aKeyOrButton, Mode aMode) const {
    return _tracker.checkReleased(aKeyOrButton, aMode);
}

///////////////////////////////////////////////////////////////////////////
// KEYBOARD-SPECIFIC                                                     //
///////////////////////////////////////////////////////////////////////////

// typed txt..

///////////////////////////////////////////////////////////////////////////
// MOUSE-SPECIFIC                                                        //
///////////////////////////////////////////////////////////////////////////

bool WindowFrameInputView::checkMouseMoved() const {
    return _tracker.checkMouseMoved();
}

hg::math::Vector2d WindowFrameInputView::getViewRelativeMousePos(
    const hobgoblin::uwga::View& aView) const //
{
    return _tracker.getViewRelativeMousePos(aView);
}

hg::math::Vector2f WindowFrameInputView::getWindowRelativeMousePos() const {
    return _tracker.getWindowRelativeMousePos();
}

float WindowFrameInputView::getVerticalMouseWheelScroll() const {
    return _tracker.getVerticalMouseWheelScroll();
}

float WindowFrameInputView::getHorizontalMouseWheelScroll() const {
    return _tracker.getHorizontalMouseWheelScroll();
}

bool WindowFrameInputView::checkMouseEnteredWindow() const {
    return _tracker.checkMouseEnteredWindow();
}

bool WindowFrameInputView::checkMouseLeftWindow() const {
    return _tracker.checkMouseLeftWindow();
}

bool WindowFrameInputView::checkMouseInWindow() const {
    return _tracker.checkMouseInWindow();
}

} // namespace spempe
} // namespace jbatnozic
