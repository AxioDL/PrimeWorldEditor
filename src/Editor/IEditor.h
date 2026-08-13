#ifndef IEDITOR_H
#define IEDITOR_H

#include <QMainWindow>
#include <QAction>
#include <QList>
#include <QUndoStack>

class CBasicViewport;

/** Base class of all editor windows */
class IEditor : public QMainWindow
{
    Q_OBJECT

protected:
    // Undo stack
    QUndoStack mUndoStack;
    QList<QAction*> mUndoActions;

public:
    explicit IEditor(QWidget* pParent);
    ~IEditor() override;

    QUndoStack& UndoStack();
    void AddUndoActions(QToolBar* pToolBar, QAction* pBefore = nullptr);
    void AddUndoActions(QMenu* pMenu, QAction* pBefore = nullptr);
    bool CheckUnsavedChanges();

    /** QMainWindow overrides */
    void closeEvent(QCloseEvent*) override;

    /** Interface */
    virtual void EditorTick(float /*DeltaTime*/)    { }
    virtual CBasicViewport* Viewport() const        { return nullptr; }

public slots:
    /** Virtual slots */
    virtual bool Save();

    /** Non-virtual slots */
    bool SaveAndRepack();
    void OnUndoStackIndexChanged();

signals:
    void Closed();
};

#endif // IEDITOR_H

