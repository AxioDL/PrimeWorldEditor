#include "IPreviewPanel.h"
#include "WScanPreviewPanel.h"
#include "WStringPreviewPanel.h"
#include "WTexturePreviewPanel.h"

IPreviewPanel::IPreviewPanel(QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Plain);
    setLineWidth(2);
}

// Can add more if more preview types are implemented
// Not every resource type is really suitable for this though unfortunately
IPreviewPanel* IPreviewPanel::CreatePanel(EResType Type, QWidget *pParent)
{
    switch (Type)
    {
    case eTexture:     return new WTexturePreviewPanel(pParent);
    case eStringTable: return new WStringPreviewPanel(pParent);
    case eScan:        return new WScanPreviewPanel(pParent);
    default: return nullptr;
    }
}
