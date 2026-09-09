// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Graph_of_attachables.hpp"

#include <Hobgoblin/HGExcept.hpp>

#include <Ship_attachable.hpp>

namespace cinnabar {

GraphOfAttachables::Node::Node(GraphOfAttachables& aParentGraph, ShipAttachable& aAssociatedAttachable)
    : parentGraph{aParentGraph}
    , associatedAttachable{aAssociatedAttachable} {}

GraphOfAttachables::Node::AdjacentNode::AdjacentNode(Node* aNode, std::shared_ptr<Bond> aBond)
    : nodePtr{aNode}
    , bond{std::move(aBond)} {}

void GraphOfAttachables::init(ShipController& aController) {
    _shipController = &aController;
}

std::int16_t GraphOfAttachables::insertInitialAttachable(ShipAttachable& aAttachable) {
    HG_VALIDATE_PRECONDITION(_nodes.size() == 0);
    HG_VALIDATE_PRECONDITION(!aAttachable._assocComps.has_value());

    _nodes.push_back(std::make_unique<Node>(*this, aAttachable));
    aAttachable._assocComps.emplace(*_shipController, *_nodes.back());

    _mainNodeIndex = 0;

    return 0;
}

std::int16_t GraphOfAttachables::insertAttachable(ShipAttachable& aAttachable,
                                                  const decltype(AttachmentEvaluation::bonds)& aBonds,
                                                  hg::alvin::Space&                            aSpace) {
    HG_VALIDATE_PRECONDITION(!aAttachable._assocComps.has_value());

    // *** Determine Index ***

    std::size_t index = 0;
    while (index < _nodes.size() && _nodes[index] != nullptr) {
        ++index;
    }
    if (index >= _nodes.size()) {
        _nodes.resize(index + 1);
    }

    // *** Create Node ***

    auto node = std::make_unique<Node>(*this, aAttachable);

    // *** Create Constraints & Bonds ***

    auto body1 = aAttachable.getPhysicsBody();
    for (const auto& bondStrength : aBonds) {
        auto* node1 = node.get();
        auto* node2 = const_cast<Node*>(getNode(bondStrength.attachableId));
        HG_ASSERT(node2 != nullptr);

        auto body2 = node2->associatedAttachable.getPhysicsBody();

        auto pivot = hg::alvin::Constraint{
            cpPivotJointNew(body1, body2, (cpBodyGetPosition(body1) + cpBodyGetPosition(body2)) * 0.5)};

        cpConstraintSetCollideBodies(pivot, cpFalse); // TODO: temporary

        const cpFloat phase = cpBodyGetAngle(body2) - cpBodyGetAngle(body1);
        const cpFloat ratio = 1.0f; // 1:1 angular lock

        auto gear = hg::alvin::Constraint{cpGearJointNew(body1, body2, phase, ratio)};

        cpConstraintSetCollideBodies(gear, cpFalse); // TODO: temporary

        aSpace.add(pivot);
        aSpace.add(gear);

        auto bond = std::make_shared<Bond>(std::move(pivot), std::move(gear), node1, node2);
        node1->adjacentNodes.push_back({node2, bond});
        node2->adjacentNodes.push_back({node1, bond});
    }

    // *** Connect Everything ***

    aAttachable._assocComps.emplace(*_shipController, *node);
    _nodes[index] = std::move(node);

    return index;
}

const GraphOfAttachables::Node* GraphOfAttachables::getNode(std::int16_t aIndex) const {
    const auto szIdx = hg::pztos(aIndex);
    if (szIdx >= _nodes.size()) {
        return nullptr;
    }
    return _nodes[szIdx].get();
}

std::int16_t GraphOfAttachables::getMainNodeIndex() const {
    return _mainNodeIndex;
}

const GraphOfAttachables::Node& GraphOfAttachables::getMainNode() const {
    return *(_nodes[hg::ToSz(_mainNodeIndex)]);
}

} // namespace cinnabar
