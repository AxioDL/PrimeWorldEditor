#include "Editor/CEditorApplication.h"

#include "Editor/CBasicViewport.h"
#include "Editor/CProgressDialog.h"
#include "Editor/CProjectSettingsDialog.h"
#include "Editor/CTweakEditor.h"
#include "Editor/IEditor.h"
#include "Editor/NDolphinIntegration.h"
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
#include <Core/GameProject/CPackage.h>
#include <Core/Resource/CWorld.h>
#include <Core/Resource/Animation/CAnimSet.h>
#include <Core/Resource/Collision/CCollisionMeshGroup.h>
#include <Core/Resource/Model/CModel.h>
#include <Core/Resource/Scan/CScan.h>
#include <Core/Resource/StringTable/CStringTable.h>
#include <Core/Tweaks/CTweakData.h>
#include <Core/Tweaks/CTweakManager.h>

#include <QFuture>
#include <QtConcurrentRun>

#include <algorithm>

CEditorApplication::CEditorApplication(int& rArgc, char **ppArgv)
    : QApplication(rArgc, ppArgv)
    , mLastUpdate{CTimer::GlobalTime()}
{
    connect(&mRefreshTimer, &QTimer::timeout, this, &CEditorApplication::TickEditors);
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
    for (IEditor *pEditor : mEditorWindows)
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
    [[maybe_unused]] auto pOldProj = std::move(mpActiveProject);
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

    CProgressDialog Dialog(tr("Opening %1").arg(TO_QSTRING(Path.GetFileName())), true, true, mpWorldEditor);
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
        UICommon::ErrorMsg(mpWorldEditor, tr("Failed to open project!"));
        return false;
    }
}

void CEditorApplication::EditResource(CResourceEntry* pEntry)
{
    ASSERT(pEntry != nullptr);

    // Check if we're already editing this resource
    if (mEditingMap.contains(pEntry))
    {
        IEditor* pEd = mEditingMap[pEntry];
        pEd->show();
        pEd->raise();
        return;
    }

    // Attempt to load asset
    CResource* pRes = pEntry->Load();
    if (!pRes)
    {
        UICommon::ErrorMsg(mpWorldEditor, tr("Failed to load resource!"));
        return;
    }

    // Create and show an editor if the particular resource can be handled
    if (auto* editor = CreateEditor(pEntry, pRes))
    {
        editor->show();

        if (pEntry->ResourceType() != EResourceType::Tweaks)
            mEditingMap[pEntry] = editor;
    }
    else if (pEntry->ResourceType() != EResourceType::Area)
    {
        UICommon::InfoMsg(mpWorldEditor, tr("Unsupported Resource"), tr("This resource type is currently unsupported for editing."));
    }
}


IEditor* CEditorApplication::CreateEditor(CResourceEntry* entry, CResource* res)
{
    switch (entry->ResourceType())
    {
    case EResourceType::Area:
        // We can't open an area on its own. Find a world that contains this area.
        for (const auto& worldEntry : entry->ResourceStore()->MakeTypedResourceView(EResourceType::World))
        {
            if (worldEntry->Dependencies()->HasDependency(worldEntry->ID()))
            {
                auto* world = static_cast<CWorld*>(worldEntry->Load());
                const auto areaIdx = world->AreaIndex(worldEntry->ID());

                if (areaIdx != UINT32_MAX)
                {
                    mpWorldEditor->SetArea(world, areaIdx);
                    break;
                }
            }
        }
        // Handled by the world editor, so technically it's already been created.
        return nullptr;

    case EResourceType::Model:
        return new CModelEditorWindow(static_cast<CModel*>(res), mpWorldEditor);

    case EResourceType::AnimSet:
        return new CCharacterEditor(static_cast<CAnimSet*>(res), mpWorldEditor);

    case EResourceType::Scan:
        return new CScanEditor(static_cast<CScan*>(res), mpWorldEditor);

    case EResourceType::StringTable:
        return new CStringEditor(static_cast<CStringTable*>(res), mpWorldEditor);

    case EResourceType::Tweaks:
    {
        auto* pTweakEditor = mpWorldEditor->TweakEditor();
        pTweakEditor->SetActiveTweakData(static_cast<CTweakData*>(res));
        return pTweakEditor;
    }

    case EResourceType::DynamicCollision:
        return new CCollisionEditor(static_cast<CCollisionMeshGroup*>(res), mpWorldEditor);

    default:
        // Unhandled resource.
        return nullptr;
    }
}

void CEditorApplication::NotifyAssetsModified()
{
    emit AssetsModified();
}

bool CEditorApplication::CookPackage(CPackage *pPkg)
{
    return CookPackageList({pPkg});
}

bool CEditorApplication::CookAllDirtyPackages()
{
    ASSERT(mpActiveProject != nullptr);
    QList<CPackage*> PackageList;

    for (const auto& pkg : mpActiveProject->Packages())
    {
        if (pkg->NeedsRecook())
            PackageList.push_back(pkg.get());
    }

    return CookPackageList(PackageList);
}

bool CEditorApplication::CookPackageList(const QList<CPackage*>& PackageList)
{
    if (PackageList.isEmpty())
        return true;

    const auto NumPackages = static_cast<int>(PackageList.size());
    CProgressDialog Dialog(tr("Cooking package(s)", "", NumPackages), false, true, mpWorldEditor);

    QFuture<void> Future = QtConcurrent::run([&]()
    {
        Dialog.SetNumTasks(NumPackages);

        for (int PkgIdx = 0; PkgIdx < NumPackages && !Dialog.ShouldCancel(); PkgIdx++)
        {
            CPackage *pPkg = PackageList[PkgIdx];
            Dialog.SetTask(PkgIdx, (tr("Cooking %1.pak...").arg(TO_QSTRING(pPkg->Name())).toStdString()));
            pPkg->Cook(&Dialog);
        }
    });

    Dialog.WaitForResults(Future);

    emit PackagesCooked();
    return !Dialog.ShouldCancel();
}

bool CEditorApplication::HasAnyDirtyPackages() const
{
    if (!mpActiveProject)
        return false;

    return std::ranges::any_of(mpActiveProject->Packages(), &CPackage::NeedsRecook);
}

bool CEditorApplication::RebuildResourceDatabase()
{
    // Make sure all editors are closed
    if (mpActiveProject && CloseAllEditors())
    {
        // Fake-close the project, but keep it in memory so we can modify the resource store
        auto pProj = std::move(mpActiveProject);
        pProj->TweakManager()->ClearTweaks();
        emit ActiveProjectChanged(nullptr);

        // Rebuild
        CProgressDialog Dialog(tr("Rebuilding resource database"), true, false, mpWorldEditor);
        Dialog.SetOneShotTask("Rebuilding resource database");
        Dialog.DisallowCanceling();

        QFuture<void> Future = QtConcurrent::run(&CResourceStore::RebuildFromDirectory, pProj->ResourceStore());
        Dialog.WaitForResults(Future);
        Dialog.close();

        // Set project to active again
        mpActiveProject = std::move(pProj);
        mpActiveProject->TweakManager()->LoadTweaks();
        emit ActiveProjectChanged(mpActiveProject.get());

        UICommon::InfoMsg(mpWorldEditor, tr("Success"), tr("Resource database rebuilt successfully!"));
        return true;
    }

    return false;
}

// ************ SLOTS ************
void CEditorApplication::AddEditor(IEditor *pEditor)
{
    mEditorWindows.push_back(pEditor);
    connect(pEditor, &IEditor::Closed, this, &CEditorApplication::OnEditorClose);
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
    for (IEditor *pEditor : mEditorWindows)
    {
        if (pEditor->isVisible())
        {
            CBasicViewport *pViewport = pEditor->Viewport();
            const bool ViewportVisible = (pViewport && pViewport->isVisible() && !pEditor->isMinimized());

            if (ViewportVisible)
                pViewport->ProcessInput();

            pEditor->EditorTick(static_cast<float>(DeltaTime));

            if (ViewportVisible)
                pViewport->Render();
        }
    }
}

void CEditorApplication::OnEditorClose()
{
    auto* pEditor = qobject_cast<IEditor*>(sender());
    ASSERT(pEditor);

    if (pEditor == mpWorldEditor)
    {
        mpWorldEditor->deleteLater();
        mpWorldEditor = nullptr;
        quit();
    }
    else
    {
        const auto iter = std::ranges::find(mEditingMap, pEditor);
        if (iter != mEditingMap.end())
            mEditingMap.erase(iter);

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
