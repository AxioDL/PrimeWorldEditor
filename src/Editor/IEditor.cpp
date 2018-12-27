#include "IEditor.h"

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
    pUndoAction->setIcon(QIcon(":/icons/Undo.png"));
    pRedoAction->setIcon(QIcon(":/icons/Redo.png"));
    mUndoActions.push_back(pUndoAction);
    mUndoActions.push_back(pRedoAction);
}

QUndoStack& IEditor::UndoStack()
{
    return mUndoStack;
}

void IEditor::AddUndoActions(QToolBar* pToolBar, QAction* pBefore)
{
    pToolBar->insertActions(pBefore, mUndoActions);
}

void IEditor::AddUndoActions(QMenu* pMenu, QAction* pBefore)
{
    pMenu->insertActions(pBefore, mUndoActions);
}

bool IEditor::CheckUnsavedChanges()
{
    // Check whether the user has unsaved changes, return whether it's okay to clear the scene
    bool OkToClear = !isWindowModified();

    if (!OkToClear)
    {
        int Result = QMessageBox::warning(this, "Save", "You have unsaved changes. Save?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);

        if (Result == QMessageBox::Yes)
            OkToClear = Save();

        else if (Result == QMessageBox::No)
            OkToClear = true;

        else if (Result == QMessageBox::Cancel)
            OkToClear = false;
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

/** Non-virtual slots */
bool IEditor::SaveAndRepack()
{
    if (Save())
    {
        gpEdApp->CookAllDirtyPackages();
        return true;
    }
    else return false;
}
