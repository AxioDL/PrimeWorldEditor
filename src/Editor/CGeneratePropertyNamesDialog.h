#ifndef CGENERATEPROPERTYNAMESDIALOG_H
#define CGENERATEPROPERTYNAMESDIALOG_H

#include "CProgressBarNotifier.h"
#include "Editor/Widgets/TEnumComboBox.h"
#include <Core/Resource/Script/Property/CPropertyNameGenerator.h>
#include <Core/Resource/Script/Property/IProperty.h>
#include <Core/Resource/Script/Property/CEnumProperty.h>

#include <QDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QScopedPointer>
#include <QTimer>
#include <QTreeWidgetItem>
#include <memory>

using CNameCasingComboBox = TEnumComboBox<ENameCasing>;

namespace Ui {
class CGeneratePropertyNamesDialog;
}

/**
 * Dialog box for accessing property name generation functionality.
 */
class CGeneratePropertyNamesDialog : public QDialog
{
    Q_OBJECT
    std::unique_ptr<Ui::CGeneratePropertyNamesDialog> mpUI;

    /** The name generator */
    CPropertyNameGenerator mGenerator;

    /** Progress notifier for updating the progress bar */
    CProgressBarNotifier mNotifier;

    /** List of ID/type pairs in the ID pool */
    QVector<SPropertyIdTypePair> mIdPairs;

    /** Future/future watcher for name generation task */
    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    /** Timer for fetching updates from name generation task */
    QTimer mUpdateTimer;

    /** Checked items in the output tree widget */
    QVector<QTreeWidgetItem*> mCheckedItems;

    /** Whether name generation is running */
    bool mRunningNameGeneration = false;

    /** Whether name generation has been canceled */
    bool mCanceledNameGeneration = false;

public:
    explicit CGeneratePropertyNamesDialog(QWidget *pParent = nullptr);
    ~CGeneratePropertyNamesDialog() override;

    /** Add a property to the ID pool */
    void AddToIDPool(IProperty* pProperty);

    /** Populate the ID pool with the children of the given property */
    void AddChildrenToIDPool(IProperty* pProperty, bool Recursive);

    /** Populate the ID pool with enum values */
    void AddEnumValuesToIDPool(CEnumProperty* pEnum);

public slots:
    /** Show event override */
    void showEvent(QShowEvent* pEvent) override;

    /** Close event override */
    void closeEvent(QCloseEvent* pEvent) override;

    /** Add an item to the suffix list */
    void AddSuffix();

    /** Deletes an item from the suffix list */
    void DeleteSuffix();

    /** Clear the ID pool */
    void ClearIdPool();

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
