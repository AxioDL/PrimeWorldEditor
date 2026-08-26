#include "Editor/IEditor.h"

#include "Editor/CEditorApplication.h"
#include "Editor/Undo/IUndoCommand.h"

#include <Common/Log.h>

#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>

IEditor::IEditor(QWidget* pParent)
    : QMainWindow(pParent)
{
    // Register the editor window
    gpEdApp->AddEditor(this);

    // Create undo actions
    QAction *pUndoAction = mUndoStack.createUndoAction(this);
    QAction *pRedoAction = mUndoStack.createRedoAction(this);
    pUndoAction->setShortcut(QKeySequence::Undo);
    pRedoAction->setShortcut(QKeySequence::Redo);
    pUndoAction->setIcon(QIcon(QStringLiteral(":/icons/Undo.svg")));
    pRedoAction->setIcon(QIcon(QStringLiteral(":/icons/Redo.svg")));
    mUndoActions.push_back(pUndoAction);
    mUndoActions.push_back(pRedoAction);

    connect(&mUndoStack, &QUndoStack::indexChanged, this, &IEditor::OnUndoStackIndexChanged);
}

IEditor::~IEditor() = default;

QUndoStack& IEditor::UndoStack()
{
    return mUndoStack;
}

void IEditor::AddUndoActions(QToolBar* pToolBar, QAction* pBefore /*= 0*/)
{
    pToolBar->insertActions(pBefore, mUndoActions);
}

void IEditor::AddUndoActions(QMenu* pMenu, QAction* pBefore /*= 0*/)
{
    pMenu->insertActions(pBefore, mUndoActions);
}

bool IEditor::CheckUnsavedChanges()
{
    // Check whether the user has unsaved changes, return whether it's okay to clear the scene
    bool OkToClear = !isWindowModified();

    if (!OkToClear)
    {
        const int Result = QMessageBox::warning(this, tr("Save"), tr("You have unsaved changes. Save?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);

        if (Result == QMessageBox::Yes)
        {
            OkToClear = Save();
        }
        else if (Result == QMessageBox::No)
        {
            mUndoStack.setIndex(0); // Revert all changes
            OkToClear = true;
        }
        else if (Result == QMessageBox::Cancel)
        {
            OkToClear = false;
        }
    }

    return OkToClear;
}

/** QMainWindow overrides */
void IEditor::closeEvent(QCloseEvent* pEvent)
{
    if (CheckUnsavedChanges())
    {
        mUndoStack.clear();
        pEvent->accept();
        emit Closed();
    }
    else
    {
        pEvent->ignore();
    }
}

bool IEditor::Save()
{
    // Default implementation for editor windows that do not support resaving assets.
    // This should not be called.
    NLog::Error("Base IEditor::Save() implementation called. Changes will not be saved.");
    return true;
}

/** Non-virtual slots */
bool IEditor::SaveAndRepack()
{
    if (Save())
    {
        gpEdApp->CookAllDirtyPackages();
        return true;
    }

    return false;
}


void IEditor::OnUndoStackIndexChanged()
{
    // Check the commands that have been executed on the undo stack and find out whether any of them affect the clean state.
    // This is to prevent commands like select/deselect from altering the clean state.
    int CurrentIndex = mUndoStack.index();
    int CleanIndex = mUndoStack.cleanIndex();

    if (CleanIndex == -1)
    {
        if (!isWindowModified())
            mUndoStack.setClean();

        return;
    }

    if (CurrentIndex == CleanIndex)
    {
        setWindowModified(false);
    }
    else
    {
        bool IsClean = true;
        int LowIndex = (CurrentIndex > CleanIndex ? CleanIndex : CurrentIndex);
        int HighIndex = (CurrentIndex > CleanIndex ? CurrentIndex - 1 : CleanIndex - 1);

        for (int i = LowIndex; i <= HighIndex; i++)
        {
            const QUndoCommand *pkQCmd = mUndoStack.command(i);

            if (const auto* pkCmd = dynamic_cast<const IUndoCommand*>(pkQCmd))
            {
                if (pkCmd->AffectsCleanState())
                    IsClean = false;
            }
            else if (pkQCmd->childCount() > 0)
            {
                for (int ChildIdx = 0; ChildIdx < pkQCmd->childCount(); ChildIdx++)
                {
                    const auto *childCmd = static_cast<const IUndoCommand*>(pkQCmd->child(ChildIdx));

                    if (childCmd->AffectsCleanState())
                    {
                        IsClean = false;
                        break;
                    }
                }
            }

            if (!IsClean)
                break;
        }

        setWindowModified(!IsClean);
    }
}
