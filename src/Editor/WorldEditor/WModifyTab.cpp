#include "WModifyTab.h"
#include "ui_WModifyTab.h"

#include "CLinkDialog.h"
#include "CWorldEditor.h"
#include <Core/Scene/CScriptNode.h>

#include <QScrollArea>
#include <QScrollBar>

WModifyTab::WModifyTab(QWidget *pParent)
    : QWidget(pParent)
    , ui(new Ui::WModifyTab)
{
    ui->setupUi(this);

    int PropViewWidth = ui->PropertyView->width();
    ui->PropertyView->header()->resizeSection(0, PropViewWidth * 0.3);
    ui->PropertyView->header()->resizeSection(1, PropViewWidth * 0.3);
    ui->PropertyView->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    mpInLinkModel = new CLinkModel(this);
    mpInLinkModel->SetConnectionType(CLinkModel::eIncoming);
    mpOutLinkModel = new CLinkModel(this);
    mpOutLinkModel->SetConnectionType(CLinkModel::eOutgoing);
    mpLinkDialog = nullptr;

    ui->InLinksTableView->setModel(mpInLinkModel);
    ui->OutLinksTableView->setModel(mpOutLinkModel);
    ui->InLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->OutLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(ui->InLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));
    connect(ui->OutLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));
    connect(ui->AddOutgoingConnectionButton, SIGNAL(clicked()), this, SLOT(OnAddOutgoingLinkClicked()));
    connect(ui->AddIncomingConnectionButton, SIGNAL(clicked()), this, SLOT(OnAddIncomingLinkClicked()));
    connect(ui->DeleteOutgoingConnectionButton, SIGNAL(clicked()), this, SLOT(OnDeleteOutgoingLinkClicked()));
    connect(ui->DeleteIncomingConnectionButton, SIGNAL(clicked()), this, SLOT(OnDeleteIncomingLinkClicked()));

    ClearUI();
}

WModifyTab::~WModifyTab()
{
    delete ui;
}

void WModifyTab::SetEditor(CWorldEditor *pEditor)
{
    mpWorldEditor = pEditor;
    ui->PropertyView->SetEditor(mpWorldEditor);
    connect(mpWorldEditor, SIGNAL(Closed()), this, SLOT(OnWorldEditorClosed()));
    connect(mpWorldEditor, SIGNAL(SelectionTransformed()), this, SLOT(OnWorldSelectionTransformed()));
}

void WModifyTab::GenerateUI(QList<CSceneNode*>& Selection)
{
    if (Selection.size() == 1)
    {
        if (mpSelectedNode != Selection.front())
        {
            mpSelectedNode = Selection.front();

            // todo: set up editing UI for Light Nodes
            if (mpSelectedNode->NodeType() == eScriptNode)
            {
                ui->ObjectsTabWidget->show();
                CScriptNode *pScriptNode = static_cast<CScriptNode*>(mpSelectedNode);
                CScriptObject *pObj = pScriptNode->Object();

                // Set up UI
                ui->PropertyView->SetInstance(pObj);
                mpInLinkModel->SetObject(pObj);
                mpOutLinkModel->SetObject(pObj);
                ui->LightGroupBox->hide();
            }
        }
    }

    else
        ClearUI();
}

void WModifyTab::ClearUI()
{
    ui->ObjectsTabWidget->hide();
    ui->PropertyView->SetInstance(nullptr);
    ui->LightGroupBox->hide();
    mpSelectedNode = nullptr;
}

void WModifyTab::CreateLinkDialog()
{
    if (!mpLinkDialog)
    {
        mpLinkDialog = new CLinkDialog(mpWorldEditor, this);

        if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(mpSelectedNode);
            mpLinkDialog->SetMaster(pScript->Object()->MasterTemplate());
        }

        connect(mpLinkDialog, SIGNAL(accepted()), this, SLOT(OnLinkDialogAccept()));
        connect(mpLinkDialog, SIGNAL(rejected()), this, SLOT(OnLinkDialogReject()));
    }
}

void WModifyTab::DeleteLinkDialog()
{
    if (mpLinkDialog)
    {
        delete mpLinkDialog;
        mpLinkDialog = nullptr;
    }
}

// ************ PUBLIC SLOTS ************
void WModifyTab::OnWorldEditorClosed()
{
    DeleteLinkDialog();
}

void WModifyTab::OnWorldSelectionTransformed()
{
    ui->PropertyView->UpdateEditorProperties(QModelIndex());
}

void WModifyTab::OnAddOutgoingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
        CreateLinkDialog();

        if (mpLinkDialog->Sender() != pInst)
        {
            mpLinkDialog->SetSender(pInst);
            mpLinkDialog->SetReceiver(nullptr);
        }

        mpLinkDialog->show();
    }
}

void WModifyTab::OnAddIncomingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
        CreateLinkDialog();

        if (mpLinkDialog->Receiver() != pInst)
        {
            mpLinkDialog->SetSender(nullptr);
            mpLinkDialog->SetReceiver(pInst);
        }

        mpLinkDialog->show();
    }
}

void WModifyTab::OnDeleteOutgoingLinkClicked()
{
}

void WModifyTab::OnDeleteIncomingLinkClicked()
{
}

void WModifyTab::OnLinkDialogAccept()
{
    DeleteLinkDialog();
}

void WModifyTab::OnLinkDialogReject()
{
    DeleteLinkDialog();
}

// ************ PRIVATE SLOTS ************
void WModifyTab::OnLinkTableDoubleClick(QModelIndex Index)
{
    if (Index.column() == 0)
    {
        // The link table will only be visible if the selected node is a script node
        CScriptNode *pNode = static_cast<CScriptNode*>(mpSelectedNode);
        SLink Link;

        if (sender() == ui->InLinksTableView)
            Link = pNode->Object()->InLink(Index.row());
        else if (sender() == ui->OutLinksTableView)
            Link = pNode->Object()->OutLink(Index.row());

        CScriptNode *pLinkedNode = pNode->Scene()->ScriptNodeByID(Link.ObjectID);

        if (pLinkedNode)
        {
            mpWorldEditor->ClearSelection();
            mpWorldEditor->SelectNode(pLinkedNode);
        }

        ui->InLinksTableView->clearSelection();
        ui->OutLinksTableView->clearSelection();
    }
}
