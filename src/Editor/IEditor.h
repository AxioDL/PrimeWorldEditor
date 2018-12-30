#ifndef IEDITOR
#define IEDITOR

#include <QMainWindow>
#include <QAction>
#include <QList>
#include <QUndoStack>

#include "CEditorApplication.h"

/** Base class of all editor windows */
class IEditor : public QMainWindow
{
    Q_OBJECT

protected:
    // Undo stack
    QUndoStack mUndoStack;
    QList<QAction*> mUndoActions;

public:
    IEditor(QWidget* pParent);
    QUndoStack& UndoStack();
    void AddUndoActions(QToolBar* pToolBar, QAction* pBefore = 0);
    void AddUndoActions(QMenu* pMenu, QAction* pBefore = 0);
    bool CheckUnsavedChanges();

    /** QMainWindow overrides */
    virtual void closeEvent(QCloseEvent*);

    /** Interface */
    virtual void EditorTick(float /*DeltaTime*/)    { }
    virtual CBasicViewport* Viewport() const        { return nullptr; }

public slots:
    /** Virtual slots */
    virtual bool Save()
    {
        // Default implementation for editor windows that do not support resaving assets.
        // This should not be called.
        warnf("Base IEditor::Save() implementation called. Changes will not be saved.");
        return true;
    }

    /** Non-virtual slots */
    bool SaveAndRepack();
    void OnUndoStackIndexChanged();

signals:
    void Closed();
};

#endif // IEDITOR

