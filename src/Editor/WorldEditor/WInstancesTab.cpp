#include "WInstancesTab.h"
#include "ui_WInstancesTab.h"

#include "../CWorldEditor.h"
#include <Core/CSceneManager.h>
#include <Resource/script/CScriptLayer.h>

WInstancesTab::WInstancesTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WInstancesTab)
{
    ui->setupUi(this);

    mpEditor = nullptr;
    mpLayersModel = new CTypesInstanceModel(this);
    mpLayersModel->SetModelType(CTypesInstanceModel::eLayers);
    mpTypesModel = new CTypesInstanceModel(this);
    mpTypesModel->SetModelType(CTypesInstanceModel::eTypes);
    ui->LayersTreeView->setModel(mpLayersModel);
    ui->LayersTreeView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->LayersTreeView->resizeColumnToContents(2);
    ui->TypesTreeView->setModel(mpTypesModel);
    ui->TypesTreeView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->TypesTreeView->resizeColumnToContents(2);

    // Create context menu
    mpTreeContextMenu = new QMenu(this);
    mpHideInstance = new QAction("Hide instance", this);
    mpHideType = new QAction("", this);
    mpHideAllExceptType = new QAction("", this);
    mpTreeContextMenu->addAction(mpHideInstance);
    mpTreeContextMenu->addAction(mpHideType);
    mpTreeContextMenu->addAction(mpHideAllExceptType);

    // Configure signals/slots
    connect(ui->LayersTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(OnTreeClick(QModelIndex)));
    connect(ui->LayersTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnTreeDoubleClick(QModelIndex)));
    connect(ui->TypesTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(OnTreeClick(QModelIndex)));
    connect(ui->TypesTreeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnTreeDoubleClick(QModelIndex)));
    connect(mpHideInstance, SIGNAL(triggered()), this, SLOT(OnHideInstanceAction()));
    connect(mpHideType, SIGNAL(triggered()), this, SLOT(OnHideTypeAction()));
    connect(mpHideAllExceptType, SIGNAL(triggered()), this, SLOT(OnHideAllExceptTypeAction()));
}

WInstancesTab::~WInstancesTab()
{
    delete ui;
}

void WInstancesTab::SetEditor(CWorldEditor *pEditor, CSceneManager *pScene)
{
    mpEditor = pEditor;
    mpScene = pScene;
    mpTypesModel->SetEditor(pEditor);
    mpLayersModel->SetEditor(pEditor);
}

void WInstancesTab::SetMaster(CMasterTemplate *pMaster)
{
    mpTypesModel->SetMaster(pMaster);
    ExpandTopLevelItems();
}

void WInstancesTab::SetArea(CGameArea *pArea)
{
    mpLayersModel->SetArea(pArea);
    ExpandTopLevelItems();
}

// ************ PRIVATE SLOTS ************
void WInstancesTab::OnTreeClick(QModelIndex Index)
{
    // Single click is used to process show/hide events
    if (Index.column() == 2)
    {
        // Show/Hide Instance
        if (mpTypesModel->IndexType(Index) == CTypesInstanceModel::eInstanceIndex)
        {
            CScriptObject *pObj = mpTypesModel->IndexObject(Index);
            CScriptNode *pNode = mpScene->NodeForObject(pObj);
            pNode->SetVisible(!pNode->IsVisible());

        }

        // Show/Hide Object Type
        else if (mpTypesModel->IndexType(Index) == CTypesInstanceModel::eObjectTypeIndex)
        {
            if (sender() == ui->LayersTreeView)
            {
                CScriptLayer *pLayer = mpLayersModel->IndexLayer(Index);
                pLayer->SetVisible(!pLayer->IsVisible());
            }

            else if (sender() == ui->TypesTreeView)
            {
                CScriptTemplate *pTmp = mpTypesModel->IndexTemplate(Index);
                pTmp->SetVisible(!pTmp->IsVisible());
            }
        }

        if (sender() == ui->LayersTreeView)
            ui->LayersTreeView->update(Index);
        else if (sender() == ui->TypesTreeView)
            ui->TypesTreeView->update(Index);
    }
}

void WInstancesTab::OnTreeDoubleClick(QModelIndex Index)
{
    CTypesInstanceModel::EIndexType IndexType = mpTypesModel->IndexType(Index);

    if ((mpEditor) && (IndexType == CTypesInstanceModel::eInstanceIndex))
    {
        CTypesInstanceModel::ENodeType NodeType = mpTypesModel->IndexNodeType(Index);
        CSceneNode *pSelectedNode = nullptr;

        if (NodeType == CTypesInstanceModel::eScriptType)
            pSelectedNode = mpScene->NodeForObject( static_cast<CScriptObject*>(Index.internalPointer()) );

        if (pSelectedNode)
        {
            mpEditor->ClearSelection();
            mpEditor->SelectNode(pSelectedNode);
        }
    }
}

void WInstancesTab::OnHideInstanceAction()
{
}

void WInstancesTab::OnHideTypeAction()
{
}

void WInstancesTab::OnHideAllExceptTypeAction()
{
}

// ************ PRIVATE ************
void WInstancesTab::ExpandTopLevelItems()
{
    for (u32 iModel = 0; iModel < 2; iModel++)
    {
        QAbstractItemModel *pModel = (iModel == 0 ? mpLayersModel : mpTypesModel);
        QTreeView *pView = (iModel == 0 ? ui->LayersTreeView : ui->TypesTreeView);
        QModelIndex Index = pModel->index(0,0);

        while (Index.isValid())
        {
            pView->expand(Index);
            Index = Index.sibling(Index.row() + 1, 0);
        }
    }
}
