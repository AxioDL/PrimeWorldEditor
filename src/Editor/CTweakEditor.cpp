#include "CTweakEditor.h"
#include "ui_CTweakEditor.h"
#include "Editor/Undo/IUndoCommand.h"

/** Internal undo command for changing tabs */
class CSetTweakIndexCommand : public IUndoCommand
{
    CTweakEditor* mpEditor;
    int mOldIndex, mNewIndex;

public:
    CSetTweakIndexCommand(CTweakEditor* pEditor, int OldIndex, int NewIndex)
        : IUndoCommand("Change Tab")
        , mpEditor(pEditor)
        , mOldIndex(OldIndex)
        , mNewIndex(NewIndex)
    {}

    void undo() override { mpEditor->SetActiveTweakIndex(mOldIndex); }
    void redo() override { mpEditor->SetActiveTweakIndex(mNewIndex); }
    bool AffectsCleanState() const override { return false; }
};

/** CTweakEditor functions */
CTweakEditor::CTweakEditor(QWidget* pParent)
    : IEditor(pParent)
    , mpUI(std::make_unique<Ui::CTweakEditor>())
{
    mpUI->setupUi(this);
    mpUI->TweakTabs->setExpanding(false);
    mpUI->ToolBar->addSeparator();
    AddUndoActions(mpUI->ToolBar);
    SET_WINDOWTITLE_APPVARS(tr("%APP_FULL_NAME% - Tweak Editor[*]"));

    connect(mpUI->TweakTabs, &QTabBar::currentChanged, this, &CTweakEditor::OnTweakTabClicked);
    connect(mpUI->ActionSave, &QAction::triggered, this, &CTweakEditor::Save);
    connect(mpUI->ActionSaveAndRepack, &QAction::triggered, this, &CTweakEditor::SaveAndRepack);
}

CTweakEditor::~CTweakEditor() = default;

bool CTweakEditor::HasTweaks() const
{
    return !mTweakAssets.isEmpty();
}

bool CTweakEditor::Save()
{
    if (!gpEdApp->ActiveProject()->TweakManager()->SaveTweaks())
    {
        UICommon::ErrorMsg(this, tr("Tweaks failed to save!"));
        return false;
    }
    else
    {
        UndoStack().setClean();
        setWindowModified(false);
        return true;
    }
}

void CTweakEditor::SetActiveTweakData(CTweakData* pTweakData)
{
    for (int TweakIdx = 0; TweakIdx < mTweakAssets.size(); TweakIdx++)
    {
        if (mTweakAssets[TweakIdx] != pTweakData)
            continue;

        auto* pCommand = new CSetTweakIndexCommand(this, mCurrentTweakIndex, TweakIdx);
        UndoStack().push(pCommand);
        break;
    }
}

void CTweakEditor::SetActiveTweakIndex(int Index)
{
    if (mCurrentTweakIndex == Index)
        return;

    mCurrentTweakIndex = Index;

    CTweakData* pTweakData = mTweakAssets[Index];
    mpUI->PropertyView->SetIntrinsicProperties(pTweakData->TweakData());

    mpUI->TweakTabs->blockSignals(true);
    mpUI->TweakTabs->setCurrentIndex(Index);
    mpUI->TweakTabs->blockSignals(false);
}

void CTweakEditor::OnTweakTabClicked(int Index)
{
    if (Index == mCurrentTweakIndex)
        return;

    auto* pCommand = new CSetTweakIndexCommand(this, mCurrentTweakIndex, Index);
    UndoStack().push(pCommand);
}

void CTweakEditor::OnProjectChanged(CGameProject* pNewProject)
{
    // Close and clear tabs
    mCurrentTweakIndex = -1;
    mpUI->PropertyView->ClearProperties();
    close();

    [[maybe_unused]] const QSignalBlocker tweakBlocker{mpUI->TweakTabs};

    while (mpUI->TweakTabs->count() > 0)
    {
        mpUI->TweakTabs->removeTab(0);
    }

    mTweakAssets.clear();
    UndoStack().clear();

    // Create tweak list
    if (pNewProject != nullptr)
    {
        const auto& tweakObjects = pNewProject->TweakManager()->TweakObjects();
        mTweakAssets.reserve(static_cast<int>(tweakObjects.size()));

        for (CTweakData* tweakData : tweakObjects)
        {
            mTweakAssets.push_back(tweakData);
        }
    }

    // Sort in alphabetical order and create tabs
    if (!mTweakAssets.isEmpty())
    {
        std::sort(mTweakAssets.begin(), mTweakAssets.end(), [](CTweakData* pLeft, CTweakData* pRight) -> bool {
            return pLeft->TweakName().ToUpper() < pRight->TweakName().ToUpper();
        });

        for (CTweakData* pTweakData : mTweakAssets)
        {
            const QString TweakName = TO_QSTRING(pTweakData->TweakName());
            mpUI->TweakTabs->addTab(TweakName);
        }

        SetActiveTweakIndex(0);
    }

    // Hide "save and repack" button for MP2+ as it doesn't do anything different from the regular Save button
    mpUI->ActionSaveAndRepack->setVisible(!pNewProject || pNewProject->Game() <= EGame::Prime);
}
