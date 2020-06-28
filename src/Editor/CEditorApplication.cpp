#include "CEditorApplication.h"
#include "IEditor.h"
#include "CBasicViewport.h"
#include "CProgressDialog.h"
#include "CProjectSettingsDialog.h"
#include "NDolphinIntegration.h"

#include "Editor/CharacterEditor/CCharacterEditor.h"
#include "Editor/CollisionEditor/CCollisionEditor.h"
#include "Editor/ModelEditor/CModelEditorWindow.h"
#include "Editor/ScanEditor/CScanEditor.h"
#include "Editor/StringEditor/CStringEditor.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include "Editor/WorldEditor/CWorldEditor.h"

#include <Common/Macros.h>
#include <Common/CTimer.h>
#include <Core/GameProject/CGameProject.h>

#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

CEditorApplication::CEditorApplication(int& rArgc, char **ppArgv)
    : QApplication(rArgc, ppArgv)
    , mLastUpdate{CTimer::GlobalTime()}
{
    connect(&mRefreshTimer, SIGNAL(timeout()), this, SLOT(TickEditors()));
    mRefreshTimer.start(8);
}

CEditorApplication::~CEditorApplication()
{
    NDolphinIntegration::KillQuickplay();
    delete mpWorldEditor;
}

void CEditorApplication::InitEditor()
{
    mpResourceBrowser = new CResourceBrowser();
    mpWorldEditor = new CWorldEditor();
    mpProjectDialog = new CProjectSettingsDialog(mpWorldEditor);
    mpWorldEditor->showMaximized();
    mInitialized = true;
}

bool CEditorApplication::CloseAllEditors()
{
    if (!mInitialized)
        return true;

    // Close active editor windows.
    foreach (IEditor *pEditor, mEditorWindows)
    {
        if (pEditor != mpWorldEditor && !pEditor->close())
            return false;
    }

    // Close world
    if (!mpWorldEditor->CloseWorld())
        return false;

    mpProjectDialog->close();
    return true;
}

bool CEditorApplication::CloseProject()
{
    if (mpActiveProject && !CloseAllEditors())
        return false;

    // Close any active quickplay sessions
    NDolphinIntegration::KillQuickplay();

    // Emit before actually deleting the project to allow editor references to clean up
    auto pOldProj = std::move(mpActiveProject);
    emit ActiveProjectChanged(nullptr);
    return true;
}

bool CEditorApplication::OpenProject(const QString& rkProjPath)
{
    // Close existing project
    if (!CloseProject())
        return false;

    // Load new project
    TString Path = TO_TSTRING(rkProjPath);

    CProgressDialog Dialog("Opening " + TO_QSTRING(Path.GetFileName()), true, true, mpWorldEditor);
    Dialog.DisallowCanceling();
    // Gross, but necessary until QtConcurrent supports move only types.
    QFuture<CGameProject*> Future = QtConcurrent::run([](const auto& path, auto* dialog) { return CGameProject::LoadProject(path, dialog).release(); }, Path, &Dialog);
    mpActiveProject = std::unique_ptr<CGameProject>(Dialog.WaitForResults(Future));
    Dialog.close();

    if (mpActiveProject)
    {
        gpResourceStore = mpActiveProject->ResourceStore();
        emit ActiveProjectChanged(mpActiveProject.get());
        return true;
    }
    else
    {
        UICommon::ErrorMsg(mpWorldEditor, "Failed to open project!");
        return false;
    }
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
            UICommon::ErrorMsg(mpWorldEditor, "Failed to load resource!");
            return;
        }

        // Launch editor window
        IEditor *pEd = nullptr;

        switch (pEntry->ResourceType())
        {
        case EResourceType::Area:
            // We can't open an area on its own. Find a world that contains this area.
            for (TResourceIterator<EResourceType::World> It; It; ++It)
            {
                if (It->Dependencies()->HasDependency(pEntry->ID()))
                {
                    CWorld *pWorld = (CWorld*) It->Load();
                    uint32 AreaIdx = pWorld->AreaIndex(pEntry->ID());

                    if (AreaIdx != -1)
                    {
                        mpWorldEditor->SetArea(pWorld, AreaIdx);
                        break;
                    }
                }
            }
            break;

        case EResourceType::Model:
            pEd = new CModelEditorWindow((CModel*) pRes, mpWorldEditor);
            break;

        case EResourceType::AnimSet:
            pEd = new CCharacterEditor((CAnimSet*) pRes, mpWorldEditor);
            break;

        case EResourceType::Scan:
            pEd = new CScanEditor((CScan*) pRes, mpWorldEditor);
            break;

        case EResourceType::StringTable:
            pEd = new CStringEditor((CStringTable*) pRes, mpWorldEditor);
            break;

        case EResourceType::Tweaks:
        {
            CTweakEditor* pTweakEditor = mpWorldEditor->TweakEditor();
            pTweakEditor->SetActiveTweakData( (CTweakData*) pRes );
            pEd = pTweakEditor;
            break;
        }

        case EResourceType::DynamicCollision:
        {
            pEd = new CCollisionEditor((CCollisionMeshGroup*) pRes, mpWorldEditor);
            break;
        }
        default: break;
        }

        if (pEd)
        {
            pEd->show();

            if (pEntry->ResourceType() != EResourceType::Tweaks)
                mEditingMap[pEntry] = pEd;
        }
        else if (pEntry->ResourceType() != EResourceType::Area)
            UICommon::InfoMsg(mpWorldEditor, "Unsupported Resource", "This resource type is currently unsupported for editing.");
    }
}

void CEditorApplication::NotifyAssetsModified()
{
    emit AssetsModified();
}

bool CEditorApplication::CookPackage(CPackage *pPkg)
{
    return CookPackageList(QList<CPackage*>() << pPkg);
}

bool CEditorApplication::CookAllDirtyPackages()
{
    ASSERT(mpActiveProject != nullptr);
    QList<CPackage*> PackageList;

    for (size_t iPkg = 0; iPkg < mpActiveProject->NumPackages(); iPkg++)
    {
        CPackage *pPackage = mpActiveProject->PackageByIndex(iPkg);

        if (pPackage->NeedsRecook())
            PackageList << pPackage;
    }

    return CookPackageList(PackageList);
}

bool CEditorApplication::CookPackageList(QList<CPackage*> PackageList)
{
    if (!PackageList.isEmpty())
    {
        CProgressDialog Dialog("Cooking package" + QString(PackageList.size() > 1  ? "s" : ""), false, true, mpWorldEditor);

        QFuture<void> Future = QtConcurrent::run([&]()
        {
            Dialog.SetNumTasks(PackageList.size());

            for (int PkgIdx = 0; PkgIdx < PackageList.size() && !Dialog.ShouldCancel(); PkgIdx++)
            {
                CPackage *pPkg = PackageList[PkgIdx];
                Dialog.SetTask(PkgIdx, "Cooking " + pPkg->Name() + ".pak...");
                pPkg->Cook(&Dialog);
            }
        });

        Dialog.WaitForResults(Future);

        emit PackagesCooked();
        return !Dialog.ShouldCancel();
    }
    else return true;
}

bool CEditorApplication::HasAnyDirtyPackages()
{
    if (!mpActiveProject)
        return false;

    for (size_t PkgIdx = 0; PkgIdx < mpActiveProject->NumPackages(); PkgIdx++)
    {
        CPackage *pPackage = mpActiveProject->PackageByIndex(PkgIdx);

        if (pPackage->NeedsRecook())
            return true;
    }

    return false;
}

bool CEditorApplication::RebuildResourceDatabase()
{
    // Make sure all editors are closed
    if (mpActiveProject && CloseAllEditors())
    {
        // Fake-close the project, but keep it in memory so we can modify the resource store
        auto pProj = std::move(mpActiveProject);
        mpActiveProject->TweakManager()->ClearTweaks();
        emit ActiveProjectChanged(nullptr);

        // Rebuild
        CProgressDialog Dialog("Rebuilding resource database", true, false, mpWorldEditor);
        Dialog.SetOneShotTask("Rebuilding resource database");
        Dialog.DisallowCanceling();

        QFuture<void> Future = QtConcurrent::run(pProj->ResourceStore(), &CResourceStore::RebuildFromDirectory);
        Dialog.WaitForResults(Future);
        Dialog.close();

        // Set project to active again
        mpActiveProject = std::move(pProj);
        mpActiveProject->TweakManager()->LoadTweaks();
        emit ActiveProjectChanged(mpActiveProject.get());

        UICommon::InfoMsg(mpWorldEditor, "Success", "Resource database rebuilt successfully!");
        return true;
    }

    return false;
}

// ************ SLOTS ************
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

    // Make sure the resource store caches are up-to-date
    if (gpEditorStore)
        gpEditorStore->ConditionalSaveStore();

    if (gpResourceStore)
        gpResourceStore->ConditionalSaveStore();

    // Tick each editor window and redraw their viewports
    foreach(IEditor *pEditor, mEditorWindows)
    {
        if (pEditor->isVisible())
        {
            CBasicViewport *pViewport = pEditor->Viewport();
            bool ViewportVisible = (pViewport && pViewport->isVisible() && !pEditor->isMinimized());

            if (ViewportVisible) pViewport->ProcessInput();
            pEditor->EditorTick((float) DeltaTime);
            if (ViewportVisible) pViewport->Render();
        }
    }
}

void CEditorApplication::OnEditorClose()
{
    IEditor *pEditor = qobject_cast<IEditor*>(sender());
    ASSERT(pEditor);

    if (pEditor == mpWorldEditor)
    {
        mpWorldEditor->deleteLater();
        mpWorldEditor = nullptr;
        quit();
    }
    else
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

        if (pEditor != mpWorldEditor->TweakEditor())
        {
            pEditor->deleteLater();
        }

        if (mpActiveProject)
        {
            mpActiveProject->ResourceStore()->DestroyUnreferencedResources();
        }
    }
}
