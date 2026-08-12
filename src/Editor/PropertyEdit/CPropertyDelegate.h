#ifndef CPROPERTYDELEGATE_H
#define CPROPERTYDELEGATE_H

#include "Core/Resource/Script/Property/IProperty.h"
#include <QStyledItemDelegate>

class CPropertyModel;
class IEditor;

class CPropertyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    IEditor* mpEditor = nullptr;
    CPropertyModel* mpModel = nullptr;
    bool mInRelayWidgetEdit = false;
    mutable bool mEditInProgress = false;
    mutable bool mRelaysBlocked = false;

public:
    explicit CPropertyDelegate(QObject* pParent = nullptr);
    ~CPropertyDelegate() override;

    void SetEditor(IEditor* pEditor);
    void SetPropertyModel(CPropertyModel* pModel);

    QWidget* createEditor(QWidget* pParent, const QStyleOptionViewItem& rkOption, const QModelIndex& rkIndex) const override;
    void setEditorData(QWidget* pEditor, const QModelIndex& rkIndex) const override;
    void setModelData(QWidget* pEditor, QAbstractItemModel* pModel, const QModelIndex& rkIndex) const override;
    bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    QWidget* CreateCharacterEditor(QWidget* pParent, const QModelIndex& rkIndex) const;
    void SetCharacterEditorData(QWidget* pEditor, const QModelIndex& rkIndex) const;
    void SetCharacterModelData(QWidget* pEditor, const QModelIndex& rkIndex) const;
    static EPropertyType DetermineCharacterPropType(EGame Game, const QModelIndex& rkIndex);

public slots:
    void WidgetEdited(QWidget* pWidget, const QModelIndex& rkIndex);

protected:
    void BlockRelays(bool Block) const { mRelaysBlocked = Block; }
};

#endif // CPROPERTYDELEGATE_H
