#ifndef WSCANPREVIEWPANEL_H
#define WSCANPREVIEWPANEL_H

#include "IPreviewPanel.h"

namespace Ui {
class WScanPreviewPanel;
}

class WScanPreviewPanel : public IPreviewPanel
{
    Q_OBJECT

public:
    explicit WScanPreviewPanel(QWidget *pParent = 0);
    ~WScanPreviewPanel();
    QSize sizeHint() const;
    EResourceType ResType();
    void SetResource(CResource *pRes);

private:
    Ui::WScanPreviewPanel *ui;
};

#endif // WSCANPREVIEWPANEL_H
