#ifndef WTEXTUREPREVIEWPANEL_H
#define WTEXTUREPREVIEWPANEL_H

#include "IPreviewPanel.h"
#include <Core/Resource/CTexture.h>

namespace Ui {
class WTexturePreviewPanel;
}

class WTexturePreviewPanel : public IPreviewPanel
{
    Q_OBJECT

public:
    explicit WTexturePreviewPanel(QWidget *pParent = 0, CTexture *pTexture = 0);
    ~WTexturePreviewPanel();
    EResourceType ResType();
    void SetResource(CResource *pRes);

private:
    Ui::WTexturePreviewPanel *ui;
};

#endif // WTEXTUREPREVIEWPANEL_H
