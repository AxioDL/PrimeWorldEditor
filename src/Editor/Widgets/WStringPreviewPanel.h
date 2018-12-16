#ifndef WSTRINGPREVIEWPANEL_H
#define WSTRINGPREVIEWPANEL_H

#include "IPreviewPanel.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QVector>

class WStringPreviewPanel : public IPreviewPanel
{
    Q_OBJECT

    QVector<QLabel*> mLabels;
    QVBoxLayout *mpLayout;

public:
    explicit WStringPreviewPanel(QWidget *pParent = 0);
    ~WStringPreviewPanel();
    QSize sizeHint() const;
    EResourceType ResType();
    void SetResource(CResource *pRes);
};

#endif // WSTRINGPREVIEWPANEL_H
