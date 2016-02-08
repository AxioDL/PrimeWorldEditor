#include "CChangeLayerCommand.h"

CChangeLayerCommand::CChangeLayerCommand(CWorldEditor *pEditor, const QList<CScriptNode*>& rkNodeList, CScriptLayer *pNewLayer)
    : IUndoCommand("Change Layer")
    , mNodeList(rkNodeList)
    , mpNewLayer(pNewLayer)
    , mpEditor(pEditor)
{
    foreach (CScriptNode *pNode, mNodeList)
    {
        CScriptLayer *pLayer = pNode->Object()->Layer();

        if (pLayer == pNewLayer)
        {
            mNodeList.removeOne(pNode);
            continue;
        }

        mOldLayers[pNode] = pLayer;
    }
}

void CChangeLayerCommand::undo()
{
    mpEditor->InstancesLayerAboutToChange();

    foreach (CScriptNode *pNode, mNodeList)
        pNode->Object()->SetLayer(mOldLayers[pNode]);

    mpEditor->InstancesLayerChanged(mNodeList);
}

void CChangeLayerCommand::redo()
{
    mpEditor->InstancesLayerAboutToChange();

    foreach (CScriptNode *pNode, mNodeList)
        pNode->Object()->SetLayer(mpNewLayer);

    mpEditor->InstancesLayerChanged(mNodeList);
}

