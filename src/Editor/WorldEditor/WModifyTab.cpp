#include "WModifyTab.h"
#include "ui_WModifyTab.h"

#include "CLinkDialog.h"
#include "CSelectInstanceDialog.h"
#include "CWorldEditor.h"
#include "Editor/Undo/UndoCommands.h"
#include <Core/Scene/CScriptNode.h>

#include <QScrollArea>
#include <QScrollBar>

WModifyTab::WModifyTab(CWorldEditor *pEditor, QWidget *pParent)
    : QWidget(pParent)
    , ui(std::make_unique<Ui::WModifyTab>())
{
    ui->setupUi(this);
    ui->PropertyView->SetEditor(pEditor);

    mpWorldEditor = pEditor;

    mpInLinkModel = new CLinkModel(this);
    mpInLinkModel->SetConnectionType(ELinkType::Incoming);
    mpOutLinkModel = new CLinkModel(this);
    mpOutLinkModel->SetConnectionType(ELinkType::Outgoing);

    mpAddFromViewportAction = new QAction(tr("Choose from viewport"), this);
    mpAddFromListAction = new QAction(tr("Choose from list"), this);
    mpAddLinkMenu = new QMenu(this);
    mpAddLinkMenu->addAction(mpAddFromViewportAction);
    mpAddLinkMenu->addAction(mpAddFromListAction);
    ui->AddOutgoingConnectionToolButton->setMenu(mpAddLinkMenu);
    ui->AddIncomingConnectionToolButton->setMenu(mpAddLinkMenu);

    ui->InLinksTableView->setModel(mpInLinkModel);
    ui->OutLinksTableView->setModel(mpOutLinkModel);
    connect(ui->InLinksTableView, &QTableView::doubleClicked, this, &WModifyTab::OnLinkTableDoubleClick);
    connect(ui->OutLinksTableView, &QTableView::doubleClicked, this, &WModifyTab::OnLinkTableDoubleClick);
    connect(ui->InLinksTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WModifyTab::OnLinksSelectionModified);
    connect(ui->OutLinksTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WModifyTab::OnLinksSelectionModified);
    connect(ui->AddOutgoingConnectionToolButton, &QToolButton::triggered, this, &WModifyTab::OnAddLinkActionClicked);
    connect(ui->AddIncomingConnectionToolButton, &QToolButton::triggered, this, &WModifyTab::OnAddLinkActionClicked);
    connect(ui->DeleteOutgoingConnectionButton, &QToolButton::clicked, this, &WModifyTab::OnDeleteLinksClicked);
    connect(ui->DeleteIncomingConnectionButton, &QToolButton::clicked, this, &WModifyTab::OnDeleteLinksClicked);
    connect(ui->EditOutgoingConnectionButton, &QToolButton::clicked, this, &WModifyTab::OnEditLinkClicked);
    connect(ui->EditIncomingConnectionButton, &QToolButton::clicked, this, &WModifyTab::OnEditLinkClicked);
    connect(ui->PropertyView, qOverload<IProperty*>(&CPropertyView::PropertyModified), mpWorldEditor, &CWorldEditor::OnPropertyModified);
    connect(mpWorldEditor, &CWorldEditor::MapChanged, this, &WModifyTab::OnMapChanged);
    connect(mpWorldEditor, &CWorldEditor::SelectionTransformed, this, &WModifyTab::OnWorldSelectionTransformed);
    connect(mpWorldEditor, &CWorldEditor::InstanceLinksModified, this, &WModifyTab::OnInstanceLinksModified);
    connect(mpWorldEditor->Selection(), &CNodeSelection::Modified, this, &WModifyTab::GenerateUI);

    ClearUI();
}

WModifyTab::~WModifyTab() = default;

void WModifyTab::ClearUI()
{
    ui->ObjectsTabWidget->hide();
    ui->PropertyView->SetInstance(nullptr);
    mpSelectedNode = nullptr;
}

CPropertyView* WModifyTab::PropertyView() const
{
    return ui->PropertyView;
}

// ************ PUBLIC SLOTS ************
void WModifyTab::GenerateUI()
{
    if (mIsPicking)
        mpWorldEditor->ExitPickMode();

    if (mpWorldEditor->Selection()->Size() == 1)
    {
        if (mpSelectedNode != mpWorldEditor->Selection()->Front())
        {
            mpSelectedNode = mpWorldEditor->Selection()->Front();

            if (mpSelectedNode->NodeType() == ENodeType::Script)
            {
                CScriptNode *pScriptNode = static_cast<CScriptNode*>(mpSelectedNode);
                CScriptObject *pObj = pScriptNode->Instance();

                // Set up UI
                ui->ObjectsTabWidget->show();
                ui->PropertyView->SetInstance(pObj);
                mpInLinkModel->SetObject(pObj);
                mpOutLinkModel->SetObject(pObj);
            }
            // disabled this for now! implemented it as a quick test, it's cool it works,
            // but it's buggy & not ready for deployment
#if 0
            else
            {
                CStructRef Properties = mpSelectedNode->GetProperties();
                ui->PropertyView->SetProperties(Properties);

                if (Properties.IsValid())
                    ui->ObjectsTabWidget->show();
            }
#endif

            ui->InLinksTableView->clearSelection();
            ui->OutLinksTableView->clearSelection();
            ui->InLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            ui->OutLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
        }
    }

    else
        ClearUI();
}

void WModifyTab::OnInstanceLinksModified(const QList<CScriptObject*>& rkInstances)
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == ENodeType::Script)
    {
        CScriptObject *pInstance = static_cast<CScriptNode*>(mpSelectedNode)->Instance();

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

void WModifyTab::OnMapChanged()
{
    ClearUI();
}

void WModifyTab::OnLinksSelectionModified()
{
    if (sender() == ui->InLinksTableView->selectionModel())
    {
        uint32 NumSelectedRows = ui->InLinksTableView->selectionModel()->selectedRows().size();
        ui->EditIncomingConnectionButton->setEnabled(NumSelectedRows == 1);
        ui->DeleteIncomingConnectionButton->setEnabled(NumSelectedRows > 0);
    }
    else
    {
        uint32 NumSelectedRows = ui->OutLinksTableView->selectionModel()->selectedRows().size();
        ui->EditOutgoingConnectionButton->setEnabled(NumSelectedRows == 1);
        ui->DeleteOutgoingConnectionButton->setEnabled(NumSelectedRows > 0);
    }
}

void WModifyTab::OnAddLinkActionClicked(QAction *pAction)
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == ENodeType::Script)
    {
        mAddLinkType = (sender() == ui->AddOutgoingConnectionToolButton ? ELinkType::Outgoing : ELinkType::Incoming);

        if (pAction == mpAddFromViewportAction)
        {
            mpWorldEditor->EnterPickMode(ENodeType::Script, true, false, false);
            connect(mpWorldEditor, &CWorldEditor::PickModeClick, this, &WModifyTab::OnPickModeClick);
            connect(mpWorldEditor, &CWorldEditor::PickModeExited, this, &WModifyTab::OnPickModeExit);
            mIsPicking = true;
        }

        else if (pAction == mpAddFromListAction)
        {
            if (mIsPicking)
                mpWorldEditor->ExitPickMode();

            CSelectInstanceDialog Dialog(mpWorldEditor, this);
            Dialog.exec();
            CScriptObject *pTarget = Dialog.SelectedInstance();

            if (pTarget)
            {
                CLinkDialog *pLinkDialog = mpWorldEditor->LinkDialog();
                CScriptObject *pSelected = static_cast<CScriptNode*>(mpSelectedNode)->Instance();

                CScriptObject *pSender      = (mAddLinkType == ELinkType::Outgoing ? pSelected : pTarget);
                CScriptObject *pReceiver    = (mAddLinkType == ELinkType::Outgoing ? pTarget : pSelected);
                pLinkDialog->NewLink(pSender, pReceiver);
                pLinkDialog->show();
            }
        }
    }
}

void WModifyTab::OnPickModeClick(const SRayIntersection& rkIntersect)
{
    mpWorldEditor->ExitPickMode();
    CScriptObject *pTarget = static_cast<CScriptNode*>(rkIntersect.pNode)->Instance();

    if (pTarget)
    {
        CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
        CScriptObject *pSelected = static_cast<CScriptNode*>(mpSelectedNode)->Instance();

        CScriptObject *pSender      = (mAddLinkType == ELinkType::Outgoing ? pSelected : pTarget);
        CScriptObject *pReceiver    = (mAddLinkType == ELinkType::Outgoing ? pTarget : pSelected);
        pDialog->NewLink(pSender, pReceiver);
        pDialog->show();
    }
}

void WModifyTab::OnPickModeExit()
{
    disconnect(mpWorldEditor, &CWorldEditor::PickModeClick, this, nullptr);
    disconnect(mpWorldEditor, &CWorldEditor::PickModeExited, this, nullptr);
    mIsPicking = false;
}

void WModifyTab::OnDeleteLinksClicked()
{
    if (mpSelectedNode == nullptr || mpSelectedNode->NodeType() != ENodeType::Script)
        return;

    const ELinkType Type = (sender() == ui->DeleteOutgoingConnectionButton ? ELinkType::Outgoing : ELinkType::Incoming);
    const QModelIndexList SelectedIndices = (Type == ELinkType::Outgoing ? ui->OutLinksTableView->selectionModel()->selectedRows() : ui->InLinksTableView->selectionModel()->selectedRows());

    if (SelectedIndices.isEmpty())
        return;

    QVector<uint32> Indices;
    Indices.reserve(SelectedIndices.size());
    for (const auto& index : SelectedIndices)
        Indices.push_back(index.row());

    auto *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Instance();
    auto *pCmd = new CDeleteLinksCommand(mpWorldEditor, pInst, Type, Indices);
    mpWorldEditor->UndoStack().push(pCmd);
}

void WModifyTab::OnEditLinkClicked()
{
    if (mpSelectedNode && mpSelectedNode->NodeType() == ENodeType::Script)
    {
        ELinkType Type = (sender() == ui->EditOutgoingConnectionButton ? ELinkType::Outgoing : ELinkType::Incoming);
        QModelIndexList SelectedIndices = (Type == ELinkType::Outgoing ? ui->OutLinksTableView->selectionModel()->selectedRows() : ui->InLinksTableView->selectionModel()->selectedRows());

        if (SelectedIndices.size() == 1)
        {
            CScriptObject *pInst = static_cast<CScriptNode*>(mpSelectedNode)->Instance();
            CLinkDialog *pDialog = mpWorldEditor->LinkDialog();
            pDialog->EditLink(pInst->Link(Type, SelectedIndices.front().row()));
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
        uint32 InstanceID = 0;

        if (sender() == ui->InLinksTableView)
            InstanceID = pNode->Instance()->Link(ELinkType::Incoming, Index.row())->SenderID();
        else if (sender() == ui->OutLinksTableView)
            InstanceID = pNode->Instance()->Link(ELinkType::Outgoing, Index.row())->ReceiverID();

        CScriptNode *pLinkedNode = pNode->Scene()->NodeForInstanceID(InstanceID);

        if (pLinkedNode)
            mpWorldEditor->ClearAndSelectNode(pLinkedNode);

        ui->InLinksTableView->clearSelection();
        ui->OutLinksTableView->clearSelection();
    }
}
