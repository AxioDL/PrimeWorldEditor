#include "WTexturePreviewPanel.h"
#include "ui_WTexturePreviewPanel.h"
#include "WTextureGLWidget.h"
#include "Editor/UICommon.h"

WTexturePreviewPanel::WTexturePreviewPanel(QWidget *parent, CTexture *pTexture)
    : IPreviewPanel(parent)
    , ui(new Ui::WTexturePreviewPanel)
{
    ui->setupUi(this);
    SetResource(pTexture);
}

WTexturePreviewPanel::~WTexturePreviewPanel()
{
    delete ui;
}

EResourceType WTexturePreviewPanel::ResType()
{
    return EResourceType::Texture;
}

void WTexturePreviewPanel::SetResource(CResource *pRes)
{
    CTexture *pTexture = (CTexture*) pRes;
    ui->TextureGLWidget->SetTexture(pTexture);

    if (pTexture)
    {
        TString Name = pTexture->Source();
        ui->TextureNameLabel->setText( TO_QSTRING(Name) );

        QString TexInfo;
        TexInfo += QString::number(pTexture->Width()) + "x" + QString::number(pTexture->Height());
        TexInfo += " ";

        switch (pTexture->SourceTexelFormat())
        {
        case ETexelFormat::GX_I4:     TexInfo += "I4";     break;
        case ETexelFormat::GX_I8:     TexInfo += "I8";     break;
        case ETexelFormat::GX_IA4:    TexInfo += "IA4";    break;
        case ETexelFormat::GX_IA8:    TexInfo += "IA8";    break;
        case ETexelFormat::GX_C4:     TexInfo += "C4";     break;
        case ETexelFormat::GX_C8:     TexInfo += "C8";     break;
        case ETexelFormat::GX_C14x2:  TexInfo += "C14x2";  break;
        case ETexelFormat::GX_RGB565: TexInfo += "RGB565"; break;
        case ETexelFormat::GX_RGB5A3: TexInfo += "RGB5A3"; break;
        case ETexelFormat::GX_RGBA8:  TexInfo += "RGBA8";  break;
        case ETexelFormat::GX_CMPR:   TexInfo += "CMPR";   break;
        default:                      TexInfo += "Invalid Format"; break;
        }

        TexInfo += " / ";
        TexInfo += QString::number(pTexture->NumMipMaps()) + " mipmaps";

        ui->TextureInfoLabel->setText(TexInfo);
    }
    else
    {
        ui->TextureNameLabel->setText("No texture");
        ui->TextureInfoLabel->setText("");
    }
}
