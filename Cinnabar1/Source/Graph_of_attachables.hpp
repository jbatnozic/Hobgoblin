// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Attachment_evaluation.hpp>

#include <Hobgoblin/Alvin/Constraint.hpp>
#include <Hobgoblin/Alvin/Space.hpp>
#include <Hobgoblin/Utility/Compressed_small_vector.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace cinnabar {

class ShipAttachable;
class ShipController;

class GraphOfAttachables {
public:
    GraphOfAttachables() = default;

    void init(ShipController& aController);

    struct Node;

    //! \brief Holds data regarding the bond (constraints) between two attachables in the graph.
    struct Bond {
        hg::alvin::Constraint pivot;
        hg::alvin::Constraint gear;

        Node* node1;
        Node* node2;
    };

    //! \brief A single node of the graph, representing exactly one attachable in the graph.
    struct Node {
        GraphOfAttachables& parentGraph;          //!< The graph that owns this node
        ShipAttachable&     associatedAttachable; //!< The Node's sibling ShipAttachable

        struct AdjacentNode {
            Node*                 nodePtr;
            std::shared_ptr<Bond> bond;

            AdjacentNode(Node* aNode, std::shared_ptr<Bond> aBond);
        };

        hg::util::CompressedSmallVector<AdjacentNode, 3> adjacentNodes;

        bool visited = false;

        Node(GraphOfAttachables& aParentGraph, ShipAttachable& aAssociatedAttachable);
    };

    //! returns the index of the inserted attachable
    std::int16_t insertInitialAttachable(ShipAttachable& aAttachable);

    std::int16_t insertAttachable(ShipAttachable&                              aAttachable,
                                  const decltype(AttachmentEvaluation::bonds)& aBonds,
                                  hg::alvin::Space&                            aSpace);

    void removeAttachable();

    //! get node by index
    const Node* getNode(std::int16_t aIndex) const;

    std::int16_t getMainNodeIndex() const;

    const Node& getMainNode() const;

private:
    friend class ShipAttachable;

    ShipController* _shipController = nullptr;

    std::vector<std::unique_ptr<Node>> _nodes;

    std::int16_t _mainNodeIndex = 0;
};

} // namespace cinnabar
