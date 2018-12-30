#include "CTweakEditor.h"
#include "ui_CTweakEditor.h"
#include "Editor/Undo/IUndoCommand.h"

CTweakEditor::CTweakEditor(QWidget* pParent)
    : IEditor(pParent)
    , mpUI(new Ui::CTweakEditor)
    , mCurrentTweakIndex(-1)
    , mHasBeenShown(false)
{
    mpUI->setupUi(this);
    mpUI->TweakTabs->setExpanding(false);
    mpUI->ToolBar->addSeparator();
    AddUndoActions(mpUI->ToolBar);
    SET_WINDOWTITLE_APPVARS("%APP_FULL_NAME% - Tweak Editor[*]");

    connect(mpUI->TweakTabs, SIGNAL(currentChanged(int)), this, SLOT(OnTweakTabClicked(int)));
    connect(mpUI->ActionSave, SIGNAL(triggered(bool)), this, SLOT(Save()));
    connect(mpUI->ActionSaveAndRepack, SIGNAL(triggered(bool)), this, SLOT(SaveAndRepack()));
}

CTweakEditor::~CTweakEditor()
{
    delete mpUI;
}

bool CTweakEditor::HasTweaks()
{
    return !mTweakAssets.isEmpty();
}

bool CTweakEditor::Save()
{
    bool SavedAll = true;

    foreach (CTweakData* pData, mTweakAssets)
    {
        if (!pData->Entry()->Save())
        {
            SavedAll = false;
        }
    }

    if (!SavedAll)
    {
        UICommon::ErrorMsg(this, "Tweaks failed to save!");
        return false;
    }
    else
    {
        UndoStack().setClean();
        setWindowModified(false);
        return true;
    }
}

void CTweakEditor::showEvent(QShowEvent* pEvent)
{
    // Perform first-time UI initialization
    // Property view cannot initialize correctly until first show due to window width not being configured
    if (!mHasBeenShown)
    {
        mpUI->PropertyView->InitColumnWidths(0.6f, 0.3f);
        mHasBeenShown = true;
    }

    IEditor::showEvent(pEvent);
}

void CTweakEditor::SetActiveTweakData(CTweakData* pTweakData)
{
    for( int TweakIdx = 0; TweakIdx < mTweakAssets.size(); TweakIdx++ )
    {
        if (mTweakAssets[TweakIdx] == pTweakData)
        {
            SetActiveTweakIndex(TweakIdx);
            break;
        }
    }
}

void CTweakEditor::SetActiveTweakIndex(int Index)
{
    if( mCurrentTweakIndex != Index )
    {
        mCurrentTweakIndex = Index;

        CTweakData* pTweakData = mTweakAssets[Index];
        mpUI->PropertyView->SetIntrinsicProperties(pTweakData->TweakData());

        mpUI->TweakTabs->blockSignals(true);
        mpUI->TweakTabs->setCurrentIndex(Index);
        mpUI->TweakTabs->blockSignals(false);
    }
}

void CTweakEditor::OnTweakTabClicked(int Index)
{
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

        virtual void undo() override { mpEditor->SetActiveTweakIndex(mOldIndex); }
        virtual void redo() override { mpEditor->SetActiveTweakIndex(mNewIndex); }
        virtual bool AffectsCleanState() const { return false; }
    };

    if (Index != mCurrentTweakIndex)
    {
        CSetTweakIndexCommand* pCommand = new CSetTweakIndexCommand(this, mCurrentTweakIndex, Index);
        UndoStack().push(pCommand);
    }
}

void CTweakEditor::OnProjectChanged(CGameProject* pNewProject)
{
    // Close and clear tabs
    mCurrentTweakIndex = -1;
    mpUI->PropertyView->ClearProperties();
    close();

    mpUI->TweakTabs->blockSignals(true);

    while (mpUI->TweakTabs->count() > 0)
    {
        mpUI->TweakTabs->removeTab(0);
    }

    mTweakAssets.clear();
    UndoStack().clear();

    // Create tweak list
    if (pNewProject != nullptr)
    {
        for (TResPtr<CTweakData> pTweakData : pNewProject->TweakManager()->TweakObjects())
        {
            mTweakAssets << pTweakData.RawPointer();
        }
    }

    // Sort in alphabetical order and create tabs
    if (!mTweakAssets.isEmpty())
    {
        qSort(mTweakAssets.begin(), mTweakAssets.end(), [](CTweakData* pLeft, CTweakData* pRight) -> bool {
            return pLeft->Entry()->Name().ToUpper() < pRight->Entry()->Name().ToUpper();
        });

        foreach (CTweakData* pTweakData, mTweakAssets)
        {
            QString TweakName = TO_QSTRING( pTweakData->Entry()->Name() );
            mpUI->TweakTabs->addTab(TweakName);
        }

        SetActiveTweakIndex(0);
    }

    mpUI->TweakTabs->blockSignals(false);
}
