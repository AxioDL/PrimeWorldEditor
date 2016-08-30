#include "CEditorApplication.h"
#include "CEditorUpdateEvent.h"
#include "IEditor.h"
#include "CBasicViewport.h"
#include <Common/CTimer.h>

CEditorApplication::CEditorApplication(int& rArgc, char **ppArgv)
    : QApplication(rArgc, ppArgv)
{
    mLastUpdate = CTimer::GlobalTime();

    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(TickEditors()));
    mRefreshTimer.start(8);
}

void CEditorApplication::TickEditors()
{
    double LastUpdate = mLastUpdate;
    mLastUpdate = CTimer::GlobalTime();
    double DeltaTime = mLastUpdate - LastUpdate;

    foreach(IEditor *pEditor, mEditorWindows)
    {
        if (pEditor->isVisible())
        {
            pEditor->EditorTick((float) DeltaTime);

            CBasicViewport *pViewport = pEditor->Viewport();

            if (pViewport && pViewport->isVisible())
            {
                pViewport->ProcessInput();
                pViewport->Render();
            }
        }
    }
}
