#ifndef CWORLDINFOSIDEBAR_H
#define CWORLDINFOSIDEBAR_H

#include <QWidget>
#include <QSortFilterProxyModel>
#include "CWorldEditorSidebar.h"
#include "CWorldTreeModel.h"

#include <memory>

class CWorldEditor;

namespace Ui {
class CWorldInfoSidebar;
}

class CWorldInfoSidebar : public CWorldEditorSidebar
{
    Q_OBJECT

    std::unique_ptr<Ui::CWorldInfoSidebar> mpUI;
    CWorldTreeModel mModel;
    CWorldTreeProxyModel mProxyModel;

public:
    explicit CWorldInfoSidebar(CWorldEditor *pEditor);
    ~CWorldInfoSidebar() override;

public slots:
    void OnActiveProjectChanged(CGameProject *pProj);
    void OnAreaFilterStringChanged(const QString& rkFilter);
    void OnWorldTreeClicked(QModelIndex Index);
    void OnWorldTreeDoubleClicked(QModelIndex Index);
    void ClearWorldInfo();
    void ClearAreaInfo();
};

#endif // CWORLDINFOSIDEBAR_H
