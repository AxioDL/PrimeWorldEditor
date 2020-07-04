#include "CChangeLayerCommand.h"

CChangeLayerCommand::CChangeLayerCommand(CWorldEditor *pEditor, const QList<CScriptNode*>& rkNodeList, CScriptLayer *pNewLayer)
    : IUndoCommand("Change Layer")
    , mpNewLayer(pNewLayer)
    , mpEditor(pEditor)
{
    for (CScriptNode *pNode : rkNodeList)
    {
        CScriptLayer *pLayer = pNode->Instance()->Layer();

        if (pLayer != pNewLayer && !mNodes.contains(pNode))
        {
            mNodes.push_back(pNode);
            mOldLayers[pNode->ID()] = pLayer;
        }
    }
}

void CChangeLayerCommand::undo()
{
    mpEditor->InstancesLayerAboutToChange();
    QList<CSceneNode*> Nodes = mNodes.DereferenceList();

    QList<CScriptNode*> ScriptNodes;
    for (CSceneNode* pNode : Nodes)
        ScriptNodes.push_back(static_cast<CScriptNode*>(pNode));

    for (CScriptNode *pNode : ScriptNodes)
        pNode->Instance()->SetLayer(mOldLayers[pNode->ID()]);

    mpEditor->InstancesLayerChanged(ScriptNodes);
}

void CChangeLayerCommand::redo()
{
    mpEditor->InstancesLayerAboutToChange();
    QList<CSceneNode*> Nodes = mNodes.DereferenceList();

    QList<CScriptNode*> ScriptNodes;
    for (CSceneNode* pNode : Nodes)
        ScriptNodes.push_back(static_cast<CScriptNode*>(pNode));

    for (CScriptNode *pNode : ScriptNodes)
        pNode->Instance()->SetLayer(mpNewLayer);

    mpEditor->InstancesLayerChanged(ScriptNodes);
}

