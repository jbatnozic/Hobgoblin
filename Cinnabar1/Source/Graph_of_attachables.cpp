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
        // TODO: temporary implementation!!! (needs to find first empty spot instead and put it there)
        _nodes.push_back(std::make_unique<Node>(*this, aAttachable));
        aAttachable._myNode = _nodes.back().get();
        return _nodes.size() - 1;
    }
}

const GraphOfAttachables::Node* GraphOfAttachables::getNode(hg::PZInteger aIndex) const {
    const auto szIdx = hg::pztos(aIndex);
    if (szIdx >= _nodes.size()) {
        return nullptr;
    }
    return _nodes[szIdx].get();
}

} // namespace cinnabar
