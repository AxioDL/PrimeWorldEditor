#include "CPropertyView.h"
#include "CPropertyDelegate.h"
#include "Editor/WorldEditor/CTemplateEditDialog.h"
#include <Core/Resource/Script/Property/Properties.h>

#include <QEvent>
#include <QMenu>
#include <QToolTip>

CPropertyView::CPropertyView(QWidget *pParent)
    : QTreeView(pParent)
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

    mpGenNamesForPropertyAction = new QAction("Generate names for this property", this);
    mpGenNamesForSiblingsAction = new QAction(this); // Text set in CreateContextMenu()
    mpGenNamesForChildrenAction = new QAction(this); // Text set in CreateContextMenu()
    connect(mpGenNamesForPropertyAction, SIGNAL(triggered(bool)), this, SLOT(GenerateNamesForProperty()));
    connect(mpGenNamesForSiblingsAction, SIGNAL(triggered(bool)), this, SLOT(GenerateNamesForSiblings()));
    connect(mpGenNamesForChildrenAction, SIGNAL(triggered(bool)), this, SLOT(GenerateNamesForChildren()));

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

void CPropertyView::InitColumnWidths(float NameColumnPercentage, float ValueColumnPercentage)
{
    header()->resizeSection(0, width() * NameColumnPercentage);
    header()->resizeSection(1, width() * ValueColumnPercentage);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
}

void CPropertyView::ClearProperties()
{
    mpObject = nullptr;
    mpModel->ConfigureScript(nullptr, nullptr, nullptr);
}

void CPropertyView::SetIntrinsicProperties(CStructRef InProperties)
{
    mpObject = nullptr;
    mpModel->ConfigureIntrinsic(nullptr, InProperties.Property(), InProperties.DataPointer());
    SetPersistentEditors(QModelIndex());
}

void CPropertyView::SetInstance(CScriptObject *pObj)
{
    mpObject = pObj;
    mpModel->SetBoldModifiedProperties(gpEdApp->CurrentGame() > EGame::Prime);

    if (pObj)
        mpModel->ConfigureScript(pObj->Area()->Entry()->Project(), pObj->Template()->Properties(), pObj);
    else
        mpModel->ConfigureScript(nullptr, nullptr, nullptr);

    SetPersistentEditors(QModelIndex());

    // Auto-expand EditorProperties
    QModelIndex Index = mpModel->index(0, 0, QModelIndex());
    IProperty *pProp = mpModel->PropertyForIndex(Index, false);
    if (pProp && pProp->ID() == 0x255A4580)
        expand(Index);
}

void CPropertyView::UpdateEditorProperties(const QModelIndex& rkParent)
{
    // Check what game this is
    EGame Game = gpEdApp->CurrentGame();

    // Iterate over all properties and update if they're an editor property.
    for (int iRow = 0; iRow < mpModel->rowCount(rkParent); iRow++)
    {
        QModelIndex Index0 = mpModel->index(iRow, 0, rkParent);
        QModelIndex Index1 = mpModel->index(iRow, 1, rkParent);
        IProperty *pProp = mpModel->PropertyForIndex(Index0, false);

        if (pProp)
        {
            // For structs, update sub-properties.
            if (pProp->Type() == EPropertyType::Struct)
            {
                CStructProperty *pStruct = TPropCast<CStructProperty>(pProp);

                // As an optimization, in MP2+, we don't need to update unless this is an atomic struct or if
                // it's EditorProperties, because other structs never have editor properties in them.
                // In MP1 this isn't the case so we need to update every struct regardless
                if ((Game <= EGame::Prime) || (pStruct->IsAtomic() || pStruct->ID() == 0x255A4580))
                    UpdateEditorProperties(Index0);
                else
                    continue;
            }

            else if (mpObject && mpObject->IsEditorProperty(pProp))
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
    uint32 NumChildren = mpModel->rowCount(rkParent);

    for (uint32 iChild = 0; iChild < NumChildren; iChild++)
    {
        QModelIndex ChildIndex = mpModel->index(iChild, 1, rkParent);
        IProperty *pProp = mpModel->PropertyForIndex(ChildIndex, false);
        EPropertyType Type = (pProp ? pProp->Type() : EPropertyType::Invalid);
        bool IsAnimSet = false;

        // Handle persistent editors under character properties
        if (!pProp && ChildIndex.internalId() & 0x80000000)
        {
            pProp = mpModel->PropertyForIndex(ChildIndex, true);

            if (pProp->Type() == EPropertyType::AnimationSet)
            {
                EGame Game = mpObject->Area()->Game();
                Type = mpDelegate->DetermineCharacterPropType(Game, ChildIndex);
                IsAnimSet = true;
            }

            if (pProp->Type() == EPropertyType::Flags)
                Type = EPropertyType::Bool;
        }


        switch (Type)
        {
        case EPropertyType::Bool:
        case EPropertyType::Color:
        case EPropertyType::Asset:
            openPersistentEditor(ChildIndex);
            break;

        case EPropertyType::Enum:
        case EPropertyType::Choice:
            if (IsAnimSet || TPropCast<CEnumProperty>(pProp)->NumPossibleValues() > 0)
                openPersistentEditor(ChildIndex);
            break;

        case EPropertyType::Struct:
            setFirstColumnSpanned(iChild, rkParent, true);
            break;
        }

        if (isExpanded(ChildIndex))
            SetPersistentEditors(ChildIndex);
    }
}

void CPropertyView::ClosePersistentEditors(const QModelIndex& rkIndex)
{
    uint32 NumChildren = mpModel->rowCount(rkIndex);

    for (uint32 iChild = 0; iChild < NumChildren; iChild++)
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
    IProperty* pProperty = mpModel->PropertyForIndex(rkIndex, true);

    if (pProperty->Type() == EPropertyType::AnimationSet /*&& rkIndex.internalId() & 0x1*/)
    {
        ClosePersistentEditors(rkIndex);
        SetPersistentEditors(rkIndex);
    }

    scrollTo(rkIndex);
    emit PropertyModified(rkIndex);
    emit PropertyModified(pProperty);
}

void CPropertyView::RefreshView()
{
    SetInstance(mpObject);
}

void CPropertyView::CreateContextMenu(const QPoint& rkPos)
{
    QModelIndex Index = indexAt(rkPos);

    if (Index.isValid() && Index.column() == 0)
    {
        IProperty* pProperty = mpModel->PropertyForIndex(Index, true);
        mpMenuProperty = pProperty;

        QMenu Menu;

        if (!pProperty->IsIntrinsic())
        {
            Menu.addAction(mpEditTemplateAction);
        }

        if (gpEdApp->CurrentGame() >= EGame::EchoesDemo)
        {
            Menu.addAction(mpShowNameValidityAction);
        }

        // Add options for generating property names
        if (pProperty->UsesNameMap())
        {
            Menu.addSeparator();
            Menu.addAction(mpGenNamesForPropertyAction);

            if (!pProperty->IsRootParent())
            {
                QString TypeName = TO_QSTRING( pProperty->Parent()->RootArchetype()->Name() );
                mpGenNamesForSiblingsAction->setText( QString("Generate names for %1 properties").arg(TypeName) );
                Menu.addAction(mpGenNamesForSiblingsAction);
            }

            if (pProperty->Type() == EPropertyType::Struct && !pProperty->IsAtomic())
            {
                QString TypeName = TO_QSTRING( pProperty->RootArchetype()->Name() );
                mpGenNamesForChildrenAction->setText( QString("Generate names for %1 properties").arg(TypeName) );
                Menu.addAction(mpGenNamesForChildrenAction);
            }
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
    QMainWindow* pParentWindow = UICommon::FindAncestor<QMainWindow>(this);
    CTemplateEditDialog Dialog(mpMenuProperty, pParentWindow);
    connect(&Dialog, SIGNAL(PerformedTypeConversion()), this, SLOT(RefreshView()));
    Dialog.exec();
}


void CPropertyView::GenerateNamesForProperty()
{
    CGeneratePropertyNamesDialog* pDialog = gpEdApp->WorldEditor()->NameGeneratorDialog();
    pDialog->AddToIDPool(mpMenuProperty);
    pDialog->show();
}

void CPropertyView::GenerateNamesForSiblings()
{
    CGeneratePropertyNamesDialog* pDialog = gpEdApp->WorldEditor()->NameGeneratorDialog();
    pDialog->AddChildrenToIDPool(mpMenuProperty->Parent(), false);
    pDialog->show();
}

void CPropertyView::GenerateNamesForChildren()
{
    CGeneratePropertyNamesDialog* pDialog = gpEdApp->WorldEditor()->NameGeneratorDialog();
    pDialog->AddChildrenToIDPool(mpMenuProperty, false);
    pDialog->show();
}
