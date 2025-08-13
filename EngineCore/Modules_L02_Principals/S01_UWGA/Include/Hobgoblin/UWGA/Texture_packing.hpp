// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_TEXTURE_PACKING_HPP
#define UHOBGOBLIN_UWGA_TEXTURE_PACKING_HPP

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/UWGA/Image.hpp>
#include <Hobgoblin/UWGA/Texture.hpp>
#include <Hobgoblin/UWGA/Texture_rect.hpp>

#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

enum class TexturePackingHeuristic {
	BestShortSideFit,
	BestLongSideFit,
	BestAreaFit,
	BottomLeftRule,
	ContactPointRule
};

class TexturePackingError : public TracedRuntimeError {
public:
	using TracedRuntimeError::TracedRuntimeError;
};

std::vector<TextureRect> PackTexture(Texture& texture, 
									 const std::vector<Image*>& images,
                                     TexturePackingHeuristic heuristic, 
									 float* occupancy = nullptr);

} // namespace gr
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>
#include <Hobgoblin/Private/Short_namespace.hpp>

#endif // !UHOBGOBLIN_UWGA_TEXTURE_PACKING_HPP
