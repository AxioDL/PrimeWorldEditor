#ifndef CMODELEDITORVIEWPORT_H
#define CMODELEDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"

#include <memory>

class CMaterial;
class CModelNode;
class CRenderer;

class CModelEditorViewport : public CBasicViewport
{
    Q_OBJECT

public:
    enum class EDrawMode {
        DrawMesh, DrawSphere, DrawSquare,
    };

    explicit CModelEditorViewport(QWidget *pParent = nullptr);
    ~CModelEditorViewport() override;

    void SetNode(CModelNode *pNode);
    void SetActiveMaterial(CMaterial *pMat);
    void SetDrawMode(EDrawMode Mode);
    void SetClearColor(const CColor& Color);
    void SetGridEnabled(bool Enable);

protected:
    void Paint() override;
    void OnResize() override;

private:
    EDrawMode mMode{EDrawMode::DrawMesh};
    CGridRenderable mGrid;
    CModelNode* mpModelNode = nullptr;
    CMaterial* mpActiveMaterial = nullptr;
    std::unique_ptr<CRenderer> mpRenderer;
    bool mGridEnabled = true;
};

#endif // CMODELEDITORVIEWPORT_H
