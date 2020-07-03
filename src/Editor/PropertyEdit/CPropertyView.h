#ifndef CPROPERTYVIEW_H
#define CPROPERTYVIEW_H

#include "CPropertyModel.h"
#include "CPropertyDelegate.h"
#include <Core/Resource/Script/CScriptObject.h>
#include <QTreeView>

class CPropertyView : public QTreeView
{
    Q_OBJECT

    CPropertyModel* mpModel;
    CPropertyDelegate* mpDelegate;
    CScriptObject* mpObject = nullptr;

    IProperty* mpMenuProperty = nullptr;
    QAction* mpShowNameValidityAction;
    QAction* mpEditTemplateAction;
    QAction* mpGenNamesForPropertyAction;
    QAction* mpGenNamesForSiblingsAction;
    QAction* mpGenNamesForChildrenAction;

public:
    explicit CPropertyView(QWidget* pParent = nullptr);

    void setModel(QAbstractItemModel* pModel) override;
    bool event(QEvent* pEvent) override;
    int sizeHintForColumn(int Column) const override;

    void SetEditor(IEditor* pEditor);
    void ClearProperties();
    void SetIntrinsicProperties(CStructRef InProperties);
    void SetInstance(CScriptObject* pObj);
    void UpdateEditorProperties(const QModelIndex& rkParent);

    CPropertyModel* PropertyModel() const { return mpModel; }

public slots:
    void SetPersistentEditors(const QModelIndex& rkIndex);
    void ClosePersistentEditors(const QModelIndex& rkIndex);
    void OnPropertyModified(const QModelIndex& rkIndex);

    void RefreshView();
    void CreateContextMenu(const QPoint& rkPos);
    void ToggleShowNameValidity(bool ShouldShow);
    void EditPropertyTemplate();

    void GenerateNamesForProperty();
    void GenerateNamesForSiblings();
    void GenerateNamesForChildren();

signals:
    void PropertyModified(const QModelIndex& kIndex);
    void PropertyModified(IProperty* pProperty);
};

#endif // CPROPERTYVIEW_H
