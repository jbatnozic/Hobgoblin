// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Engine.hpp>

#include <vector>

namespace cinnabar {

//! Enumerates possible orientations of an attachable's interior world slice relative to a ship
//! controller (observed from the ship controller's own coordinate system).
enum class RelativeIWSliceOrientation : char {
    //! The IW slice's rotation compared to the ship controller's rotation is not valid,
    //! meaning that an attachment cannot be made without changes to their relative rotations.
    INVALID,

    //! The IW slice's rotation compared to the ship controller's rotation is 0 degrees (in
    //! other words, in the global coordinate system, their rotations are the same).
    ROT_ALIGNED, 

    //! The IW slice is rotated 90 degrees counter-clockwise compared to the ship controller.
    ROT_90DEG_CCW,

    //! The IW slice is rotated 180 degrees counter-clockwise compared to the ship controller.
    ROT_180DEG_CCW,

    //! The IW slice is rotated 270 degrees counter-clockwise compared to the ship controller.
    ROT_270DEG_CCW,

    //! The orientation is not relevant for the attachment evaluation (they can be attached
    //! regardless of their relative rotation).
    NOT_RELEVANT,
};

struct AttachmentEvaluation {
    enum StatusBits : char {
        ALL_VALID           = 0x00,

        INVALID_ORIENTATION = 0x01,
        INVALID_POS         = 0x02,
        OVERLAP             = 0x04,
        NO_CONTACT          = 0x08,
        OUT_OF_BOUNDS       = 0x10,
    };

    char status;

    //! if `orientation` == INVALID then status bit INVALID_ORIENTATION will be set.
    RelativeIWSliceOrientation orientation;

    //! If the value of `orientation` is `INVALID`, this field contains a rotation hint: the smallest
    //! possible rotation adjustment that, if applied to the attachable, would bring its interior
    //! world slice into place, making its orientation valid.
    //! For any other value of `orientation`, the value of this field is undefined and should not be
    //! used.
    hg::math::AngleF rotationHint;

    //! Tells the coordinates of a cell in the ship'controller's coordinate system to which the
    //! top-left cell of the attachable's interior world slice maps.
    //! \warning the coordinates are relative to the ship controller's center! To map to cell positions
    //!          in the ship's interior world, offset the value by `ShipController::CELL_COUNT_X/Y / 2`.
    //! \warning if `INVALID_POS` bit is set in the `status` word, the value of this field is undefined
    //!          and should not be used.
    hg::math::Vector2i topLeftCellMapping;

    hg::math::Vector2f anchorAdjustmentHint;

    struct BondStrength {
        std::int16_t attachableId;
        std::int16_t attachmentPointCount;
    };

    std::vector<BondStrength> bonds;
};

} // namespace cinnabar
