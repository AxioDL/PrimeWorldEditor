#ifndef CCHARACTEREDITORVIEWPORT_H
#define CCHARACTEREDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include "Editor/CGridRenderable.h"
#include <Core/Scene/CCharacterNode.h>

class CCharacterEditorViewport : public CBasicViewport
{
    Q_OBJECT

    CCharacterNode *mpCharNode;
    CGridRenderable mGrid;
    CRenderer *mpRenderer;
    u32 mHoverBone;
    bool mGridEnabled;

public:
    CCharacterEditorViewport(QWidget *pParent = 0);
    ~CCharacterEditorViewport();
    void SetNode(CCharacterNode *pNode);
    void CheckUserInput();
    void Paint();
    void OnResize();
    void OnMouseClick(QMouseEvent *pEvent);

    inline u32 HoverBoneID() const          { return mHoverBone; }
    inline void SetGridEnabled(bool Enable) { mGridEnabled = Enable; }

signals:
    void HoverBoneChanged(u32 BoneID);
    void ViewportClick(QMouseEvent *pEvent);
};

#endif // CCHARACTEREDITORVIEWPORT_H
