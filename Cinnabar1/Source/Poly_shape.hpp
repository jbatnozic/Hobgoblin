// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

#include <vector>
#include <span>

namespace cinnabar {

namespace hg = ::jbatnozic::hobgoblin;

class PolyShape {
public:
    explicit PolyShape(hg::PZInteger aVertexCount);

    hg::PZInteger getVertexCount() const;

    void               setAnchor(hg::math::Vector2d aAnchor);
    hg::math::Vector2d getAnchor() const;

    void             setRotation(hg::math::AngleF aAnchor);
    hg::math::AngleF getRotation() const;

    void                      setRawVertexAt(hg::PZInteger aIndex, hg::math::Vector2f avertex);
    const hg::math::Vector2f& getRawVertexAt(hg::PZInteger aIndex) const;

    //! \brief Calculate the offset of the baricenter relative to the anchor.
    //!
    //! Given the current set of raw vertices and assuming rotation==0 and equal density, calculate
    //! and return the offset of the baricenter relative to the currently set anchor.
    //!
    //! \note add this value to every raw vertex to make the baricenter aligned with the anchor.
    hg::math::Vector2f calculateBaricenterOffset() const;

    // Outputs

    enum State {
        DIRTY,          //! Output vertices needs recalculating
        READY_RELATIVE, //! Output vertices are ready to use (and have values relative to anchor)
        READY_ABSOLUTE, //! Output vertices are ready to use (and have values in absolute coordinates)
    };

    State getState() const;

    void recalculateRelative();

    void recalculateAbsolute();

    // note: move the anchor and all output vertices, preserving current state
    void move(hg::math::Vector2d aDelta);

    // note: if `getState() == DIRTY`, results are unusable
    std::span<hg::math::Vector2d> getOutputVertices() const;

private:
    hg::math::Vector2d _anchor;

    //! Always relative to the anchor, and without any rotation taken into account
    hg::util::CompressedSmallVector<hg::math::Vector2f, 12> _rawVertices;

    hg::math::AngleF _rotation;

    State _state = DIRTY;

    std::vector<hg::math::Vector2d> _outputVertices;
};

} // namespace cinnabar