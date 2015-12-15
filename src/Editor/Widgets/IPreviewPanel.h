#ifndef IPREVIEWPANEL_H
#define IPREVIEWPANEL_H

#include <QFrame>
#include <Resource/CResource.h>
#include <Resource/EResType.h>

class IPreviewPanel : public QFrame
{
public:
    explicit IPreviewPanel(QWidget *parent = 0);
    virtual EResType ResType() = 0;
    virtual void SetResource(CResource *pRes) = 0;
    static IPreviewPanel* CreatePanel(EResType Type, QWidget *pParent = 0);
};

#endif // IPREVIEWPANEL_H
