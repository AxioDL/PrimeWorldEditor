#ifndef CCHARACTEREDITORVIEWPORT_H
#define CCHARACTEREDITORVIEWPORT_H

#include "Editor/CBasicViewport.h"
#include <Core/Scene/CCharacterNode.h>

class CCharacterEditorViewport : public CBasicViewport
{
    CCharacterNode *mpCharNode;
    CRenderer *mpRenderer;

public:
    CCharacterEditorViewport(QWidget *pParent = 0);
    ~CCharacterEditorViewport();
    void SetNode(CCharacterNode *pNode);
    void Paint();
    void OnResize();
};

#endif // CCHARACTEREDITORVIEWPORT_H
