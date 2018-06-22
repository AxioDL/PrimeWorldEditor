#include "CPropertyView.h"
#include "CPropertyDelegate.h"
#include "Editor/WorldEditor/CTemplateEditDialog.h"
#include <Core/Resource/Script/IPropertyTemplate.h>

#include <QEvent>
#include <QMenu>
#include <QToolTip>

CPropertyView::CPropertyView(QWidget *pParent)
    : QTreeView(pParent)
    , mpEditor(nullptr)
    , mpMenuProperty(nullptr)
{
    mpModel = new CPropertyModel(this);
    mpDelegate = new CPropertyDelegate(this);
    setItemDelegateForColumn(1, mpDelegate);
    setEditTriggers(AllEditTriggers);
    setModel(mpModel);

    setContextMenuPolicy(Qt::CustomContextMenu);

    mpShowNameValidityAction = new QAction("Show whether property name is correct", this);
    mpShowNameValidityAction->setCheckable(true);
    mpShowNameValidityAction->setChecked(false);
    connect(mpShowNameValidityAction, SIGNAL(triggered(bool)), this, SLOT(ToggleShowNameValidity(bool)));

    mpEditTemplateAction = new QAction("Edit template", this);
    connect(mpEditTemplateAction, SIGNAL(triggered()), this, SLOT(EditPropertyTemplate()));

    connect(this, SIGNAL(expanded(QModelIndex)), this, SLOT(SetPersistentEditors(QModelIndex)));
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(edit(QModelIndex)));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CreateContextMenu(QPoint)));
    connect(mpModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(SetPersistentEditors(QModelIndex)));
    connect(mpModel, SIGNAL(PropertyModified(const QModelIndex&)), this, SLOT(OnPropertyModified(const QModelIndex&)));
}

void CPropertyView::setModel(QAbstractItemModel *pModel)
{
    CPropertyModel *pPropModel = qobject_cast<CPropertyModel*>(pModel);
    mpModel = pPropModel;
    mpDelegate->SetPropertyModel(pPropModel);
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

void CPropertyView::SetEditor(CWorldEditor *pEditor)
{
    mpEditor = pEditor;
    mpDelegate->SetEditor(pEditor);
    connect(mpEditor, SIGNAL(PropertyModified(CScriptObject*,IProperty*)), mpModel, SLOT(NotifyPropertyModified(CScriptObject*,IProperty*)));
}

void CPropertyView::SetInstance(CScriptObject *pObj)
{
    mpObject = pObj;
    mpModel->SetBoldModifiedProperties(mpEditor ? (mpEditor->CurrentGame() > ePrime) : true);

    if (pObj)
        mpModel->ConfigureScript(pObj->Area()->Entry()->Project(), pObj->Template()->Properties(), pObj);
    else
        mpModel->ConfigureScript(nullptr, nullptr, nullptr);

    SetPersistentEditors(QModelIndex());

    // Auto-expand EditorProperties
    QModelIndex Index = mpModel->index(0, 0, QModelIndex());
    IPropertyNew *pProp = mpModel->PropertyForIndex(Index, false);
    if (pProp && pProp->ID() == 0x255A4580)
        expand(Index);
}

void CPropertyView::UpdateEditorProperties(const QModelIndex& rkParent)
{
    // Check what game this is
    EGame Game = mpEditor->CurrentGame();

    // Iterate over all properties and update if they're an editor property.
    for (int iRow = 0; iRow < mpModel->rowCount(rkParent); iRow++)
    {
        QModelIndex Index0 = mpModel->index(iRow, 0, rkParent);
        QModelIndex Index1 = mpModel->index(iRow, 1, rkParent);
        IPropertyNew *pProp = mpModel->PropertyForIndex(Index0, false);

        if (pProp)
        {
            // For structs, update sub-properties.
            if (pProp->Type() == EPropertyTypeNew::Struct)
            {
                CStructPropertyNew *pStruct = TPropCast<CStructPropertyNew>(pProp);

                // As an optimization, in MP2+, we don't need to update unless this is an atomic struct or if
                // it's EditorProperties, because other structs never have editor properties in them.
                // In MP1 this isn't the case so we need to update every struct regardless
                if ((Game <= ePrime) || (pStruct->IsAtomic() || pStruct->ID() == 0x255A4580))
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
        IPropertyNew *pProp = mpModel->PropertyForIndex(ChildIndex, false);
        EPropertyTypeNew Type = (pProp ? pProp->Type() : EPropertyTypeNew::Invalid);

        // Handle persistent editors under character properties
        if (!pProp && ChildIndex.internalId() & 0x80000000)
        {
            pProp = mpModel->PropertyForIndex(ChildIndex, true);

            if (pProp->Type() == EPropertyTypeNew::AnimationSet)
            {
                EGame Game = mpObject->Area()->Game();
                Type = mpDelegate->DetermineCharacterPropType(Game, ChildIndex);
            }

            if (pProp->Type() == EPropertyTypeNew::Flags)
                Type = EPropertyTypeNew::Bool;
        }


        switch (Type)
        {
        case EPropertyTypeNew::Bool:
        case EPropertyTypeNew::Enum:
        case EPropertyTypeNew::Choice:
        case EPropertyTypeNew::Color:
        case EPropertyTypeNew::Asset:
            openPersistentEditor(ChildIndex);
            break;
        case EPropertyTypeNew::Struct:
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

void CPropertyView::OnPropertyModified(const QModelIndex& rkIndex)
{
    // Check for a character resource being changed. If that's the case we need to remake the persistent editors.
    IPropertyNew *pProp = mpModel->PropertyForIndex(rkIndex, true);

    if (pProp->Type() == EPropertyTypeNew::AnimationSet /*&& rkIndex.internalId() & 0x1*/)
    {
        ClosePersistentEditors(rkIndex);
        SetPersistentEditors(rkIndex);
    }
}

void CPropertyView::CreateContextMenu(const QPoint& rkPos)
{
    QModelIndex Index = indexAt(rkPos);

    if (Index.isValid() && Index.column() == 0)
    {
        IPropertyNew *pProp = mpModel->PropertyForIndex(Index, true);
        mpMenuProperty = pProp;

        QMenu Menu;
        Menu.addAction(mpEditTemplateAction);

        if (mpEditor->CurrentGame() >= eEchoesDemo)
        {
            Menu.addAction(mpShowNameValidityAction);
        }

        Menu.exec(viewport()->mapToGlobal(rkPos));
    }
}

void CPropertyView::ToggleShowNameValidity(bool ShouldShow)
{
    mpModel->SetShowPropertyNameValidity(ShouldShow);
}

void CPropertyView::EditPropertyTemplate()
{
    CTemplateEditDialog Dialog(mpMenuProperty, mpEditor);
    Dialog.exec();
}
