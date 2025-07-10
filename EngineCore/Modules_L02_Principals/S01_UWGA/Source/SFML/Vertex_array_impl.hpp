// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_VERTEX_ARRAY_IMPL_HPP
#define UHOBGOBLIN_UWGA_SFML_VERTEX_ARRAY_IMPL_HPP

#include <Hobgoblin/Common.hpp>
#include <Hobgoblin/UWGA/Vertex_array.hpp>

#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLVertexArrayImpl : public VertexArray {
public:
    SFMLVertexArrayImpl(PZInteger aLength);

    ~SFMLVertexArrayImpl() override = default;

    // MARK: Primitive type

    void setPrimitiveType(PrimitiveType aPrimitiveType) override;

    PrimitiveType getPrimitiveType() const override;

    // MARK: Length

    void setLength(PZInteger aLength) override {
        _vertices.resize(pztos(aLength));
    }

    PZInteger getLength() const override {
        return stopz(_vertices.size());
    }

    // MARK: Vertices

    void setVertex(PZInteger aIndex, const Vertex& aVertex) override;

    Vertex getVertex(PZInteger aIndex) const override {
        const auto& vertex = _vertices[pztos(aIndex)];
        // return Vertex{vertex.position, vertex.color, vertex.texCoords};
        return {}; // TODO
    }

    // MARK: Misc.

    void setAnchor(math::Vector2d aAnchor) override;

    math::Vector2d getAnchor() const override;

    void drawOnto(Canvas& aCanvas, const RenderStates& aRenderStates) const override;

private:
    std::vector<sf::Vertex> _vertices;
    sf::PrimitiveType       _primitiveType;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_VERTEX_ARRAY_IMPL_HPP
