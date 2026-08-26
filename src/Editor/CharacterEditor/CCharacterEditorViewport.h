#ifndef CCHARACTEREDITORVIEWPORT_H
#define CCHARACTEREDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"

#include <memory>

class CCharacterNode;

class CCharacterEditorViewport : public CBasicViewport
{
    Q_OBJECT

    CCharacterNode *mpCharNode = nullptr;
    CGridRenderable mGrid;
    std::unique_ptr<CRenderer> mpRenderer;
    uint32_t mHoverBone = 0;
    bool mGridEnabled = true;

public:
    explicit CCharacterEditorViewport(QWidget* pParent = nullptr);
    ~CCharacterEditorViewport() override;

    void SetNode(CCharacterNode* pNode) { mpCharNode = pNode; }
    void SetGridEnabled(bool Enable) { mGridEnabled = Enable; }

    uint32_t HoverBoneID() const { return mHoverBone; }

protected:
    void CheckUserInput() override;
    void Paint() override;
    void OnResize() override;
    void OnMouseClick(QMouseEvent* pEvent) override;

signals:
    void HoverBoneChanged(uint32_t BoneID);
    void ViewportClick(QMouseEvent *pEvent);
};

#endif // CCHARACTEREDITORVIEWPORT_H
