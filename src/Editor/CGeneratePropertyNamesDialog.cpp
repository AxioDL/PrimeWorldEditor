#include "CGeneratePropertyNamesDialog.h"
#include "ui_CGeneratePropertyNamesDialog.h"

#include "Editor/Widgets/CCheckableTreeWidgetItem.h"
#include "UICommon.h"
#include <Core/Resource/Script/NGameList.h>
#include <Core/Resource/Script/NPropertyMap.h>
#include <QtConcurrent/QtConcurrent>
#include <iterator>

CGeneratePropertyNamesDialog::CGeneratePropertyNamesDialog(QWidget* pParent)
    : QDialog(pParent)
    , mpUI( new Ui::CGeneratePropertyNamesDialog )
    , mFutureWatcher( this )
    , mRunningNameGeneration( false )
    , mCanceledNameGeneration( false )
{
    mpUI->setupUi(this);
    mNotifier.SetProgressBar( mpUI->ProgressBar );

    connect( mpUI->AddSuffixButton, SIGNAL(pressed()), this, SLOT(AddSuffix()) );
    connect( mpUI->RemoveSuffixButton, SIGNAL(pressed()), this, SLOT(DeleteSuffix()) );
    connect( mpUI->ClearIdPoolButton, SIGNAL(pressed()), this, SLOT(ClearIdPool()) );
    connect( mpUI->StartButton, SIGNAL(pressed()), this, SLOT(StartGeneration()) );
    connect( mpUI->CancelButton, SIGNAL(pressed()), this, SLOT(CancelGeneration()) );
    connect( mpUI->CheckAllButton, SIGNAL(pressed()), this, SLOT(CheckAll()) );
    connect( mpUI->UncheckAllButton, SIGNAL(pressed()), this, SLOT(UncheckAll()) );
    connect( mpUI->ApplyButton, SIGNAL(pressed()), this, SLOT(ApplyChanges()) );
    connect( mpUI->OutputTreeWidget, SIGNAL(CheckStateChanged(QTreeWidgetItem*)),
             this, SLOT(OnTreeItemChecked(QTreeWidgetItem*)) );
    connect( mpUI->OutputTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(OnTreeItemDoubleClicked(QTreeWidgetItem*)) );

    // Configure default tree view split sizes
    // I don't know why it needs to be multiplied by 1.5, it just does
    int TreeWidth = mpUI->OutputTreeWidget->width();
    mpUI->OutputTreeWidget->setColumnWidth(0, TreeWidth * 1.5);
    mpUI->OutputTreeWidget->setHeaderHidden(false);

    // Allow the generator to initialize in the background while the user is getting set up
    QtConcurrent::run(&mGenerator, &CPropertyNameGenerator::Warmup);
}

CGeneratePropertyNamesDialog::~CGeneratePropertyNamesDialog()
{
    delete mpUI;
}

/** Add a property to the ID pool */
void CGeneratePropertyNamesDialog::AddToIDPool(IProperty* pProperty)
{
    if (!pProperty->UsesNameMap())
    {
        Log::Error("Failed to add property " + pProperty->IDString(false) + " to the generator ID pool because it doesn't use the name map");
        return;
    }

    u32 ID = pProperty->ID();
    const char* pkTypeName = pProperty->HashableTypeName();
    mIdPairs << SPropertyIdTypePair { ID, pkTypeName };

    QString ItemText = QString("%1 [%2]").arg( *TString::HexString(pProperty->ID(), 8, false) ).arg( pkTypeName );
    mpUI->IdPoolList->addItem( ItemText );

    // We probably don't want to call UpdateUI every single time we add a property, but
    // we do need to call it somewhere to make sure the ID list shows up on the UI...
    if (mpUI->IdPoolGroupBox->isHidden())
    {
        UpdateUI();
    }
}

/** Populate the ID pool with the children of the given property */
void CGeneratePropertyNamesDialog::AddChildrenToIDPool(IProperty* pProperty, bool Recursive)
{
    for (u32 ChildIdx = 0; ChildIdx < pProperty->NumChildren(); ChildIdx++)
    {
        IProperty* pChild = pProperty->ChildByIndex(ChildIdx);

        // Skip children that already have valid property names
        if (!pChild->HasAccurateName() && pChild->UsesNameMap())
        {
            AddToIDPool(pChild);
        }

        if (Recursive)
        {
            AddChildrenToIDPool(pChild, true);
        }
    }
}

/** Show event override */
void CGeneratePropertyNamesDialog::showEvent(QShowEvent*)
{
    UpdateUI();
}

/** Close event override */
void CGeneratePropertyNamesDialog::closeEvent(QCloseEvent*)
{
    if (mRunningNameGeneration)
    {
        CancelGeneration();
    }
    ClearIdPool();
}

/** Add an item to the suffix list */
void CGeneratePropertyNamesDialog::AddSuffix()
{
    QListWidgetItem* pNewItem = new QListWidgetItem("New Suffix", mpUI->TypeSuffixesListWidget);
    pNewItem->setFlags( Qt::ItemIsEditable |
                        Qt::ItemIsEnabled |
                        Qt::ItemIsSelectable );
    mpUI->TypeSuffixesListWidget->setCurrentItem(pNewItem, QItemSelectionModel::ClearAndSelect);
    mpUI->TypeSuffixesListWidget->editItem(pNewItem);
}

/** Deletes an item from the suffix list */
void CGeneratePropertyNamesDialog::DeleteSuffix()
{
    if (mpUI->TypeSuffixesListWidget->selectedItems().size() > 0)
    {
        int Row = mpUI->TypeSuffixesListWidget->currentRow();
        delete mpUI->TypeSuffixesListWidget->takeItem(Row);
    }
}

/** Clear the ID pool */
void CGeneratePropertyNamesDialog::ClearIdPool()
{
    mIdPairs.clear();
    mpUI->IdPoolList->clear();
    UpdateUI();
}

/** Start name generation */
void CGeneratePropertyNamesDialog::StartGeneration()
{
    ASSERT(!mRunningNameGeneration);
    mRunningNameGeneration = true;
    mCanceledNameGeneration = false;
    mTaskOutput.clear();
    mCheckedItems.clear();
    mpUI->OutputTreeWidget->clear();

    // Load all templates so we can match as many properties as possible
    NGameList::LoadAllGameTemplates();

    // Configure the generator
    SPropertyNameGenerationParameters Params;

    for (int RowIdx = 0; RowIdx < mpUI->TypeSuffixesListWidget->count(); RowIdx++)
    {
        QString ItemText = mpUI->TypeSuffixesListWidget->item(RowIdx)->text();
        Params.TypeNames.push_back( TO_TSTRING(ItemText) );
    }

    Params.MaxWords = mpUI->NumWordsSpinBox->value();
    Params.Prefix = TO_TSTRING( mpUI->PrefixLineEdit->text() );
    Params.Suffix = TO_TSTRING( mpUI->SuffixLineEdit->text() );
    Params.Casing = mpUI->CasingComboBox->currentEnum();
    Params.ValidIdPairs = mIdPairs.toStdVector();
    Params.ExcludeAccuratelyNamedProperties = mpUI->UnnamedOnlyCheckBox->isChecked();
    Params.TestIntsAsChoices = mpUI->TestIntsAsChoicesCheckBox->isChecked();
    Params.PrintToLog = mpUI->LogOutputCheckBox->isChecked();

    // Run the task and configure ourselves so we can update correctly
    connect( &mFutureWatcher, SIGNAL(finished()), this, SLOT(GenerationComplete()) );
    mFuture = QtConcurrent::run(&mGenerator, &CPropertyNameGenerator::Generate, Params, &mNotifier);
    mFutureWatcher.setFuture(mFuture);

    mUpdateTimer.start(500);
    connect( &mUpdateTimer, SIGNAL(timeout()), this, SLOT(CheckForNewResults()) );

    UpdateUI();
}

/** Cancel name generation */
void CGeneratePropertyNamesDialog::CancelGeneration()
{
    mNotifier.SetCanceled(true);
    mCanceledNameGeneration = true;
    UpdateUI();
}

/** Called when name generation is complete */
void CGeneratePropertyNamesDialog::GenerationComplete()
{
    mRunningNameGeneration = false;
    mCanceledNameGeneration = false;
    mNotifier.SetCanceled(false);
    mUpdateTimer.stop();

    mTaskOutput = QList<SGeneratedPropertyName>::fromStdList(
                    mGenerator.GetOutput()
                );

    mpUI->ProgressBar->setValue( mpUI->ProgressBar->maximum() );

    disconnect( &mFutureWatcher, 0, this, 0 );
    disconnect( &mUpdateTimer, 0, this, 0 );
    UpdateUI();
}

/** Called when an item in the output tree has been checked or unchecked */
void CGeneratePropertyNamesDialog::OnTreeItemChecked(QTreeWidgetItem* pItem)
{
    if (pItem->checkState(0) == Qt::Checked)
        mCheckedItems.append(pItem);
    else
        mCheckedItems.removeOne(pItem);

    UpdateUI();
}

/** Called when an item in the output tree has been double clicked */
void CGeneratePropertyNamesDialog::OnTreeItemDoubleClicked(QTreeWidgetItem* pItem)
{
    // Check whether this is an XML path
    if (pItem->parent() != nullptr)
    {
        QString Text = pItem->text(0);

        if (Text.endsWith(".xml"))
        {
            TString TStrText = TO_TSTRING(Text);
            TString DirPath = "../templates/" + TStrText.GetFileDirectory();
            TString AbsPath = FileUtil::MakeAbsolute(DirPath) + TStrText.GetFileName();
            UICommon::OpenInExternalApplication( TO_QSTRING(AbsPath) );
        }
    }
}

/** Check all items in the output tree */
void CGeneratePropertyNamesDialog::CheckAll()
{
    mpUI->OutputTreeWidget->blockSignals(true);
    mCheckedItems.clear();
    mCheckedItems.reserve( mpUI->OutputTreeWidget->topLevelItemCount() );

    for (int RowIdx = 0; RowIdx < mpUI->OutputTreeWidget->topLevelItemCount(); RowIdx++)
    {
        QTreeWidgetItem* pItem = mpUI->OutputTreeWidget->topLevelItem(RowIdx);
        pItem->setCheckState( 0, Qt::Checked );
        mCheckedItems << pItem;
    }

    mpUI->OutputTreeWidget->blockSignals(false);
    UpdateUI();
}

/** Uncheck all items in the output tree */
void CGeneratePropertyNamesDialog::UncheckAll()
{
    mpUI->OutputGroupBox->blockSignals(true);

    for (int RowIdx = 0; RowIdx < mpUI->OutputTreeWidget->topLevelItemCount(); RowIdx++)
    {
        QTreeWidgetItem* pItem = mpUI->OutputTreeWidget->topLevelItem(RowIdx);
        pItem->setCheckState( 0, Qt::Unchecked );
    }

    mCheckedItems.clear();
    mpUI->OutputTreeWidget->blockSignals(false);
    UpdateUI();
}

/** Apply generated names on selected items */
void CGeneratePropertyNamesDialog::ApplyChanges()
{
    // make sure the user really wants to do this
    QString WarningText =
            QString("Are you sure you want to rename %1 %2? This operation cannot be undone.")
                .arg(mCheckedItems.size())
                .arg(mCheckedItems.size() == 1 ? "property" : "properties");

    bool ReallyRename = UICommon::YesNoQuestion(this, "Warning", WarningText);

    if (!ReallyRename)
    {
        return;
    }

    // Perform rename operation
    for (int ItemIdx = 0; ItemIdx < mCheckedItems.size(); ItemIdx++)
    {
        QTreeWidgetItem* pItem = mCheckedItems[ItemIdx];
        u32 ID = TO_TSTRING( pItem->text(2) ).ToInt32();
        TString Type = TO_TSTRING( pItem->text(1) );
        TString NewName = TO_TSTRING( pItem->text(0) );

        NPropertyMap::SetPropertyName( ID, *Type, *NewName );
        pItem->setText( 3, TO_QSTRING(NewName) );
    }

    NPropertyMap::SaveMap();
}

/** Check progress on name generation task and display results on the UI */
void CGeneratePropertyNamesDialog::CheckForNewResults()
{
    const std::list<SGeneratedPropertyName>& rkOutput = mGenerator.GetOutput();

    QTreeWidget* pTreeWidget = mpUI->OutputTreeWidget;
    int CurItemCount = pTreeWidget->topLevelItemCount();

    // Add new items to the tree
    if (rkOutput.size() > CurItemCount)
    {
        std::list<SGeneratedPropertyName>::const_iterator Iter = rkOutput.cbegin();
        std::list<SGeneratedPropertyName>::const_iterator End = rkOutput.cend();
        std::advance(Iter, CurItemCount);

        for (; Iter != End; Iter++)
        {
            const SGeneratedPropertyName& rkName = *Iter;

            // Add an item to the tree for this name
            QStringList ColumnText;
            ColumnText << TO_QSTRING( rkName.Name )
                       << TO_QSTRING( rkName.Type )
                       << TO_QSTRING( TString::HexString(rkName.ID) )
                       << TO_QSTRING( NPropertyMap::GetPropertyName(rkName.ID, *rkName.Type) );

            QTreeWidgetItem* pItem = new CCheckableTreeWidgetItem(pTreeWidget, ColumnText);
            pItem->setFlags(Qt::ItemIsEnabled |
                            Qt::ItemIsSelectable |
                            Qt::ItemIsUserCheckable);
            pItem->setCheckState(0, Qt::Unchecked);

            // Add children items
            for (auto Iter = rkName.XmlList.begin(); Iter != rkName.XmlList.end(); Iter++)
            {
                QString XmlName = TO_QSTRING( *Iter );
                ColumnText.clear();
                ColumnText << XmlName;

                QTreeWidgetItem* pChild = new QTreeWidgetItem(pItem, ColumnText);
                pChild->setFlags(Qt::ItemIsEnabled);
                pChild->setFirstColumnSpanned(true);
            }
        }
    }

    UpdateUI();
}

/** Updates the enabled status of various widgets */
void CGeneratePropertyNamesDialog::UpdateUI()
{
    mpUI->SettingsGroupBox->setEnabled( !mRunningNameGeneration );
    mpUI->TypeSuffixesGroupBox->setEnabled( !mRunningNameGeneration );
    mpUI->TypeSuffixesGroupBox->setHidden( !mIdPairs.isEmpty() );
    mpUI->IdPoolGroupBox->setEnabled( !mRunningNameGeneration );
    mpUI->IdPoolGroupBox->setHidden( mIdPairs.isEmpty() );
    mpUI->StartButton->setEnabled( !mRunningNameGeneration );
    mpUI->CancelButton->setEnabled( mRunningNameGeneration && !mCanceledNameGeneration );

    int TotalItems = mpUI->OutputTreeWidget->topLevelItemCount();
    bool HasResults = TotalItems > 0;
    bool HasCheckedResults = HasResults && !mCheckedItems.isEmpty();
    mpUI->CheckAllButton->setEnabled( HasResults );
    mpUI->UncheckAllButton->setEnabled( HasResults );
    mpUI->ApplyButton->setEnabled( !mRunningNameGeneration && HasCheckedResults );

    // Update label
    if (HasResults)
    {
        mpUI->NumSelectedLabel->setText(
                    QString("%1 names, %2 selected")
                        .arg(TotalItems)
                        .arg(mCheckedItems.size())
                );
    }
    else
        mpUI->NumSelectedLabel->clear();
}
