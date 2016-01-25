#include "CPropertyView.h"
#include "CPropertyDelegate.h"
#include <Core/Resource/Script/IPropertyTemplate.h>

#include <QEvent>
#include <QToolTip>

CPropertyView::CPropertyView(QWidget *pParent)
    : QTreeView(pParent)
{
    mpModel = new CPropertyModel(this);
    mpDelegate = new CPropertyDelegate(this);
    setItemDelegateForColumn(1, mpDelegate);
    setEditTriggers(AllEditTriggers);
    setModel(mpModel);

    connect(mpModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(SetPersistentEditors(QModelIndex)));
    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(SetPersistentEditors(QModelIndex)));
}

void CPropertyView::setModel(QAbstractItemModel *pModel)
{
    CPropertyModel *pPropModel = qobject_cast<CPropertyModel*>(pModel);
    mpModel = pPropModel;
    mpDelegate->SetModel(pPropModel);
    QTreeView::setModel(pPropModel);

    if (pPropModel)
    {
        QModelIndex Root = pPropModel->index(0, 0, QModelIndex());
        SetPersistentEditors(Root);
        setExpanded(Root, true);
    }
}

bool CPropertyView::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ToolTip)
    {
        QPoint MousePos = mapFromGlobal(QCursor::pos());
        QModelIndex Index = indexAt(MousePos);
        QString Desc = mpModel->data(Index, Qt::ToolTipRole).toString();

        if (!Desc.isEmpty())
        {
            QToolTip::showText(MousePos, Desc, this);
            pEvent->accept();
        }
        else
        {
            QToolTip::hideText();
            pEvent->ignore();
        }

        return true;
    }

    else return QTreeView::event(pEvent);
}

void CPropertyView::SetBaseStruct(CPropertyStruct *pStruct)
{
    mpModel->SetBaseStruct(pStruct);
    SetPersistentEditors(QModelIndex());

    // Auto-expand EditorProperties
    QModelIndex Index = mpModel->index(0, 0, QModelIndex());
    IProperty *pProp = mpModel->PropertyForIndex(Index, false);
    if (pProp && pProp->ID() == 0x255A4580)
        expand(Index);
}

void CPropertyView::SetPersistentEditors(const QModelIndex& rkParent)
{
    u32 NumChildren = mpModel->rowCount(rkParent);

    for (u32 iChild = 0; iChild < NumChildren; iChild++)
    {
        QModelIndex ChildIndex = mpModel->index(iChild, 1, rkParent);
        IProperty *pProp = mpModel->PropertyForIndex(ChildIndex, false);
        EPropertyType Type = (pProp ? pProp->Type() : eInvalidProperty);

        // Handle persistent editors under character properties
        if (!pProp && ChildIndex.internalId() & 0x1)
        {
            pProp = mpModel->PropertyForIndex(ChildIndex, true);

            if (pProp->Type() == eCharacterProperty)
            {
                EGame Game = static_cast<TCharacterProperty*>(pProp)->Get().Version();
                Type = mpDelegate->DetermineCharacterPropType(Game, ChildIndex);
            }

            if (pProp->Type() == eBitfieldProperty)
                Type = eBoolProperty;
        }

        switch (Type)
        {
        case eBoolProperty:
        case eEnumProperty:
        case eColorProperty:
        case eFileProperty:
            openPersistentEditor(ChildIndex);
            break;
        case eStructProperty:
            setFirstColumnSpanned(iChild, rkParent, true);
            break;
        }

        if (isExpanded(ChildIndex))
            SetPersistentEditors(ChildIndex);
    }
}

