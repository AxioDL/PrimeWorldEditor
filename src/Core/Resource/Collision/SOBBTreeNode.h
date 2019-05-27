#ifndef SOBBTREENODE_H
#define SOBBTREENODE_H

#include <Common/BasicTypes.h>
#include <Common/Math/CTransform4f.h>
#include <Common/Math/CVector3f.h>
#include <memory>

enum class EOBBTreeNodeType : uint8
{
    Branch = 0,
    Leaf = 1
};

struct SOBBTreeNode
{
    virtual ~SOBBTreeNode() = default;
    CTransform4f        Transform;
    CVector3f           Radii;
    EOBBTreeNodeType    NodeType;
};

struct SOBBTreeBranch : public SOBBTreeNode
{
    std::unique_ptr<SOBBTreeNode> pLeft;
    std::unique_ptr<SOBBTreeNode> pRight;

    SOBBTreeBranch() : SOBBTreeNode()
    {
        NodeType = EOBBTreeNodeType::Branch;
    }
};

struct SOBBTreeLeaf : public SOBBTreeNode
{
    std::vector<uint16> TriangleIndices;

    SOBBTreeLeaf() : SOBBTreeNode()
    {
        NodeType = EOBBTreeNodeType::Leaf;
    }
};

#endif // SOBBTREENODE_H
