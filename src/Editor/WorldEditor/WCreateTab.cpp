#include "WCreateTab.h"
#include "ui_WCreateTab.h"
#include "CTemplateMimeData.h"
#include "CWorldEditor.h"
#include "Editor/Undo/UndoCommands.h"
#include <Core/Resource/Script/NGameList.h>

WCreateTab::WCreateTab(CWorldEditor *pEditor, QWidget *pParent /*= 0*/)
    : QWidget(pParent)
    , ui(new Ui::WCreateTab)
    , mpSpawnLayer(nullptr)
{
    ui->setupUi(this);

    mpEditor = pEditor;
    mpEditor->Viewport()->installEventFilter(this);

    connect(mpEditor, SIGNAL(LayersModified()), this, SLOT(OnLayersChanged()));
    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(OnActiveProjectChanged(CGameProject*)));
    connect(ui->SpawnLayerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSpawnLayerChanged(int)));
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
                CCreateInstanceCommand *pCmd = new CCreateInstanceCommand(mpEditor, pMimeData->Template(), mpSpawnLayer, SpawnPoint);
                mpEditor->UndoStack().push(pCmd);
                return true;
            }
        }
    }

    return false;
}

// ************ PUBLIC SLOTS ************
void WCreateTab::OnActiveProjectChanged(CGameProject *pProj)
{
    EGame Game = (pProj ? pProj->Game() : EGame::Invalid);
    CGameTemplate *pGame = NGameList::GetGameTemplate(Game);
    ui->TemplateView->SetGame(pGame);
}

void WCreateTab::OnLayersChanged()
{
    CGameArea *pArea = mpEditor->ActiveArea();

    ui->SpawnLayerComboBox->blockSignals(true);
    ui->SpawnLayerComboBox->clear();

    for (uint32 iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
        ui->SpawnLayerComboBox->addItem(TO_QSTRING(pArea->ScriptLayer(iLyr)->Name()));

    ui->SpawnLayerComboBox->setCurrentIndex(0);
    ui->SpawnLayerComboBox->blockSignals(false);

    OnSpawnLayerChanged(0);
}

void WCreateTab::OnSpawnLayerChanged(int LayerIndex)
{
    CGameArea *pArea = mpEditor->ActiveArea();
    mpSpawnLayer = pArea->ScriptLayer(LayerIndex);
}
