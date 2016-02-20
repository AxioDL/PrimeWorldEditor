#ifndef CMODELEDITORVIEWPORT_H
#define CMODELEDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include <Core/Scene/CModelNode.h>

class CModelEditorViewport : public CBasicViewport
{
public:
    enum EDrawMode {
        eDrawMesh, eDrawSphere, eDrawSquare
    };

private:
    EDrawMode mMode;
    CModelNode *mpModelNode;
    CMaterial *mpActiveMaterial;
    CRenderer *mpRenderer;
    bool mGridEnabled;

public:
    CModelEditorViewport(QWidget *pParent = 0);
    ~CModelEditorViewport();
    void SetNode(CModelNode *pNode);
    void SetActiveMaterial(CMaterial *pMat);
    void SetDrawMode(EDrawMode mode);
    void SetClearColor(CColor color);
    void SetGridEnabled(bool Enable);
    void Paint();
    void OnResize();
};

#endif // CMODELEDITORVIEWPORT_H
