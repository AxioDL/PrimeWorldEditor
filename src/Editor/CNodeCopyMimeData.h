#ifndef CNODECOPYMIMEDATA
#define CNODECOPYMIMEDATA

#include <Common/TString.h>
#include <Common/Math/CVector3f.h>
#include <Core/Resource/Cooker/CScriptCooker.h>
#include <Core/Resource/Factory/CScriptLoader.h>
#include <Core/Scene/CSceneNode.h>
#include "Editor/CSelectionIterator.h"
#include "Editor/WorldEditor/CWorldEditor.h"

#include <QMimeData>

class CNodeCopyMimeData : public QMimeData
{
    Q_OBJECT

public:
    struct SCopiedNode
    {
        ENodeType Type;
        TString Name;
        CVector3f Position;
        CQuaternion Rotation;
        CVector3f Scale;

        uint32 OriginalInstanceID;
        std::vector<char> InstanceData;
    };

private:
    CWorldEditor *mpEditor;
    CAssetID mAreaID;
    QVector<SCopiedNode> mCopiedNodes;
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

        uint32 NodeIndex = 0;
        CVector3f FirstNodePos;
        bool SetFirstNodePos = false;

        for (CSelectionIterator It(pEditor->Selection()); It; ++It)
        {
            SCopiedNode& rNode = mCopiedNodes[NodeIndex];
            rNode.Type = It->NodeType();
            rNode.Name = It->Name();
            rNode.Position = It->LocalPosition();
            rNode.Rotation = It->LocalRotation();
            rNode.Scale = It->LocalScale();

            if (rNode.Type == ENodeType::Script)
            {
                CScriptObject *pInst = static_cast<CScriptNode*>(*It)->Instance();
                rNode.OriginalInstanceID = pInst->InstanceID();

                CVectorOutStream Out(&rNode.InstanceData, EEndian::BigEndian);

                CScriptCooker Cooker(mGame);
                Cooker.WriteInstance(Out, static_cast<CScriptNode*>(*It)->Instance());

                // Replace instance ID with 0xFFFFFFFF to force it to generate a new one.
                Out.Seek(mGame <= EGame::Prime ? 0x5 : 0x6, SEEK_SET);
                Out.WriteLong(0xFFFFFFFF);

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

    int IndexOfInstanceID(uint32 InstanceID) const
    {
        for (int iNode = 0; iNode < mCopiedNodes.size(); iNode++)
        {
            if (mCopiedNodes[iNode].OriginalInstanceID == InstanceID)
                return iNode;
        }

        return -1;
    }

    CAssetID AreaID() const                         { return mAreaID; }
    EGame Game() const                              { return mGame; }
    const QVector<SCopiedNode>& CopiedNodes() const { return mCopiedNodes; }
};

#endif // CNODECOPYMIMEDATA

