// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

#include <memory>
#include <vector>

namespace cinnabar {

class ShipAttachable;

class GraphOfAttachables {
public:
    struct Node {
        GraphOfAttachables& parentGraph;          //!< The graph that owns this node
        ShipAttachable&     associatedAttachable; //!< The Node's sibling ShipAttachable

        hg::util::CompressedSmallVector<Node*, 3> adjacentNodes;

        bool visited = false;

        Node(GraphOfAttachables& aParentGraph, ShipAttachable& aAssociatedAttachable);
    };

    void insert(ShipAttachable& aAttachable);

private:
    friend class ShipAttachable;

    std::vector<std::unique_ptr<Node>> _nodes;
};

} // namespace cinnabar
