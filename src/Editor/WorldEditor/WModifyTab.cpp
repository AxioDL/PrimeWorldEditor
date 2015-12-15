#include "WModifyTab.h"
#include "ui_WModifyTab.h"
#include <Scene/CScriptNode.h>
#include <QScrollArea>
#include <QScrollBar>
#include "../CWorldEditor.h"

WModifyTab::WModifyTab(QWidget *pParent) :
    QWidget(pParent),
    ui(new Ui::WModifyTab)
{
    ui->setupUi(this);

    mpCurPropEditor = nullptr;

    mpInLinkModel = new CLinkModel(this);
    mpInLinkModel->SetConnectionType(CLinkModel::eIncoming);
    mpOutLinkModel = new CLinkModel(this);
    mpOutLinkModel->SetConnectionType(CLinkModel::eOutgoing);

    ui->InLinksTableView->setModel(mpInLinkModel);
    ui->OutLinksTableView->setModel(mpOutLinkModel);
    ui->InLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->OutLinksTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(ui->InLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));
    connect(ui->OutLinksTableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnLinkTableDoubleClick(QModelIndex)));

    ClearUI();
}

WModifyTab::~WModifyTab()
{
    delete ui;
}

void WModifyTab::SetEditor(CWorldEditor *pEditor)
{
    mpWorldEditor = pEditor;
}

void WModifyTab::GenerateUI(QList<CSceneNode*>& Selection)
{
    WPropertyEditor *pOldEditor = mpCurPropEditor;
    ClearUI();

    if (Selection.size() == 1)
    {
        mpSelectedNode = Selection.front();

        // todo: set up editing UI for Light Nodes
        if (mpSelectedNode->NodeType() == eScriptNode)
        {
            ui->ObjectsTabWidget->show();
            CScriptNode *pScriptNode = static_cast<CScriptNode*>(mpSelectedNode);
            CScriptObject *pObj = pScriptNode->Object();
            CScriptTemplate *pTemplate = pObj->Template();
            CPropertyStruct *pProperties = pObj->Properties();

            // Check whether a cached UI for this object exists
            auto it = mCachedPropEditors.find(pTemplate);

            // Load precached UI
            if (it != mCachedPropEditors.end())
            {
                mpCurPropEditor = *it;
                mpCurPropEditor->SetProperty(pProperties);
            }

            // Generate new UI
            else
            {
                mpCurPropEditor = new WPropertyEditor(ui->PropertiesScrollContents, pProperties);
                mCachedPropEditors[pTemplate] = mpCurPropEditor;
            }

            ui->PropertiesScrollLayout->insertWidget(0, mpCurPropEditor);
            mpCurPropEditor->show();

            // Scroll back up to the top, but only if this is a new editor.
            // (This is so clicking on multiple objects of the same type, or even
            // the same object twice, won't cause you to lose your place.)
            if (pOldEditor != mpCurPropEditor)
            {
                ui->PropertiesScrollArea->horizontalScrollBar()->setValue(0);
                ui->PropertiesScrollArea->verticalScrollBar()->setValue(0);
            }

            // Set up connection table model
            mpInLinkModel->SetObject(pObj);
            mpOutLinkModel->SetObject(pObj);
        }
    }

    else
        ClearUI();
}

void WModifyTab::ClearUI()
{
    if (mpCurPropEditor)
    {
        ui->PropertiesScrollLayout->removeWidget(mpCurPropEditor);
        mpCurPropEditor->hide();
        mpCurPropEditor = nullptr;
    }

    ui->ObjectsTabWidget->hide();
    ui->LightGroupBox->hide();
}

void WModifyTab::ClearCachedEditors()
{
    foreach(WPropertyEditor *pEditor, mCachedPropEditors)
        delete pEditor;

    mCachedPropEditors.clear();
}

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
        else
            std::cout << "Error - OnLinkTableDoubleClick() activated by invalid sender\n";

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
