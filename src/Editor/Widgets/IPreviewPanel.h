#ifndef IPREVIEWPANEL_H
#define IPREVIEWPANEL_H

#include <Core/Resource/CResource.h>
#include <Core/Resource/EResType.h>

#include <QFrame>

class IPreviewPanel : public QFrame
{
public:
    explicit IPreviewPanel(QWidget *parent = 0);
    virtual EResourceType ResType() = 0;
    virtual void SetResource(CResource *pRes) = 0;
    static IPreviewPanel* CreatePanel(EResourceType Type, QWidget *pParent = 0);
};

#endif // IPREVIEWPANEL_H
