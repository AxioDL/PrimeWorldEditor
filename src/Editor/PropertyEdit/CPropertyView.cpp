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
    connect(mpDelegate, SIGNAL(PropertyEdited(QModelIndex,bool)), this, SLOT(OnPropertyModified(QModelIndex,bool)));
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
        QPoint MousePos = QCursor::pos();
        QModelIndex Index = indexAt(viewport()->mapFromGlobal(MousePos));

        if (Index.isValid())
        {
            QString Desc = mpModel->data(Index, Qt::ToolTipRole).toString();

            if (!Desc.isEmpty())
            {
                QToolTip::showText(MousePos, Desc, this);
                pEvent->accept();
                return true;
            }
        }

        QToolTip::hideText();
        pEvent->ignore();
        return true;
    }

    else return QTreeView::event(pEvent);
}

void CPropertyView::SetObject(CScriptObject *pObj)
{
    mpObject = pObj;
    mpModel->SetBaseStruct(pObj ? pObj->Properties() : nullptr);
    SetPersistentEditors(QModelIndex());

    // Auto-expand EditorProperties
    QModelIndex Index = mpModel->index(0, 0, QModelIndex());
    IProperty *pProp = mpModel->PropertyForIndex(Index, false);
    if (pProp && pProp->ID() == 0x255A4580)
        expand(Index);
}

void CPropertyView::UpdateEditorProperties(const QModelIndex& rkParent)
{
    // Iterate over all properties and update if they're an editor property. Ignore structs unless they're EditorProperties or a single struct.
    for (int iRow = 0; iRow < mpModel->rowCount(rkParent); iRow++)
    {
        QModelIndex Index0 = mpModel->index(iRow, 0, rkParent);
        QModelIndex Index1 = mpModel->index(iRow, 1, rkParent);
        IProperty *pProp = mpModel->PropertyForIndex(Index0, false);

        if (pProp)
        {
            if (pProp->Type() == eStructProperty)
            {
                CStructTemplate *pStruct = static_cast<CStructTemplate*>(pProp->Template());

                if (pStruct->IsSingleProperty() || pStruct->PropertyID() == 0x255A4580)
                    UpdateEditorProperties(Index0);
                else
                    continue;
            }

            else if (mpObject->IsEditorProperty(pProp))
            {
                mpModel->dataChanged(Index1, Index1);

                if (mpModel->rowCount(Index0) != 0)
                {
                    QModelIndex SubIndexA = mpModel->index(0, 1, Index0);
                    QModelIndex SubIndexB = mpModel->index(mpModel->rowCount(Index0) - 1, 1, Index0);
                    mpModel->dataChanged(SubIndexA, SubIndexB);
                }
            }
        }
    }
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

void CPropertyView::ClosePersistentEditors(const QModelIndex& rkIndex)
{
    u32 NumChildren = mpModel->rowCount(rkIndex);

    for (u32 iChild = 0; iChild < NumChildren; iChild++)
    {
        QModelIndex ChildIndex = rkIndex.child(iChild, 1);
        closePersistentEditor(ChildIndex);

        if (mpModel->rowCount(ChildIndex) != 0)
            ClosePersistentEditors(ChildIndex);
    }
}

void CPropertyView::OnPropertyModified(const QModelIndex &rkIndex, bool IsDone)
{
    // Check for a resource being changed on a character property. If that's the case we need to remake the persistent editors.
    if (rkIndex.internalId() & 0x1)
    {
        if (mpModel->PropertyForIndex(rkIndex, true)->Type() == eCharacterProperty)
        {
            EGame Game = static_cast<TCharacterProperty*>(mpModel->PropertyForIndex(rkIndex, true))->Get().Version();

            if (mpDelegate->DetermineCharacterPropType(Game, rkIndex) == eFileProperty)
            {
                QModelIndex Parent = rkIndex.parent();
                ClosePersistentEditors(Parent);
                SetPersistentEditors(Parent);
            }
        }
    }

    emit PropertyModified(rkIndex, IsDone);
}
