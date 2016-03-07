#include "WModifyTab.h"
#include "ui_WModifyTab.h"

#include "CLinkDialog.h"
#include "CWorldEditor.h"
#include "Editor/Undo/UndoCommands.h"
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
    mpInLinkModel->SetConnectionType(eIncoming);
    mpOutLinkModel = new CLinkModel(this);
    mpOutLinkModel->SetConnectionType(eOutgoing);

    ui->InLinksTableView->setModel(mpInLinkModel);
    ui->OutLinksTableView->setModel(mpOutLinkModel);
    ui->InLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->OutLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(ui->InLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));
    connect(ui->OutLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));
    connect(ui->InLinksTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnIncomingLinksSelectionModified()));
    connect(ui->OutLinksTableView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(OnOutgoingLinksSelectionModified()));
    connect(ui->AddOutgoingConnectionButton, SIGNAL(clicked()), this, SLOT(OnAddOutgoingLinkClicked()));
    connect(ui->AddIncomingConnectionButton, SIGNAL(clicked()), this, SLOT(OnAddIncomingLinkClicked()));
    connect(ui->DeleteOutgoingConnectionButton, SIGNAL(clicked()), this, SLOT(OnDeleteOutgoingLinkClicked()));
    connect(ui->DeleteIncomingConnectionButton, SIGNAL(clicked()), this, SLOT(OnDeleteIncomingLinkClicked()));
    connect(ui->EditOutgoingConnectionButton, SIGNAL(clicked()), this, SLOT(OnEditOutgoingLinkClicked()));
    connect(ui->EditIncomingConnectionButton, SIGNAL(clicked()), this, SLOT(OnEditIncomingLinkClicked()));

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
    connect(mpWorldEditor, SIGNAL(SelectionTransformed()), this, SLOT(OnWorldSelectionTransformed()));
    connect(mpWorldEditor, SIGNAL(InstanceLinksModified(const QList<CScriptObject*>&)), this, SLOT(OnInstanceLinksModified(const QList<CScriptObject*>&)));
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
                ui->LightGroupBox->hide();

                ui->InLinksTableView->clearSelection();
                ui->OutLinksTableView->clearSelection();
                mpInLinkModel->SetObject(pObj);
                mpOutLinkModel->SetObject(pObj);
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

// ************ PUBLIC SLOTS ************
void WModifyTab::OnInstanceLinksModified(const QList<CScriptObject*>& rkInstances)
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        CScriptObject *pInstance = static_cast<CScriptNode*>(mpSelectedNode)->Object();

        if (pInstance && rkInstances.contains(pInstance))
        {
            mpInLinkModel->layoutChanged();
            mpOutLinkModel->layoutChanged();
            ui->InLinksTableView->clearSelection();
            ui->OutLinksTableView->clearSelection();
        }
    }
}

void WModifyTab::OnWorldSelectionTransformed()
{
    ui->PropertyView->UpdateEditorProperties(QModelIndex());
}

void WModifyTab::OnOutgoingLinksSelectionModified()
{
    u32 NumSelectedRows = ui->OutLinksTableView->selectionModel()->selectedRows().size();
    ui->EditOutgoingConnectionButton->setEnabled(NumSelectedRows == 1);
}

void WModifyTab::OnIncomingLinksSelectionModified()
{
    u32 NumSelectedRows = ui->InLinksTableView->selectionModel()->selectedRows().size();
    ui->EditIncomingConnectionButton->setEnabled(NumSelectedRows == 1);
}

void WModifyTab::OnAddOutgoingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
        CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
        pDialog->NewLink(pInst, nullptr);
        pDialog->show();
    }
}

void WModifyTab::OnAddIncomingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
        CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
        pDialog->NewLink(nullptr, pInst);
        pDialog->show();
    }
}

void WModifyTab::OnDeleteOutgoingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        QModelIndexList SelectedIndices = ui->OutLinksTableView->selectionModel()->selectedRows();

        if (!SelectedIndices.isEmpty())
        {
            QVector<u32> Indices;

            for (int iIdx = 0; iIdx < SelectedIndices.size(); iIdx++)
                Indices << SelectedIndices[iIdx].row();

            CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
            CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(mpWorldEditor, pInst, eOutgoing, Indices);
            mpWorldEditor->UndoStack()->push(pCmd);
        }
    }
}

void WModifyTab::OnDeleteIncomingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        QModelIndexList SelectedIndices = ui->InLinksTableView->selectionModel()->selectedRows();

        if (!SelectedIndices.isEmpty())
        {
            QVector<u32> Indices;

            for (int iIdx = 0; iIdx < SelectedIndices.size(); iIdx++)
                Indices << SelectedIndices[iIdx].row();

            CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
            CDeleteLinksCommand *pCmd = new CDeleteLinksCommand(mpWorldEditor, pInst, eIncoming, Indices);
            mpWorldEditor->UndoStack()->push(pCmd);
        }
    }
}

void WModifyTab::OnEditOutgoingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        QModelIndexList SelectedIndices = ui->OutLinksTableView->selectionModel()->selectedRows();

        if (SelectedIndices.size() == 1)
        {
            CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
            CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
            pDialog->EditLink(pInst->Link(eOutgoing, SelectedIndices.front().row()));
            pDialog->show();
        }
    }
}

void WModifyTab::OnEditIncomingLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == eScriptNode)
    {
        QModelIndexList SelectedIndices = ui->InLinksTableView->selectionModel()->selectedRows();

        if (SelectedIndices.size() == 1)
        {
            CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Object();
            CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
            pDialog->EditLink(pInst->Link(eIncoming, SelectedIndices.front().row()));
            pDialog->show();
        }
    }
}

// ************ PRIVATE SLOTS ************
void WModifyTab::OnLinkTableDoubleClick(QModelIndex Index)
{
    if (Index.column() == 0)
    {
        // The link table will only be visible if the selected node is a script node
        CScriptNode *pNode = static_cast<CScriptNode*>(mpSelectedNode);
        u32 InstanceID;

        if (sender() == ui->InLinksTableView)
            InstanceID = pNode->Object()->Link(eIncoming, Index.row())->SenderID();
        else if (sender() == ui->OutLinksTableView)
            InstanceID = pNode->Object()->Link(eOutgoing, Index.row())->ReceiverID();

        CScriptNode *pLinkedNode = pNode->Scene()->ScriptNodeByID(InstanceID);

        if (pLinkedNode)
        {
            mpWorldEditor->ClearSelection();
            mpWorldEditor->SelectNode(pLinkedNode);
        }

        ui->InLinksTableView->clearSelection();
        ui->OutLinksTableView->clearSelection();
    }
}
