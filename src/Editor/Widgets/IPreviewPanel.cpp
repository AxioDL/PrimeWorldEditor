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
IPreviewPanel* IPreviewPanel::CreatePanel(EResourceType Type, QWidget *pParent)
{
    switch (Type)
    {
    case EResourceType::Texture:     return new WTexturePreviewPanel(pParent);
    case EResourceType::StringTable: return new WStringPreviewPanel(pParent);
    case EResourceType::Scan:        return new WScanPreviewPanel(pParent);
    default: return nullptr;
    }
}
