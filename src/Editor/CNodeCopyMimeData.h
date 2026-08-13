#ifndef CNODECOPYMIMEDATA_H
#define CNODECOPYMIMEDATA_H

#include <Common/CAssetID.h>
#include <Common/TString.h>
#include <Common/FileIO/CVectorOutStream.h>
#include <Common/Math/CQuaternion.h>
#include <Common/Math/CVector3f.h>
#include <Core/Resource/Cooker/CScriptCooker.h>
#include <Core/Resource/Script/CScriptObject.h>
#include <Core/Scene/CSceneNode.h>
#include "Editor/WorldEditor/CWorldEditor.h"

#include <QList>
#include <QMimeData>

#include <vector>

class CNodeCopyMimeData : public QMimeData
{
    Q_OBJECT

public:
    struct SCopiedNode
    {
        ENodeType Type{};
        TString Name;
        CVector3f Position;
        CQuaternion Rotation;
        CVector3f Scale;

        CInstanceID OriginalInstanceID;
        std::vector<char> InstanceData;
    };

private:
    CWorldEditor *mpEditor;
    CAssetID mAreaID;
    QList<SCopiedNode> mCopiedNodes;
    EGame mGame;

public:
    CNodeCopyMimeData(const CNodeCopyMimeData& rkSrc)
        : mpEditor(rkSrc.mpEditor)
        , mAreaID(rkSrc.mAreaID)
        , mCopiedNodes(rkSrc.mCopiedNodes)
        , mGame(rkSrc.mGame)
    {
    }

    explicit CNodeCopyMimeData(CWorldEditor *pEditor)
        : mpEditor(pEditor)
        , mAreaID(pEditor->ActiveArea()->ID())
        , mGame(pEditor->CurrentGame())
    {
        CNodeSelection *pSelection = pEditor->Selection();
        mCopiedNodes.resize(pSelection->Size());

        uint32_t NodeIndex = 0;
        CVector3f FirstNodePos;
        bool SetFirstNodePos = false;

        for (auto* node : pEditor->Selection()->Nodes())
        {
            SCopiedNode& rNode = mCopiedNodes[NodeIndex];
            rNode.Type = node->NodeType();
            rNode.Name = node->Name();
            rNode.Position = node->LocalPosition();
            rNode.Rotation = node->LocalRotation();
            rNode.Scale = node->LocalScale();

            if (rNode.Type == ENodeType::Script)
            {
                CScriptObject *pInst = static_cast<CScriptNode*>(node)->Instance();
                rNode.OriginalInstanceID = pInst->InstanceID();

                CVectorOutStream Out(&rNode.InstanceData, std::endian::big);

                CScriptCooker Cooker(mGame);
                Cooker.WriteInstance(Out, static_cast<CScriptNode*>(node)->Instance());

                // Replace instance ID with 0xFFFFFFFF to force it to generate a new one.
                Out.Seek(mGame <= EGame::Prime ? 0x5 : 0x6, SEEK_SET);
                Out.WriteU32(0xFFFFFFFF);

                if (!SetFirstNodePos)
                {
                    FirstNodePos = rNode.Position;
                    SetFirstNodePos = true;
                }

                rNode.Position -= FirstNodePos;
            }

            NodeIndex++;
        }
    }

    int IndexOfInstanceID(CInstanceID ID) const
    {
        for (int iNode = 0; iNode < mCopiedNodes.size(); iNode++)
        {
            if (mCopiedNodes[iNode].OriginalInstanceID == ID)
                return iNode;
        }

        return -1;
    }

    CAssetID AreaID() const                       { return mAreaID; }
    EGame Game() const                            { return mGame; }
    const QList<SCopiedNode>& CopiedNodes() const { return mCopiedNodes; }
};

#endif // CNODECOPYMIMEDATA_H

