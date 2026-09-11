// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/ChipmunkPhysics.hpp>
#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Canvas.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>
#include <Hobgoblin/Utility/Stream_input.hpp>
#include <Hobgoblin/Utility/Stream_output.hpp>

#include <span>
#include <vector>

namespace cinnabar {

namespace hg = ::jbatnozic::hobgoblin;

class PolyShape {
public:
    explicit PolyShape(hg::PZInteger      aVertexCount = 0,
                       hg::math::Vector2d aAnchor      = {0.0, 0.0},
                       hg::math::AngleF   aRotation    = hg::math::AngleF::zero());

    void          setVertexCount(hg::PZInteger aVertexCount);
    hg::PZInteger getVertexCount() const;

    void               setAnchor(hg::math::Vector2d aAnchor);
    hg::math::Vector2d getAnchor() const;

    void             setRotation(hg::math::AngleF aRotation);
    hg::math::AngleF getRotation() const;

    void                      setRawVertexAt(hg::PZInteger aIndex, hg::math::Vector2f avertex);
    const hg::math::Vector2f& getRawVertexAt(hg::PZInteger aIndex) const;

    //! \brief Same as `setRawVertexAt` but without bounds checking on `aIndex`.
    //! \warning behaviour is undefined if `aIndex` is out of bounds.
    void setRawVertexAtUnchecked(hg::PZInteger aIndex, hg::math::Vector2f avertex);

    //! \brief Same as `getRawVertexAt` but without bounds checking on `aIndex`.
    //! \warning behaviour is undefined if `aIndex` is out of bounds.
    const hg::math::Vector2f& getRawVertexAtUnchecked(hg::PZInteger aIndex) const;

    //! \brief Calculate the offset of the baricenter relative to the anchor.
    //!
    //! Given the current set of raw vertices and assuming rotation==0 and equal density, calculate
    //! and return the offset of the baricenter relative to the currently set anchor.
    //!
    //! \note subtract this value from every raw vertex to make the baricenter aligned with the anchor.
    hg::math::Vector2f calculateBaricenterOffset() const;

    //! \brief Returns the index of the raw vertex with the shortest distance from (0, 0) to it.
    //! \pre the shape must have at least one vertex.
    hg::PZInteger getShortestRawVertexIndex() const;

    //! \brief Returns the index of the raw vertex with the longest distance from (0, 0) to it.
    //! \pre the shape must have at least one vertex.
    hg::PZInteger getLongestRawVertexIndex() const;

    // Outputs

    enum State {
        DIRTY,          //! Output vertices needs recalculating
        READY_RELATIVE, //! Output vertices are ready to use (and have values relative to anchor)
#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
        READY_ABSOLUTE, //! Output vertices are ready to use (and have values in absolute coordinates)
#endif
    };

    //! \brief Get the current state of the poly shape.
    State getState() const;

    //! TODO(add description)
    void recalcRel();

#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
    //! TODO(add description)
    void recalcAbs();
#endif

    // note: move the anchor and all output vertices, preserving current state
    void move(hg::math::Vector2d aDelta);

    // note: if `getState() == DIRTY`, results are unusable
    std::span<const hg::math::Vector2d> getOutputVertices() const;

    //! \brief Returns the output vertices reinterpreted as an array of `cpVect` (Chipmunk Physics).
    //!
    //! The returned pointer aliases the same buffer as `getOutputVertices()`; use `getVertexCount()`
    //! for the number of elements. This is sound because `cpVect` and `hg::math::Vector2d` are
    //! guaranteed to share the same size, alignment and layout (enforced via static_asserts).
    //!
    //! \note if `getState() == DIRTY`, results are unusable.
    const cpVect* getOutputVerticesAsCpVect() const;

#ifdef CINNABAR_POLYSHAPE_ENABLE_ABSOLUTE
    //! \brief Check if a point intersects with the poly shape.
    //!
    //! \param aPoint the point to check. This point is interpreted as absolute coordinates
    //!               in the game world.
    //!
    //! \warning the status of the poly shape must be `READY_ABSOLUTE`; otherwise the result of this
    //!          function call is unspecified (throws, asserts, or returns garbage values).
    bool intersectsWithPointAbs(hg::math::Vector2d aPoint) const;
#endif

    //! \brief Check if a point intersects with the poly shape.
    //!
    //! \param aPoint the point to check. This point s interpreted as relative coordinates
    //!               relative to the anchor of the poly shape.
    //!
    //! \warning the status of the poly shape must be `READY_RELATIVE`; otherwise the result of this
    //!          function call is unspecified (throws, asserts, or returns garbage values).
    bool intersectsWithPointRel(hg::math::Vector2d aPoint) const;

    float getDistanceToFarthestRawVertexSquared() const {
        return _distanceToFarthestRawVertexSquared;
    }

    void debugDraw(hg::uwga::Color               aColor,
                   hg::uwga::Canvas&             aCanvas,
                   const hg::uwga::RenderStates& aRenderStates = hg::uwga::RENDER_STATES_DEFAULT) const;

    void writeRawVerticesToStream(hg::util::OutputStream& aOStream) const;

    void readRawVerticesFromStream(hg::util::InputStream& aIStream);

private:
    hg::math::Vector2d _anchor;

    //! Always relative to the anchor, and without any rotation taken into account
    hg::util::CompressedSmallVector<hg::math::Vector2f, 12> _rawVertices;

    hg::math::AngleF _rotation;

    std::vector<hg::math::Vector2d> _outputVertices;

    float _distanceToFarthestRawVertexSquared = 0.f;

    State _state = DIRTY;

    void _recalcDistanceToFarthestRawVertex();
};

} // namespace cinnabar