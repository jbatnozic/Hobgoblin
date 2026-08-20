// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Graph_of_attachables.hpp"

#include <Hobgoblin/HGExcept.hpp>

#include <Ship_attachable.hpp>

namespace cinnabar {

GraphOfAttachables::Node::Node(GraphOfAttachables& aParentGraph, ShipAttachable& aAssociatedAttachable)
    : parentGraph{aParentGraph}
    , associatedAttachable{aAssociatedAttachable} {}

hg::PZInteger GraphOfAttachables::insert(ShipAttachable& aAttachable) {
    HG_VALIDATE_PRECONDITION(aAttachable._myNode == nullptr);

    if (_nodes.empty()) {
        _nodes.push_back(std::make_unique<Node>(*this, aAttachable));
        aAttachable._myNode = _nodes[0].get();
        return 0;
    } else {
        HG_NOT_IMPLEMENTED("TODO");
    }
}

} // namespace cinnabar
