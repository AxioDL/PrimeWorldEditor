#ifndef CGENERATEPROPERTYNAMESDIALOG_H
#define CGENERATEPROPERTYNAMESDIALOG_H

#include "CProgressBarNotifier.h"
#include <Core/Resource/Script/Property/CPropertyNameGenerator.h>

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QScopedPointer>
#include <QTimer>
#include <QTreeWidgetItem>

namespace Ui {
class CGeneratePropertyNamesDialog;
}

/**
 * Dialog box for accessing property name generation functionality.
 */
class CGeneratePropertyNamesDialog : public QDialog
{
    Q_OBJECT
    Ui::CGeneratePropertyNamesDialog* mpUI;

    /** The name generator */
    CPropertyNameGenerator mGenerator;

    /** Progress notifier for updating the progress bar */
    CProgressBarNotifier mNotifier;

    /** Future/future watcher for name generation task */
    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    /** Timer for fetching updates from name generation task */
    QTimer mUpdateTimer;

    /** Copy of the output buffer from the name generator; only set after completion */
    QList<SGeneratedPropertyName> mTaskOutput;

    /** Checked items in the output tree widget */
    QVector<QTreeWidgetItem*> mCheckedItems;

    /** Whether name generation is running */
    bool mRunningNameGeneration;

    /** Whether name generation has been canceled */
    bool mCanceledNameGeneration;

public:
    explicit CGeneratePropertyNamesDialog(QWidget *pParent = 0);
    ~CGeneratePropertyNamesDialog();

public slots:
    /** Close event override */
    virtual void closeEvent(QCloseEvent* pEvent);

    /** Add an item to the suffix list */
    void AddSuffix();

    /** Deletes an item from the suffix list */
    void DeleteSuffix();

    /** Start name generation */
    void StartGeneration();

    /** Cancel name generation */
    void CancelGeneration();

    /** Called when name generation is complete */
    void GenerationComplete();

    /** Called when an item in the output tree has been checked or unchecked */
    void OnTreeItemChecked(QTreeWidgetItem* pItem);

    /** Called when an item in the output tree has been double clicked */
    void OnTreeItemDoubleClicked(QTreeWidgetItem* pItem);

    /** Check all items in the output tree */
    void CheckAll();

    /** Uncheck all items in the output tree */
    void UncheckAll();

    /** Apply generated names on selected items */
    void ApplyChanges();

    /** Check progress on name generation task and display results on the UI */
    void CheckForNewResults();

    /** Updates the enabled status of various widgets */
    void UpdateUI();
};

#endif // CGENERATEPROPERTYNAMESDIALOG_H
