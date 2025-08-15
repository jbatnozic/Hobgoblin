// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/UWGA/Texture_packing.hpp>

#include "MaxRectsBinPack/Max_rects_bin_pack.hpp"

#include <cassert>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

namespace {

rbp::MaxRectsBinPack::FreeRectChoiceHeuristic ConvertHeuristic(TexturePackingHeuristic h) {
    switch (h) {
    case TexturePackingHeuristic::BEST_SHORT_SIDE_FIT:
        return rbp::MaxRectsBinPack::RectBestShortSideFit;
    case TexturePackingHeuristic::BEST_LONG_SIDE_FIT:
        return rbp::MaxRectsBinPack::RectBestLongSideFit;
    case TexturePackingHeuristic::BEST_AREA_FIT:
        return rbp::MaxRectsBinPack::RectBestAreaFit;
    case TexturePackingHeuristic::BOTTOM_LEFT_RULE:
        return rbp::MaxRectsBinPack::RectBottomLeftRule;
    case TexturePackingHeuristic::CONTACT_POINT_RULE:
        return rbp::MaxRectsBinPack::RectContactPointRule;
    default:
        assert(false && "Unreachable");
    }
}

} // namespace

std::vector<TextureRect> PackTexture(Texture&                   texture,
                                     const std::vector<Image*>& images,
                                     TexturePackingHeuristic    heuristic,
                                     float*                     occupancy) {
    // Create texture packer:
    const auto           textureSize = texture.getSize();
    rbp::MaxRectsBinPack packer{textureSize.x, textureSize.y};

    // Get sizes of images to pack:
    std::vector<rbp::RectSize> imageSizes;
    for (const auto* image : images) {
        const auto imageSize = image->getSize();
        imageSizes.push_back(rbp::RectSize{imageSize.x, imageSize.y});
    }

    // Run packing algorithm:
    const std::vector<rbp::Rect> results = packer.Insert(imageSizes, false, ConvertHeuristic(heuristic));

    std::vector<TextureRect> textureRects;
    textureRects.reserve(images.size());
    for (std::size_t i = 0; i < images.size(); i += 1) {
        const auto& image = *images[i];
        const auto& rect  = results[i];

        texture.update(image, rect.x, rect.y);
        textureRects.emplace_back(rect.x, rect.y, rect.width, rect.height);
    }

    // Output values:
    if (occupancy != nullptr) {
        *occupancy = packer.Occupancy();
    }

    return textureRects;
}

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
