#ifndef CSELECTRESOURCEPANEL_H
#define CSELECTRESOURCEPANEL_H

#include <QWidget>
#include "CFilteredResourceModel.h"
#include "CResourceSelector.h"

namespace Ui {
class CSelectResourcePanel;
}

class CSelectResourcePanel : public QFrame
{
    Q_OBJECT
    Ui::CSelectResourcePanel *mpUI;
    CResourceSelector *mpSelector;

    CFilteredResourceModel mModel;
    CFilteredResourceProxyModel mProxyModel;

public:
    explicit CSelectResourcePanel(CResourceSelector *pSelector);
    ~CSelectResourcePanel();

public slots:
    void FocusChanged(QWidget *pOld, QWidget *pNew);
    void SearchStringChanged(QString SearchString);
    void ResourceClicked(QModelIndex Index);
};

#endif // CSELECTRESOURCEPANEL_H
