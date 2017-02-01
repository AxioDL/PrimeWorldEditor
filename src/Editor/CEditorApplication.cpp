#include "CEditorApplication.h"
#include "IEditor.h"
#include "CBasicViewport.h"
#include "CProjectOverviewDialog.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Common/AssertMacro.h>
#include <Common/CTimer.h>
#include <Core/GameProject/CGameProject.h>

CEditorApplication::CEditorApplication(int& rArgc, char **ppArgv)
    : QApplication(rArgc, ppArgv)
    , mpWorldEditor(nullptr)
    , mpResourceBrowser(nullptr)
    , mpProjectDialog(nullptr)
{
    mLastUpdate = CTimer::GlobalTime();

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(TickEditors()));
    mRefreshTimer.start(8);
}

CEditorApplication::~CEditorApplication()
{
    delete mpWorldEditor;
    delete mpResourceBrowser;
    delete mpProjectDialog;
}

void CEditorApplication::InitEditor()
{
    mpWorldEditor = new CWorldEditor();
    mpResourceBrowser = new CResourceBrowser(mpWorldEditor);
    mpProjectDialog = new CProjectOverviewDialog();
    connect(mpProjectDialog, SIGNAL(ActiveProjectChanged(CGameProject*)), mpResourceBrowser, SLOT(RefreshResources()));
}

void CEditorApplication::AddEditor(IEditor *pEditor)
{
    mEditorWindows << pEditor;
    connect(pEditor, SIGNAL(Closed()), this, SLOT(OnEditorClose()));
}

void CEditorApplication::TickEditors()
{
    double LastUpdate = mLastUpdate;
    mLastUpdate = CTimer::GlobalTime();
    double DeltaTime = mLastUpdate - LastUpdate;

    // The resource store should NOT be dirty at the beginning of a tick - this indicates we forgot to save it after updating somewhere
    if (gpResourceStore && gpResourceStore->IsDirty())
    {
        Log::Error("Resource store is dirty at the beginning of a tick! Call ConditionalSaveStore() after making any significant changes to assets!");
        DEBUG_BREAK;
        gpResourceStore->ConditionalSaveStore();
    }

    // Tick each editor window and redraw their viewports
    foreach(IEditor *pEditor, mEditorWindows)
    {
        if (pEditor->isVisible())
        {
            pEditor->EditorTick((float) DeltaTime);

            CBasicViewport *pViewport = pEditor->Viewport();

            if (pViewport && pViewport->isVisible() && !pEditor->isMinimized())
            {
                pViewport->ProcessInput();
                pViewport->Render();
            }
        }
    }
}

void CEditorApplication::OnEditorClose()
{
    IEditor *pEditor = qobject_cast<IEditor*>(sender());
    ASSERT(pEditor);

    if (qobject_cast<CWorldEditor*>(pEditor) == nullptr)
    {
        mEditorWindows.removeOne(pEditor);
        delete pEditor;
    }
}
