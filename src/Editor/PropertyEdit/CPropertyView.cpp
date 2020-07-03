#include "CPropertyView.h"
#include "CPropertyDelegate.h"
#include "Editor/WorldEditor/CTemplateEditDialog.h"
#include <Core/Resource/Script/Property/Properties.h>

#include <QEvent>
#include <QMenu>
#include <QToolTip>

CPropertyView::CPropertyView(QWidget *pParent)
    : QTreeView(pParent)
{
    mpModel = new CPropertyModel(this);
    mpDelegate = new CPropertyDelegate(this);
    setItemDelegateForColumn(1, mpDelegate);
    setEditTriggers(AllEditTriggers);
    CPropertyView::setModel(mpModel);

    setContextMenuPolicy(Qt::CustomContextMenu);

    mpShowNameValidityAction = new QAction(tr("Show whether property name is correct"), this);
    mpShowNameValidityAction->setCheckable(true);
    mpShowNameValidityAction->setChecked(false);
    connect(mpShowNameValidityAction, &QAction::triggered, this, &CPropertyView::ToggleShowNameValidity);

    if (gTemplatesWritable)
    {
        mpEditTemplateAction = new QAction(tr("Edit template"), this);
        connect(mpEditTemplateAction, &QAction::triggered, this, &CPropertyView::EditPropertyTemplate);
    }
    else
    {
        mpEditTemplateAction = new QAction(tr("Template files not writable"), this);
        mpEditTemplateAction->setEnabled(false);
    }

    mpGenNamesForPropertyAction = new QAction(tr("Generate names for this property"), this);
    mpGenNamesForSiblingsAction = new QAction(this); // Text set in CreateContextMenu()
    mpGenNamesForChildrenAction = new QAction(this); // Text set in CreateContextMenu()
    connect(mpGenNamesForPropertyAction, &QAction::triggered, this, &CPropertyView::GenerateNamesForProperty);
    connect(mpGenNamesForSiblingsAction, &QAction::triggered, this, &CPropertyView::GenerateNamesForSiblings);
    connect(mpGenNamesForChildrenAction, &QAction::triggered, this, &CPropertyView::GenerateNamesForChildren);

    connect(this, &CPropertyView::expanded, this, &CPropertyView::SetPersistentEditors);
    connect(this, &CPropertyView::clicked, [this](const QModelIndex& index) { edit(index); });
    connect(this, &CPropertyView::customContextMenuRequested, this, &CPropertyView::CreateContextMenu);
    connect(mpModel, &CPropertyModel::rowsInserted, [this](const QModelIndex& index, int, int) { SetPersistentEditors(index); });
    connect(mpModel, &CPropertyModel::PropertyModified, this, &CPropertyView::OnPropertyModified);
}

void CPropertyView::setModel(QAbstractItemModel *pModel)
{
    CPropertyModel *pPropModel = qobject_cast<CPropertyModel*>(pModel);
    mpModel = pPropModel;
    mpDelegate->SetPropertyModel(pPropModel);
    QTreeView::setModel(pPropModel);

    if (pPropModel == nullptr)
        return;

    const QModelIndex Root = pPropModel->index(0, 0, QModelIndex());
    SetPersistentEditors(Root);
    setExpanded(Root, true);
}

bool CPropertyView::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ToolTip)
    {
        const QPoint MousePos = QCursor::pos();
        const QModelIndex Index = indexAt(viewport()->mapFromGlobal(MousePos));

        if (Index.isValid())
        {
            const QString Desc = mpModel->data(Index, Qt::ToolTipRole).toString();

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

    if (pEvent->type() == QEvent::Resize && !isVisible())
    {
        resizeColumnToContents(0);
    }

    return QTreeView::event(pEvent);
}

int CPropertyView::sizeHintForColumn(int Column) const
{
    if (Column == 0)
        return static_cast<int>(width() * 0.6f);
    else
        return static_cast<int>(width() * 0.4f);
}

void CPropertyView::SetEditor(IEditor* pEditor)
{
    mpDelegate->SetEditor(pEditor);
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

    if (pObj != nullptr)
        mpModel->ConfigureScript(pObj->Area()->Entry()->Project(), pObj->Template()->Properties(), pObj);
    else
        mpModel->ConfigureScript(nullptr, nullptr, nullptr);

    SetPersistentEditors(QModelIndex());

    // Auto-expand EditorProperties
    const QModelIndex Index = mpModel->index(0, 0, QModelIndex());
    const IProperty *pProp = mpModel->PropertyForIndex(Index, false);
    if (pProp != nullptr && pProp->ID() == 0x255A4580)
        expand(Index);
}

void CPropertyView::UpdateEditorProperties(const QModelIndex& rkParent)
{
    // Check what game this is
    const EGame Game = gpEdApp->CurrentGame();

    // Iterate over all properties and update if they're an editor property.
    for (int iRow = 0; iRow < mpModel->rowCount(rkParent); iRow++)
    {
        const QModelIndex Index0 = mpModel->index(iRow, 0, rkParent);
        const QModelIndex Index1 = mpModel->index(iRow, 1, rkParent);
        IProperty* pProp = mpModel->PropertyForIndex(Index0, false);

        if (pProp == nullptr)
            continue;

        // For structs, update sub-properties.
        if (pProp->Type() == EPropertyType::Struct)
        {
            const CStructProperty *pStruct = TPropCast<CStructProperty>(pProp);

            // As an optimization, in MP2+, we don't need to update unless this is an atomic struct or if
            // it's EditorProperties, because other structs never have editor properties in them.
            // In MP1 this isn't the case so we need to update every struct regardless
            if (Game <= EGame::Prime || (pStruct->IsAtomic() || pStruct->ID() == 0x255A4580))
                UpdateEditorProperties(Index0);
            else
                continue;
        }
        else if (mpObject != nullptr && mpObject->IsEditorProperty(pProp))
        {
            mpModel->dataChanged(Index1, Index1);

            if (mpModel->rowCount(Index0) != 0)
            {
                const QModelIndex SubIndexA = mpModel->index(0, 1, Index0);
                const QModelIndex SubIndexB = mpModel->index(mpModel->rowCount(Index0) - 1, 1, Index0);
                mpModel->dataChanged(SubIndexA, SubIndexB);
            }
        }
    }
}

void CPropertyView::SetPersistentEditors(const QModelIndex& rkParent)
{
    const int NumChildren = mpModel->rowCount(rkParent);

    for (int iChild = 0; iChild < NumChildren; iChild++)
    {
        const QModelIndex ChildIndex = mpModel->index(iChild, 1, rkParent);
        IProperty *pProp = mpModel->PropertyForIndex(ChildIndex, false);
        EPropertyType Type = (pProp ? pProp->Type() : EPropertyType::Invalid);
        bool IsAnimSet = false;

        // Handle persistent editors under character properties
        if (!pProp && (ChildIndex.internalId() & 0x80000000) != 0)
        {
            pProp = mpModel->PropertyForIndex(ChildIndex, true);

            if (pProp->Type() == EPropertyType::AnimationSet)
            {
                Type = mpDelegate->DetermineCharacterPropType(pProp->Game(), ChildIndex);
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

        default:
            break;
        }

        if (isExpanded(ChildIndex))
            SetPersistentEditors(ChildIndex);
    }
}

void CPropertyView::ClosePersistentEditors(const QModelIndex& rkIndex)
{
    const int NumChildren = mpModel->rowCount(rkIndex);

    for (int iChild = 0; iChild < NumChildren; iChild++)
    {
        const QModelIndex ChildIndex = mpModel->index(iChild, 1, rkIndex);
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
    const QModelIndex Index = indexAt(rkPos);

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
                const QString TypeName = TO_QSTRING(pProperty->Parent()->RootArchetype()->Name());
                mpGenNamesForSiblingsAction->setText(tr("Generate names for %1 properties").arg(TypeName));
                Menu.addAction(mpGenNamesForSiblingsAction);
            }

            if (pProperty->Type() == EPropertyType::Struct && !pProperty->IsAtomic())
            {
                const QString TypeName = TO_QSTRING(pProperty->RootArchetype()->Name());
                mpGenNamesForChildrenAction->setText(tr("Generate names for %1 properties").arg(TypeName));
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
    connect(&Dialog, &CTemplateEditDialog::PerformedTypeConversion, this, &CPropertyView::RefreshView);
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
