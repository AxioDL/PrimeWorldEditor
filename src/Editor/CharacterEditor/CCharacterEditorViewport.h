#ifndef CCHARACTEREDITORVIEWPORT_H
#define CCHARACTEREDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include <Core/Scene/CCharacterNode.h>
#include <memory>

class CCharacterEditorViewport : public CBasicViewport
{
    Q_OBJECT

    CCharacterNode *mpCharNode = nullptr;
    CGridRenderable mGrid;
    std::unique_ptr<CRenderer> mpRenderer;
    uint32 mHoverBone = 0;
    bool mGridEnabled = true;

public:
    explicit CCharacterEditorViewport(QWidget* pParent = nullptr);
    ~CCharacterEditorViewport() override;

    void SetNode(CCharacterNode *pNode);
    void CheckUserInput() override;
    void Paint() override;
    void OnResize() override;
    void OnMouseClick(QMouseEvent *pEvent) override;

    uint32 HoverBoneID() const       { return mHoverBone; }
    void SetGridEnabled(bool Enable) { mGridEnabled = Enable; }

signals:
    void HoverBoneChanged(uint32 BoneID);
    void ViewportClick(QMouseEvent *pEvent);
};

#endif // CCHARACTEREDITORVIEWPORT_H
