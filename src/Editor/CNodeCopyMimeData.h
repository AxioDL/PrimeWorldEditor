#ifndef CNODECOPYMIMEDATA
#define CNODECOPYMIMEDATA

#include <Common/TString.h>
#include <Math/CVector3f.h>
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

        u32 OriginalInstanceID;
        std::vector<char> InstanceData;
    };

private:
    CWorldEditor *mpEditor;
    CUniqueID mAreaID;
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

    CNodeCopyMimeData(CWorldEditor *pEditor)
        : mpEditor(pEditor)
        , mAreaID(pEditor->ActiveArea()->ResID())
        , mGame(pEditor->CurrentGame())
    {
        CNodeSelection *pSelection = pEditor->Selection();
        mCopiedNodes.resize(pSelection->Size());

        u32 NodeIndex = 0;
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

            if (rNode.Type == eScriptNode)
            {
                CScriptObject *pInst = static_cast<CScriptNode*>(*It)->Object();
                rNode.OriginalInstanceID = pInst->InstanceID();

                CVectorOutStream Out(&rNode.InstanceData, IOUtil::eBigEndian);
                CScriptCooker::CookInstance(eReturns, static_cast<CScriptNode*>(*It)->Object(), Out);

                // Replace instance ID with 0xFFFFFFFF to force it to generate a new one.
                Out.Seek(0x6, SEEK_SET);
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

    int IndexOfInstanceID(u32 InstanceID) const
    {
        for (int iNode = 0; iNode < mCopiedNodes.size(); iNode++)
        {
            if (mCopiedNodes[iNode].OriginalInstanceID == InstanceID)
                return iNode;
        }

        return -1;
    }

    CUniqueID AreaID() const { return mAreaID; }
    EGame Game() const { return mGame; }
    const QVector<SCopiedNode>& CopiedNodes() const { return mCopiedNodes; }
};

#endif // CNODECOPYMIMEDATA

