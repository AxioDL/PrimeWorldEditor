#ifndef CPROPERTYVIEW_H
#define CPROPERTYVIEW_H

#include <Core/Resource/Script/Property/TPropertyRef.h>
#include <QTreeView>

class CPropertyDelegate;
class CPropertyModel;
class CScriptObject;
class IEditor;
class IProperty;

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
    ~CPropertyView() override;

    void setModel(QAbstractItemModel* pModel) override;

    void SetEditor(IEditor* pEditor);
    void ClearProperties();
    void SetIntrinsicProperties(CStructRef InProperties);
    void SetInstance(CScriptObject* pObj);
    void UpdateEditorProperties(const QModelIndex& rkParent);

    CPropertyModel* PropertyModel() const { return mpModel; }

protected:
    bool event(QEvent* pEvent) override;
    int sizeHintForColumn(int Column) const override;

private slots:
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
