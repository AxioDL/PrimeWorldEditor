#include "CEditorApplication.h"
#include "IEditor.h"
#include "CBasicViewport.h"
#include "CProjectOverviewDialog.h"
#include "Editor/CharacterEditor/CCharacterEditor.h"
#include "Editor/ModelEditor/CModelEditorWindow.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/WorldEditor/CWorldEditor.h"
#include <Common/AssertMacro.h>
#include <Common/CTimer.h>
#include <Core/GameProject/CGameProject.h>
#include <QMessageBox>

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
    delete mpProjectDialog;
}

void CEditorApplication::InitEditor()
{
    mpWorldEditor = new CWorldEditor();
    mpResourceBrowser = new CResourceBrowser(mpWorldEditor);
    mpProjectDialog = new CProjectOverviewDialog();
    connect(mpProjectDialog, SIGNAL(ActiveProjectChanged(CGameProject*)), mpResourceBrowser, SLOT(UpdateStore()));
}

void CEditorApplication::EditResource(CResourceEntry *pEntry)
{
    ASSERT(pEntry != nullptr);

    // Check if we're already editing this resource
    if (mEditingMap.contains(pEntry))
    {
        IEditor *pEd = mEditingMap[pEntry];
        pEd->show();
        pEd->raise();
    }

    else
    {
        // Attempt to load asset
        CResource *pRes = pEntry->Load();

        if (!pRes)
        {
            QMessageBox::warning(nullptr, "Error", "Failed to load resource!");
            return;
        }

        // Launch editor window
        IEditor *pEd = nullptr;

        switch (pEntry->ResourceType())
        {
        case eModel:
            pEd = new CModelEditorWindow((CModel*) pRes, mpWorldEditor);
            break;

        case eAnimSet:
            pEd = new CCharacterEditor((CAnimSet*) pRes, mpWorldEditor);
            break;
        }

        if (pEd)
        {
            pEd->show();
            mEditingMap[pEntry] = pEd;
        }
        else
            QMessageBox::information(0, "Unsupported Resource", "This resource type is currently unsupported for editing.");
    }
}

void CEditorApplication::NotifyAssetsModified()
{
    emit AssetsModified();
}

void CEditorApplication::CookAllDirtyPackages()
{
    CGameProject *pProj = CGameProject::ActiveProject();

    for (u32 iPkg = 0; iPkg < pProj->NumPackages(); iPkg++)
    {
        CPackage *pPackage = pProj->PackageByIndex(iPkg);

        if (pPackage->NeedsRecook())
            pPackage->Cook();
    }

    mpProjectDialog->SetupPackagesList();
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
        for (auto Iter = mEditingMap.begin(); Iter != mEditingMap.end(); Iter++)
        {
            if (Iter.value() == pEditor)
            {
                mEditingMap.erase(Iter);
                break;
            }
        }

        mEditorWindows.removeOne(pEditor);
        delete pEditor;
    }
}
