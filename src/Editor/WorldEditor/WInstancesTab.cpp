#include "Editor/WorldEditor/WInstancesTab.h"
#include "ui_WInstancesTab.h"

#include "Editor/UICommon.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Resource/Script/NGameList.h>
#include <Core/Scene/CScene.h>

#include <QAction>
#include <QMenu>

WInstancesTab::WInstancesTab(CWorldEditor *pEditor, QWidget *parent) :
    QWidget(parent),
    ui(std::make_unique<Ui::WInstancesTab>())
{
    ui->setupUi(this);

    mpEditor = pEditor;
    mpScene = mpEditor->Scene();

    mpLayersModel = new CInstancesModel(pEditor, this);
    mpLayersModel->SetModelType(CInstancesModel::EInstanceModelType::Layers);
    mpTypesModel = new CInstancesModel(pEditor, this);
    mpTypesModel->SetModelType(CInstancesModel::EInstanceModelType::Types);
    mLayersProxyModel.setSourceModel(mpLayersModel);
    mTypesProxyModel.setSourceModel(mpTypesModel);

    connect(mpLayersModel, &CInstancesModel::modelReset, this, &WInstancesTab::ExpandTopLevelItems);
    connect(mpTypesModel, &CInstancesModel::modelReset, this, &WInstancesTab::ExpandTopLevelItems);

    int ColWidth = ui->LayersTreeView->width() * 0.29f;

    ui->LayersTreeView->setModel(&mLayersProxyModel);
    ui->LayersTreeView->resizeColumnToContents(2);
    ui->LayersTreeView->header()->resizeSection(0, ColWidth);
    ui->LayersTreeView->header()->resizeSection(1, ColWidth);
    ui->LayersTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);

    ui->TypesTreeView->setModel(&mTypesProxyModel);
    ui->TypesTreeView->resizeColumnToContents(2);
    ui->TypesTreeView->header()->resizeSection(0, ColWidth);
    ui->TypesTreeView->header()->resizeSection(1, ColWidth);
    ui->TypesTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);

    connect(ui->LayersTreeView, &QTreeView::clicked, this, &WInstancesTab::OnTreeClick);
    connect(ui->LayersTreeView, &QTreeView::doubleClicked, this, &WInstancesTab::OnTreeDoubleClick);
    connect(ui->LayersTreeView, &QTreeView::customContextMenuRequested, this, &WInstancesTab::OnTreeContextMenu);
    connect(ui->TypesTreeView, &QTreeView::clicked, this, &WInstancesTab::OnTreeClick);
    connect(ui->TypesTreeView, &QTreeView::doubleClicked, this, &WInstancesTab::OnTreeDoubleClick);
    connect(ui->TypesTreeView, &QTreeView::customContextMenuRequested, this, &WInstancesTab::OnTreeContextMenu);

    // Create context menu
    mpHideInstance = new QAction(tr("Hide instance"), this);
    mpHideType = new QAction(tr("HideType"), this);
    mpHideAllTypes = new QAction(tr("Hide all types"), this);
    mpHideAllExceptType = new QAction(tr("HideAllButType"), this);
    mpSeparator = new QAction(this);
    mpSeparator->setSeparator(true);
    mpUnhideAllTypes = new QAction(tr("UnhideAllTypes"), this);
    mpUnhideAll = new QAction(tr("Unhide all"), this);


    mpTreeContextMenu = new QMenu(this);
    mpTreeContextMenu->addActions({
        mpHideInstance,
        mpHideType,
        mpHideAllTypes,
        mpHideAllExceptType,
        mpSeparator,
        mpUnhideAllTypes,
        mpUnhideAll,
    });

    connect(mpHideInstance, &QAction::triggered, this, &WInstancesTab::OnHideInstanceAction);
    connect(mpHideType, &QAction::triggered, this, &WInstancesTab::OnHideTypeAction);
    connect(mpHideAllTypes, &QAction::triggered, this, &WInstancesTab::OnHideAllTypesAction);
    connect(mpHideAllExceptType, &QAction::triggered, this, &WInstancesTab::OnHideAllExceptTypeAction);
    connect(mpUnhideAllTypes, &QAction::triggered, this, &WInstancesTab::OnUnhideAllTypes);
    connect(mpUnhideAll, &QAction::triggered, this, &WInstancesTab::OnUnhideAll);
}

WInstancesTab::~WInstancesTab() = default;

// ************ PRIVATE SLOTS ************
void WInstancesTab::OnTreeClick(const QModelIndex& Index)
{
    // Single click is used to process show/hide events
    const QModelIndex SourceIndex = (ui->TabWidget->currentIndex() == 0 ? mLayersProxyModel.mapToSource(Index) : mTypesProxyModel.mapToSource(Index));

    if (SourceIndex.column() == 2)
    {
        const auto IndexType = CInstancesModel::IndexType(SourceIndex);

        // Show/Hide Instance
        if (IndexType == CInstancesModel::EIndexType::Instance)
        {
            if (const CScriptObject* pObj = mpTypesModel->IndexObject(SourceIndex))
            {
                if (CScriptNode* pNode = mpScene->NodeForInstance(pObj))
                    pNode->SetVisible(!pNode->IsVisible());
            }

        }
        // Show/Hide Object Type
        else if (IndexType == CInstancesModel::EIndexType::ObjectType)
        {
            if (sender() == ui->LayersTreeView)
            {
                CScriptLayer *pLayer = mpLayersModel->IndexLayer(SourceIndex);
                pLayer->SetVisible(!pLayer->IsVisible());
            }

            else if (sender() == ui->TypesTreeView)
            {
                CScriptTemplate *pTmp = mpTypesModel->IndexTemplate(SourceIndex);
                pTmp->SetVisible(!pTmp->IsVisible());
            }
        }

        static_cast<QTreeView*>(sender())->update(Index);
    }
}

void WInstancesTab::OnTreeDoubleClick(const QModelIndex& Index)
{
    const QModelIndex SourceIndex = (ui->TabWidget->currentIndex() == 0 ? mLayersProxyModel.mapToSource(Index) : mTypesProxyModel.mapToSource(Index));;
    const auto IndexType = CInstancesModel::IndexType(SourceIndex);

    if (mpEditor && IndexType == CInstancesModel::EIndexType::Instance)
    {
        const auto NodeType = CInstancesModel::IndexNodeType(SourceIndex);

        if (NodeType == ENodeType::Script)
        {
            CSceneNode *pSelectedNode = mpScene->NodeForInstance( static_cast<CScriptObject*>(SourceIndex.internalPointer()) );
            mpEditor->ClearAndSelectNode(pSelectedNode);
        }
    }
}

void WInstancesTab::OnTreeContextMenu(QPoint Pos)
{
    const bool IsLayers = (sender() == ui->LayersTreeView);

    const QModelIndex Index = (IsLayers ? ui->LayersTreeView->indexAt(Pos) : ui->TypesTreeView->indexAt(Pos));
    mMenuIndex = (IsLayers ? mLayersProxyModel.mapToSource(Index) : mTypesProxyModel.mapToSource(Index));

    // Determine type
    mMenuIndexType = CInstancesModel::IndexType(mMenuIndex);

    const CScriptObject *pObject = nullptr;
    mpMenuObject = nullptr;
    mpMenuLayer = nullptr;
    mpMenuTemplate = nullptr;

    if (mMenuIndexType == CInstancesModel::EIndexType::ObjectType)
    {
        pObject = nullptr;
        mpMenuObject = nullptr;

        if (IsLayers)
            mpMenuLayer = mpLayersModel->IndexLayer(mMenuIndex);
        else
            mpMenuTemplate = mpTypesModel->IndexTemplate(mMenuIndex);
    }
    else if (mMenuIndexType == CInstancesModel::EIndexType::Instance)
    {
        pObject = ( IsLayers ? mpLayersModel->IndexObject(mMenuIndex) : mpTypesModel->IndexObject(mMenuIndex) );
        mpMenuObject = mpScene->NodeForInstance(pObject);

        if (IsLayers)
            mpMenuLayer = pObject->Layer();
        else
            mpMenuTemplate = pObject->Template();
    }

    // Set visibility and text
    if (pObject)
    {
        QString Hide = mpMenuObject->MarkedVisible() ? tr("Hide") : tr("Unhide");
        mpHideInstance->setText(tr("%1 instance").arg(Hide));
        mpHideInstance->setVisible(true);
    }
    else
    {
        mpHideInstance->setVisible(false);
    }

    if (mpMenuLayer)
    {
        QString Hide = mpMenuLayer->IsVisible() ? tr("Hide") : tr("Unhide");
        mpHideType->setText(tr("%1 layer %2").arg(Hide).arg(TO_QSTRING(mpMenuLayer->Name())));
        mpHideType->setVisible(true);

        mpHideAllExceptType->setText(tr("Hide all layers but %1").arg(TO_QSTRING(mpMenuLayer->Name())));
        mpHideAllExceptType->setVisible(true);
    }
    else if (mpMenuTemplate)
    {
        QString Hide = mpMenuTemplate->IsVisible() ? tr("Hide") : tr("Unhide");
        mpHideType->setText(tr("%1 all %2 objects").arg(Hide).arg(TO_QSTRING(mpMenuTemplate->Name())));
        mpHideType->setVisible(true);

        mpHideAllExceptType->setText(tr("Hide all types but %1").arg(TO_QSTRING(mpMenuTemplate->Name())));
        mpHideAllExceptType->setVisible(true);
    }
    else
    {
        mpHideType->setVisible(false);
        mpHideAllExceptType->setVisible(false);
    }

    mpHideAllTypes->setText(tr("Hide all %1").arg(IsLayers ? tr("layers") : tr("types")));
    mpUnhideAllTypes->setText(tr("Unhide all %1").arg(IsLayers ? tr("layers") : tr("types")));

    QPoint GlobalPos = static_cast<QTreeView*>(sender())->viewport()->mapToGlobal(Pos);
    mpTreeContextMenu->exec(GlobalPos);
}

void WInstancesTab::OnHideInstanceAction()
{
    const bool IsLayers = (ui->TabWidget->currentIndex() == 0);
    mpMenuObject->SetVisible(!mpMenuObject->MarkedVisible());

    if (IsLayers)
        mpLayersModel->dataChanged(mMenuIndex, mMenuIndex);
    else
        mpTypesModel->dataChanged(mMenuIndex, mMenuIndex);
}

void WInstancesTab::OnHideTypeAction()
{
    const bool IsLayers = (ui->TabWidget->currentIndex() == 0);
    const QModelIndex TypeIndex = (mMenuIndexType == CInstancesModel::EIndexType::Instance ? mMenuIndex.parent() : mMenuIndex);

    if (IsLayers)
    {
        mpMenuLayer->SetVisible(!mpMenuLayer->IsVisible());
        mpLayersModel->dataChanged(TypeIndex, TypeIndex);
    }
    else
    {
        mpMenuTemplate->SetVisible(!mpMenuTemplate->IsVisible());
        mpTypesModel->dataChanged(TypeIndex, TypeIndex);
    }
}

void WInstancesTab::OnHideAllTypesAction()
{
    bool IsLayers = (ui->TabWidget->currentIndex() == 0);
    CInstancesModel *pModel = (IsLayers ? mpLayersModel : mpTypesModel);

    QModelIndex BaseIndex = pModel->index(0, 0);

    for (int iIdx = 0; iIdx < pModel->rowCount(BaseIndex); iIdx++)
    {
        QModelIndex Index = pModel->index(iIdx, 0, BaseIndex);

        if (IsLayers)
        {
            CScriptLayer *pLayer = pModel->IndexLayer(Index);
            pLayer->SetVisible(false);
        }

        else
        {
            CScriptTemplate *pTemplate = pModel->IndexTemplate(Index);
            pTemplate->SetVisible(false);
        }
    }

    pModel->dataChanged(pModel->index(0, 2, BaseIndex), pModel->index(pModel->rowCount(BaseIndex) - 1, 2, BaseIndex));
}

void WInstancesTab::OnHideAllExceptTypeAction()
{
    const bool IsLayers = (ui->TabWidget->currentIndex() == 0);
    const QModelIndex TypeIndex = (mMenuIndexType == CInstancesModel::EIndexType::Instance ? mMenuIndex.parent() : mMenuIndex);
    const QModelIndex TypeParent = TypeIndex.parent();

    if (IsLayers)
    {
        CGameArea* pArea = mpEditor->ActiveArea();

        for (auto* layer : pArea->ScriptLayers())
            layer->SetVisible(layer == mpMenuLayer);

        mpLayersModel->dataChanged(mpLayersModel->index(0, 2, TypeParent), mpLayersModel->index(mpLayersModel->rowCount(TypeParent) - 1, 2, TypeParent));
    }
    else
    {
        const EGame Game = mpEditor->CurrentGame();
        CGameTemplate *pGame = NGameList::GetGameTemplate(Game);

        for (uint32_t iTemp = 0; iTemp < pGame->NumScriptTemplates(); iTemp++)
        {
            CScriptTemplate *pTemplate = pGame->TemplateByIndex(iTemp);
            pTemplate->SetVisible(pTemplate == mpMenuTemplate);
        }

        mpTypesModel->dataChanged(mpTypesModel->index(0, 2, TypeParent), mpTypesModel->index(mpTypesModel->rowCount(TypeParent) - 1, 2, TypeParent));
    }
}

void WInstancesTab::OnUnhideAllTypes()
{
    const bool IsLayers = (ui->TabWidget->currentIndex() == 0);
    const QModelIndex TypeIndex = (mMenuIndexType == CInstancesModel::EIndexType::Instance ? mMenuIndex.parent() : mMenuIndex);
    const QModelIndex TypeParent = TypeIndex.parent();

    if (IsLayers)
    {
        CGameArea* pArea = mpEditor->ActiveArea();

        for (auto* layer : pArea->ScriptLayers())
            layer->SetVisible(true);

        mpLayersModel->dataChanged(mpLayersModel->index(0, 2, TypeParent), mpLayersModel->index(mpLayersModel->rowCount(TypeParent) - 1, 2, TypeParent));
    }
    else
    {
        const EGame Game = mpEditor->CurrentGame();
        CGameTemplate* pGame = NGameList::GetGameTemplate(Game);

        for (uint32_t iTemp = 0; iTemp < pGame->NumScriptTemplates(); iTemp++)
            pGame->TemplateByIndex(iTemp)->SetVisible(true);

        mpTypesModel->dataChanged(mpTypesModel->index(0, 2, TypeParent), mpTypesModel->index(mpTypesModel->rowCount(TypeParent) - 1, 2, TypeParent));
    }
}

void WInstancesTab::OnUnhideAll()
{
    // Unhide instances
    for (auto* node : mpScene->MakeNodeView(ENodeType::Script, true))
        node->SetVisible(true);

    // Unhide layers
    QModelIndex LayersRoot = mpLayersModel->index(0, 0, mpLayersModel->index(0, 0));

    if (LayersRoot.isValid())
    {
        CGameArea* pArea = mpEditor->ActiveArea();

        for (auto* layer : pArea->ScriptLayers())
            layer->SetVisible(true);

        mpLayersModel->dataChanged(mpLayersModel->index(0, 2, LayersRoot), mpLayersModel->index(mpLayersModel->rowCount(LayersRoot) - 1, 2, LayersRoot));
    }

    // Unhide types
    QModelIndex TypesRoot = mpTypesModel->index(0, 0, mpTypesModel->index(0, 0));

    if (TypesRoot.isValid())
    {
        EGame Game = mpEditor->CurrentGame();
        CGameTemplate *pGame = NGameList::GetGameTemplate(Game);

        for (uint32_t iTemp = 0; iTemp < pGame->NumScriptTemplates(); iTemp++)
            pGame->TemplateByIndex(iTemp)->SetVisible(true);

        mpTypesModel->dataChanged(mpTypesModel->index(0, 2, TypesRoot), mpTypesModel->index(mpTypesModel->rowCount(TypesRoot) - 1, 2, TypesRoot));
    }

    // Emit data changed on all instances
    for (int iModel = 0; iModel < 2; iModel++)
    {
        CInstancesModel *pModel = (iModel == 0 ? mpLayersModel : mpTypesModel);

        const QModelIndex Base = pModel->index(0, 0);
        const int NumRows = pModel->rowCount(Base);

        for (int iRow = 0; iRow < NumRows; iRow++)
        {
            const QModelIndex RowIndex = pModel->index(iRow, 2, Base);
            pModel->dataChanged(pModel->index(0, 2, RowIndex), pModel->index(pModel->rowCount(RowIndex) - 1, 2, RowIndex));
        }
    }

    OnUnhideAllTypes();
}

void WInstancesTab::ExpandTopLevelItems()
{
    for (int iModel = 0; iModel < 2; iModel++)
    {
        const QAbstractItemModel* pModel = (iModel == 0 ? &mLayersProxyModel : &mTypesProxyModel);
        QTreeView *pView = (iModel == 0 ? ui->LayersTreeView : ui->TypesTreeView);
        QModelIndex Index = pModel->index(0,0);

        while (Index.isValid())
        {
            pView->expand(Index);
            Index = Index.sibling(Index.row() + 1, 0);
        }
    }
}
