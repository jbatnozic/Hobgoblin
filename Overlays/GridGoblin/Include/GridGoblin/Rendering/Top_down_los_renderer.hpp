// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Hobgoblin/Math.hpp>
#include <Hobgoblin/UWGA/Color.hpp>
#include <Hobgoblin/UWGA/Render_texture.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>
#include <Hobgoblin/Utility/Grids.hpp>

#include <GridGoblin/Positional/Position_in_world.hpp>
#include <GridGoblin/Rendering/Visibility_provider.hpp>
#include <GridGoblin/World/World.hpp>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = jbatnozic::hobgoblin;

class TopDownLineOfSightRenderer : public VisibilityProvider {
public:
    enum Purpose {
        FOR_TOPDOWN,
        FOR_DIMETRIC
    };

    TopDownLineOfSightRenderer(const World& aWorld, hg::PZInteger aTextureSize, Purpose aPurpose);

    ~TopDownLineOfSightRenderer();

    void start(PositionInWorld    aPosInView,
               hg::math::Vector2f aViewSize,
               PositionInWorld    aLineOfSightOrigin,
               float              aPadding);

    void render();

    std::optional<bool> testVisibilityAt(PositionInWorld aPos) const override;

    //! For debug purposes only.
    const hg::uwga::Texture& __ggimpl_getTexture(hg::math::Vector2f* aRecommendedScale = nullptr) const;

private:
    const World& _world;

    float              _sizeMultiplier;
    float              _recommendedScale = 1.f;
    PositionInWorld    _losOrigin;
    hg::math::Vector2d _viewCenterOffset;

    //! Texture to which visibility is rendered.
    std::unique_ptr<hg::uwga::RenderTexture> _renderTexture;

    //! Width and Height of _renderTexture (it's always a square).
    hg::PZInteger _textureSize;

    //! Buffer which will hold the contents of _renderTexture in RAM
    //! (in RGBA format), for fast access for purposes of getColorAt().
    std::vector<std::uint8_t> _textureRamBuffer;

    //! Names of OpenGL Pixel Buffer Objects.
    std::array<unsigned int, 2> _pboNames;

    //! Counter used to know which PBO to write to and which PBO to read from.
    //! - In even-numbered steps, we start writing to pbo[0] and read from pbo[1],
    //! - In odd-numbered steps, we start writing to pbo[1] and read from pbo[0].
    unsigned int _stepCounter = static_cast<unsigned int>(-1);

    void _renderOcclusion();
};

} // namespace gridgoblin
} // namespace jbatnozic
