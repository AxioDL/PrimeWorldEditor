#include "WCreateTab.h"
#include "ui_WCreateTab.h"
#include "CTemplateMimeData.h"
#include "CWorldEditor.h"
#include "Editor/Undo/UndoCommands.h"

WCreateTab::WCreateTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WCreateTab)
{
    ui->setupUi(this);
}

WCreateTab::~WCreateTab()
{
    delete ui;
}

bool WCreateTab::eventFilter(QObject *pObj, QEvent *pEvent)
{
    if (pObj == mpEditor->Viewport())
    {
        if (pEvent->type() == QEvent::DragEnter)
        {
            QDragEnterEvent *pDragEvent = static_cast<QDragEnterEvent*>(pEvent);

            if (qobject_cast<const CTemplateMimeData*>(pDragEvent->mimeData()))
            {
                pDragEvent->acceptProposedAction();
                return true;
            }
        }

        else if (pEvent->type() == QEvent::Drop)
        {
            QDropEvent *pDropEvent = static_cast<QDropEvent*>(pEvent);
            const CTemplateMimeData *pMimeData = qobject_cast<const CTemplateMimeData*>(pDropEvent->mimeData());

            if (pMimeData)
            {
                CVector3f SpawnPoint = mpEditor->Viewport()->HoverPoint();
                CCreateInstanceCommand *pCmd = new CCreateInstanceCommand(mpEditor, pMimeData->Template(), mpEditor->ActiveArea()->GetScriptLayer(0), SpawnPoint);
                mpEditor->UndoStack()->push(pCmd);
                return true;
            }
        }
    }

    return false;
}

void WCreateTab::SetEditor(CWorldEditor *pEditor)
{
    mpEditor = pEditor;
    pEditor->Viewport()->installEventFilter(this);
}

void WCreateTab::SetMaster(CMasterTemplate *pMaster)
{
    ui->TemplateView->SetMaster(pMaster);
}
