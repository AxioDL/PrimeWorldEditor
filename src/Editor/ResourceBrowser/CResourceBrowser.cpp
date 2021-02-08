#include "CResourceBrowser.h"
#include "ui_CResourceBrowser.h"
#include "CProgressDialog.h"
#include "CResourceDelegate.h"
#include "CResourceTableContextMenu.h"
#include "Editor/CEditorApplication.h"
#include "Editor/Undo/CMoveDirectoryCommand.h"
#include "Editor/Undo/CMoveResourceCommand.h"
#include "Editor/Undo/CRenameDirectoryCommand.h"
#include "Editor/Undo/CRenameResourceCommand.h"
#include "Editor/Undo/CSaveStoreCommand.h"
#include "Editor/Undo/ICreateDeleteDirectoryCommand.h"
#include "Editor/Undo/ICreateDeleteResourceCommand.h"
#include <Core/GameProject/AssetNameGeneration.h>
#include <Core/GameProject/CAssetNameMap.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>

CResourceBrowser::CResourceBrowser(QWidget *pParent)
    : QWidget(pParent)
    , mpUI(std::make_unique<Ui::CResourceBrowser>())
{
    mpUI->setupUi(this);
    setEnabled(false);

    // Hide sorting combo box for now. The size isn't displayed on the UI so this isn't really useful for the end user.
    mpUI->SortComboBox->hide();

    // Create undo/redo actions
    mpUndoAction = new QAction(tr("Undo"), this);
    mpRedoAction = new QAction(tr("Redo"), this);
    mpUndoAction->setShortcut(QKeySequence::Undo);
    mpRedoAction->setShortcut(QKeySequence::Redo);

    // todo - undo/redo commands are deactivated because they conflict with the World Editor undo/redo commands. fix this
#if 0
    addAction(mpUndoAction);
    addAction(mpRedoAction);
#endif

    connect(mpUndoAction, &QAction::triggered, this, &CResourceBrowser::Undo);
    connect(mpRedoAction, &QAction::triggered, this, &CResourceBrowser::Redo);
    connect(&mUndoStack, &QUndoStack::canUndoChanged, this, &CResourceBrowser::UpdateUndoActionStates);
    connect(&mUndoStack, &QUndoStack::canRedoChanged, this, &CResourceBrowser::UpdateUndoActionStates);

    // Configure display mode buttons
    QButtonGroup *pModeGroup = new QButtonGroup(this);
    pModeGroup->addButton(mpUI->ResourceTreeButton);
    pModeGroup->addButton(mpUI->ResourceListButton);
    pModeGroup->setExclusive(true);

    // Set up table models
    mpModel = new CResourceTableModel(this, this);
    mpProxyModel = new CResourceProxyModel(this);
    mpProxyModel->setSourceModel(mpModel);
    mpUI->ResourceTableView->setModel(mpProxyModel);
    mpUI->ResourceTableView->resizeRowsToContents();

    QHeaderView *pHeader = mpUI->ResourceTableView->horizontalHeader();
    pHeader->setSectionResizeMode(0, QHeaderView::Stretch);

    mpDelegate = new CResourceBrowserDelegate(this);
    mpUI->ResourceTableView->setItemDelegate(mpDelegate);
    mpUI->ResourceTableView->installEventFilter(this);

    // Set up directory tree model
    mpDirectoryModel = new CVirtualDirectoryModel(this, this);
    mpUI->DirectoryTreeView->setModel(mpDirectoryModel);

    RefreshResources();
    UpdateDescriptionLabel();

    // Set up filter checkboxes
    mpFilterBoxesLayout = new QVBoxLayout();
    mpFilterBoxesLayout->setContentsMargins(2, 2, 2, 2);
    mpFilterBoxesLayout->setSpacing(1);

    mpFilterBoxesContainerWidget = new QWidget(this);
    mpFilterBoxesContainerWidget->setLayout(mpFilterBoxesLayout);

    mpUI->FilterCheckboxScrollArea->setWidget(mpFilterBoxesContainerWidget);
    mpUI->FilterCheckboxScrollArea->setBackgroundRole(QPalette::AlternateBase);

    mFilterBoxFont = font();
    mFilterBoxFont.setPointSize(mFilterBoxFont.pointSize() - 1);

    QFont AllBoxFont = mFilterBoxFont;
    AllBoxFont.setBold(true);

    mpFilterAllBox = new QCheckBox(this);
    mpFilterAllBox->setChecked(true);
    mpFilterAllBox->setFont(AllBoxFont);
    mpFilterAllBox->setText("All");
    mpFilterBoxesLayout->addWidget(mpFilterAllBox);

    CreateFilterCheckboxes();

    // Set up the options menu
    QMenu *pOptionsMenu = new QMenu(this);
    QMenu *pImportMenu = pOptionsMenu->addMenu(tr("Import Names"));
    pOptionsMenu->addAction(tr("Export Names"), this, &CResourceBrowser::ExportAssetNames);
    pOptionsMenu->addSeparator();

    pImportMenu->addAction(tr("Asset Name Map"), this, &CResourceBrowser::ImportAssetNameMap);
    pImportMenu->addAction(tr("Package Contents List"), this, &CResourceBrowser::ImportPackageContentsList);
    pImportMenu->addAction(tr("Generate Asset Names"), this, &CResourceBrowser::GenerateAssetNames);

    QAction *pDisplayAssetIDsAction = new QAction(tr("Display Asset IDs"), this);
    pDisplayAssetIDsAction->setCheckable(true);
    connect(pDisplayAssetIDsAction, &QAction::toggled, this, &CResourceBrowser::SetAssetIDDisplayEnabled);
    pOptionsMenu->addAction(pDisplayAssetIDsAction);

    pOptionsMenu->addAction(tr("Find Asset by ID"), this, &CResourceBrowser::FindAssetByID);
    pOptionsMenu->addAction(tr("Rebuild Database"), this, &CResourceBrowser::RebuildResourceDB);
    mpUI->OptionsToolButton->setMenu(pOptionsMenu);

#if !PUBLIC_RELEASE
    // Only add the store menu in debug builds. We don't want end users editing the editor store.
    pOptionsMenu->addSeparator();
    QMenu *pStoreMenu = pOptionsMenu->addMenu(tr("Set Store"));
    QAction *pProjStoreAction = pStoreMenu->addAction(tr("Project Store"), this, &CResourceBrowser::SetProjectStore);
    QAction *pEdStoreAction = pStoreMenu->addAction(tr("Editor Store"), this, &CResourceBrowser::SetEditorStore);

    pProjStoreAction->setCheckable(true);
    pProjStoreAction->setChecked(true);
    pEdStoreAction->setCheckable(true);

    QActionGroup *pGroup = new QActionGroup(this);
    pGroup->addAction(pProjStoreAction);
    pGroup->addAction(pEdStoreAction);
#endif

    // Resize splitter
    mpUI->splitter->setSizes({
        static_cast<int>(width() * 0.4),
        static_cast<int>(width() * 0.6),
    });

    // Create context menu for the resource table
    new CResourceTableContextMenu(this, mpUI->ResourceTableView, mpModel, mpProxyModel);
    
    // Set up connections
    connect(mpUI->SearchBar, &CTimedLineEdit::StoppedTyping, this, &CResourceBrowser::OnSearchStringChanged);
    connect(mpUI->ResourceTreeButton, &QPushButton::pressed, this, &CResourceBrowser::SetResourceTreeView);
    connect(mpUI->ResourceListButton, &QPushButton::pressed, this, &CResourceBrowser::SetResourceListView);
    connect(mpUI->SortComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &CResourceBrowser::OnSortModeChanged);
    connect(mpUI->ClearButton, &QPushButton::pressed, this, &CResourceBrowser::OnClearButtonPressed);

    connect(mpUI->DirectoryTreeView, &CVirtualDirectoryTreeView::clicked, this, &CResourceBrowser::OnDirectorySelectionChanged);
    connect(mpUI->DirectoryTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CResourceBrowser::OnDirectorySelectionChanged);
    connect(mpUI->ResourceTableView, &CResourceTableView::doubleClicked, this, &CResourceBrowser::OnDoubleClickTable);
    connect(mpUI->ResourceTableView->selectionModel(), &QItemSelectionModel::currentChanged, this, &CResourceBrowser::OnResourceSelectionChanged);
    connect(mpProxyModel, &CResourceProxyModel::rowsInserted, mpUI->ResourceTableView, &CResourceTableView::resizeRowsToContents);
    connect(mpProxyModel, &CResourceProxyModel::layoutChanged, mpUI->ResourceTableView, &CResourceTableView::resizeRowsToContents);
    connect(mpProxyModel, &CResourceProxyModel::modelReset, mpUI->ResourceTableView, &CResourceTableView::resizeRowsToContents);
    connect(mpFilterAllBox, &QCheckBox::toggled, this, &CResourceBrowser::OnFilterTypeBoxTicked);
    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CResourceBrowser::UpdateStore);
}

CResourceBrowser::~CResourceBrowser() = default;

void CResourceBrowser::SetActiveDirectory(CVirtualDirectory *pDir)
{
    if (mpSelectedDir != pDir || mpModel->IsDisplayingUserEntryList())
    {
        mpSelectedDir = pDir;
        RefreshResources();
        UpdateDescriptionLabel();

        if (sender() != mpUI->DirectoryTreeView)
        {
            QModelIndex Index = mpDirectoryModel->GetIndexForDirectory(pDir);
            mpUI->DirectoryTreeView->selectionModel()->setCurrentIndex(Index, QItemSelectionModel::ClearAndSelect);
        }
    }
}

void CResourceBrowser::SelectResource(CResourceEntry *pEntry, bool ClearFiltersIfNecessary /*= false*/)
{
    ASSERT(pEntry);

    // Set directory active
    SetActiveDirectory(pEntry->Directory());

    // Clear search
    if (!mpUI->SearchBar->text().isEmpty())
    {
        mpUI->SearchBar->clear();
        UpdateFilter();
    }

    // Change filter
    if (ClearFiltersIfNecessary && !mpProxyModel->IsTypeAccepted(pEntry->TypeInfo()))
    {
        ResetTypeFilter();
    }

    // Select resource
    QModelIndex SourceIndex = mpModel->GetIndexForEntry(pEntry);
    QModelIndex ProxyIndex = mpProxyModel->mapFromSource(SourceIndex);

    if (ProxyIndex.isValid())
    {
        // Note: We have to call scrollToBottom() first or else sometimes scrollTo() doesn't work at all
        // in large folders (like Uncategorized). Dumb but the only solution I've been able to figure out.
        mpUI->ResourceTableView->selectionModel()->select(ProxyIndex, QItemSelectionModel::ClearAndSelect);
        mpUI->ResourceTableView->scrollToBottom();
        mpUI->ResourceTableView->scrollTo(ProxyIndex, QAbstractItemView::PositionAtCenter);
    }
}

void CResourceBrowser::SelectDirectory(CVirtualDirectory *pDir)
{
    ASSERT(pDir);
    ASSERT(!pDir->IsRoot());

    // Set parent directory active
    SetActiveDirectory(pDir->Parent());

    // Clear search
    if (!mpUI->SearchBar->text().isEmpty())
    {
        mpUI->SearchBar->clear();
        UpdateFilter();
    }

    // Select directory
    QModelIndex SourceIndex = mpModel->GetIndexForDirectory(pDir);
    QModelIndex ProxyIndex = mpProxyModel->mapFromSource(SourceIndex);
    mpUI->ResourceTableView->selectionModel()->select(ProxyIndex, QItemSelectionModel::ClearAndSelect);
    mpUI->ResourceTableView->scrollTo(ProxyIndex, QAbstractItemView::PositionAtCenter);
}

void CResourceBrowser::CreateFilterCheckboxes()
{
    // Delete existing checkboxes
    for (const SResourceType& rkType : mTypeList)
        delete rkType.pFilterCheckBox;

    mTypeList.clear();

    if (mpStore)
    {
        // Get new type list
        std::list<CResTypeInfo*> TypeList;
        CResTypeInfo::GetAllTypesInGame(mpStore->Game(), TypeList);

        for (auto* type : TypeList)
        {
            QCheckBox *pCheck = new QCheckBox(this);
            pCheck->setFont(mFilterBoxFont);
            pCheck->setText(TO_QSTRING(type->TypeName()));
            mTypeList.push_back(SResourceType{type, pCheck});
        }

        std::sort(mTypeList.begin(), mTypeList.end(), [](const SResourceType& rkLeft, const SResourceType& rkRight) {
            return rkLeft.pTypeInfo->TypeName().ToUpper() < rkRight.pTypeInfo->TypeName().ToUpper();
        });

        // Add sorted checkboxes to the UI
        for (const SResourceType& rkType : mTypeList)
        {
            QCheckBox *pCheck = rkType.pFilterCheckBox;
            mpFilterBoxesLayout->addWidget(rkType.pFilterCheckBox);
            connect(pCheck, &QCheckBox::toggled, this, &CResourceBrowser::OnFilterTypeBoxTicked);
        }
    }

    QSpacerItem *pSpacer = new QSpacerItem(0, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mpFilterBoxesLayout->addSpacerItem(pSpacer);
}

void CResourceBrowser::CreateAddMenu()
{
    // Delete any existing menu
    if (mpAddMenu)
    {
        delete mpAddMenu;
        mpAddMenu = nullptr;
    }

    // Create new one, only if we have a valid resource store.
    if (mpStore)
    {
        mpAddMenu = new QMenu(this);
        mpAddMenu->addAction(tr("New Folder"), this, &CResourceBrowser::CreateDirectory);
        mpAddMenu->addSeparator();

        QMenu* pCreateMenu = new QMenu(tr("Create..."), mpAddMenu);
        mpAddMenu->addMenu(pCreateMenu);
        AddCreateAssetMenuActions(pCreateMenu);

        mpUI->AddButton->setMenu(mpAddMenu);
    }

    mpUI->AddButton->setEnabled( mpAddMenu != nullptr );
}

void CResourceBrowser::AddCreateAssetMenuActions(QMenu* pMenu)
{
    std::list<CResTypeInfo*> TypeInfos;
    CResTypeInfo::GetAllTypesInGame(mpStore->Game(), TypeInfos);

    for (const auto* typeInfo : TypeInfos)
    {
        if (typeInfo->CanBeCreated())
        {
            QString TypeName = TO_QSTRING(typeInfo->TypeName());
            QAction* pAction = pMenu->addAction(TypeName, this, &CResourceBrowser::OnCreateAssetAction);
            pAction->setProperty("TypeInfo", QVariant(static_cast<int>(typeInfo->Type())));
        }
    }
}

bool CResourceBrowser::RenameResource(CResourceEntry *pEntry, const TString& rkNewName)
{
    if (pEntry->Name() == rkNewName)
        return false;

    // Check if the move is valid
    if (!pEntry->CanMoveTo(pEntry->DirectoryPath(), rkNewName))
    {
        if (pEntry->Directory()->FindChildResource(rkNewName, pEntry->ResourceType()) != nullptr)
        {
            UICommon::ErrorMsg(this, tr("Failed to rename; the destination directory has conflicting files!"));
            return false;
        }
        else
        {
            UICommon::ErrorMsg(this, tr("Failed to rename; filename is invalid!"));
            return false;
        }
    }

    // Everything seems to be valid; proceed with the rename
    mUndoStack.beginMacro(tr("Rename Resource"));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.push(new CRenameResourceCommand(pEntry, rkNewName));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.endMacro();
    return true;
}

bool CResourceBrowser::RenameDirectory(CVirtualDirectory *pDir, const TString& rkNewName)
{
    if (pDir->Name() == rkNewName)
        return false;

    if (!CVirtualDirectory::IsValidDirectoryName(rkNewName))
    {
        UICommon::ErrorMsg(this, tr("Failed to rename; directory name is invalid!"));
        return false;
    }

    // Check for conflicts
    if (pDir->Parent()->FindChildDirectory(rkNewName, false) != nullptr)
    {
        UICommon::ErrorMsg(this, tr("Failed to rename; the destination directory has a conflicting directory!"));
        return false;
    }

    // No conflicts, proceed with the rename
    mUndoStack.beginMacro(tr("Rename Directory"));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.push(new CRenameDirectoryCommand(pDir, rkNewName));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.endMacro();
    return true;
}

bool CResourceBrowser::MoveResources(const QList<CResourceEntry*>& rkResources, const QList<CVirtualDirectory*>& rkDirectories, CVirtualDirectory *pNewDir)
{
    // Check for any conflicts
    QList<CResourceEntry*> ConflictingResources;
    QList<CResourceEntry*> ValidResources;

    for (CResourceEntry *pEntry : rkResources)
    {
        CResourceEntry *pConflict = pNewDir->FindChildResource(pEntry->Name(), pEntry->ResourceType());

        if (pConflict != pEntry)
        {
            if (pConflict != nullptr)
                ConflictingResources.push_back(pEntry);
            else
                ValidResources.push_back(pEntry);
        }
    }

    QList<CVirtualDirectory*> ConflictingDirs;
    QList<CVirtualDirectory*> ValidDirs;

    for (CVirtualDirectory *pDir : rkDirectories)
    {
        CVirtualDirectory *pConflict = pNewDir->FindChildDirectory(pDir->Name(), false);

        if (pConflict != pDir)
        {
            if (pConflict != nullptr)
                ConflictingDirs.push_back(pDir);
            else
                ValidDirs.push_back(pDir);
        }
    }

    // If there were conflicts, notify the user of them
    if (!ConflictingResources.isEmpty() || !ConflictingDirs.isEmpty())
    {
        QString ErrorMsg = tr("Failed to move; the destination directory has conflicting files.\n\n");

        for (const CVirtualDirectory *pDir : ConflictingDirs)
        {
            ErrorMsg += tr("* %1").arg(TO_QSTRING(pDir->Name()));
        }

        for (const CResourceEntry *pEntry : ConflictingResources)
        {
            ErrorMsg += tr("* %1.%2\n").arg(TO_QSTRING(pEntry->Name())).arg(TO_QSTRING(pEntry->CookedExtension().ToString()));
        }

        UICommon::ErrorMsg(this, ErrorMsg);
        return false;
    }

    // Create undo actions to actually perform the moves
    if (!ValidResources.isEmpty() || !ValidDirs.isEmpty())
    {
        mUndoStack.beginMacro(tr("Move Resources"));
        mUndoStack.push(new CSaveStoreCommand(mpStore));

        for (CVirtualDirectory* pDir : ValidDirs)
            mUndoStack.push(new CMoveDirectoryCommand(mpStore, pDir, pNewDir));

        for (CResourceEntry* pEntry : ValidResources)
            mUndoStack.push(new CMoveResourceCommand(pEntry, pNewDir));

        mUndoStack.push(new CSaveStoreCommand(mpStore));
        mUndoStack.endMacro();
    }

    return true;
}

CResourceEntry* CResourceBrowser::CreateNewResource(EResourceType Type,
                                                    TString Name /*= ""*/,
                                                    CVirtualDirectory* pDir /*= nullptr*/,
                                                    CAssetID ID /*= CAssetID()*/)
{
    if (!pDir)
    {
        pDir = mpSelectedDir;
    }

    // Create new asset ID. Sanity check to make sure the ID is unused.
    while (!ID.IsValid() || mpStore->FindEntry(ID) != nullptr)
    {
        ID = CAssetID::RandomID( mpStore->Game() );
    }

    // Boring generic default name - user will immediately be prompted to change this
    TString BaseName = Name;

    if (BaseName.IsEmpty())
    {
        BaseName = TString::Format(
            "New %s", *CResTypeInfo::FindTypeInfo(Type)->TypeName()
        );
    }

    Name = BaseName;
    int Num = 0;

    while (pDir->FindChildResource(Name, Type) != nullptr)
    {
        Num++;
        Name = TString::Format("%s (%d)", *BaseName, Num);
    }

    // Create the actual resource
    CResourceEntry* pEntry = mpStore->CreateNewResource(ID, Type, pDir->FullPath(), Name);

    // Push undo command
    mUndoStack.beginMacro(tr("Create Resource"));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.push(new CCreateResourceCommand(pEntry));
    mUndoStack.push(new CSaveStoreCommand(mpStore));
    mUndoStack.endMacro();

    pEntry->Save();

    // Select new resource so user can enter a name
    QModelIndex Index = mpModel->GetIndexForEntry(pEntry);
    ASSERT(Index.isValid());

    QModelIndex ProxyIndex = mpProxyModel->mapFromSource(Index);
    mpUI->ResourceTableView->selectionModel()->select(ProxyIndex, QItemSelectionModel::ClearAndSelect);
    mpUI->ResourceTableView->edit(ProxyIndex);

    return pEntry;
}

bool CResourceBrowser::eventFilter(QObject *pWatched, QEvent *pEvent)
{
    if (pWatched == mpUI->ResourceTableView)
    {
        if (pEvent->type() == QEvent::FocusIn || pEvent->type() == QEvent::FocusOut)
        {
            UpdateUndoActionStates();
        }
    }

    return false;
}

void CResourceBrowser::RefreshResources()
{
    // Fill resource table
    mpModel->FillEntryList(mpSelectedDir, mAssetListMode || mSearching);
}

void CResourceBrowser::RefreshDirectories()
{
    mpDirectoryModel->SetRoot(mpStore->RootDirectory());

    // Clear selection. This function is called when directories are created/deleted and our current selection might not be valid anymore
    QModelIndex RootIndex = mpDirectoryModel->index(0, 0, QModelIndex());
    mpUI->DirectoryTreeView->selectionModel()->setCurrentIndex(RootIndex, QItemSelectionModel::ClearAndSelect);
    mpUI->DirectoryTreeView->setExpanded(RootIndex, true);
}

void CResourceBrowser::UpdateDescriptionLabel()
{
    // Update main description label
    QString Desc;

    if (mpStore)
    {
        const QString ModelDesc = mpModel->ModelDescription();

        if (mSearching)
        {
            const QString SearchText = mpUI->SearchBar->text();
            Desc = tr("Searching \"%1\" in: %2").arg(SearchText).arg(ModelDesc);
        }
        else
        {
            Desc = tr("Displaying: %1").arg(ModelDesc);
        }
    }

    mpUI->TableDescriptionLabel->setText(Desc);

    // Update clear button status
    const bool CanGoUp = (mpSelectedDir && !mpSelectedDir->IsRoot());
    const bool CanClear = (!mpUI->SearchBar->text().isEmpty() || mpModel->IsDisplayingUserEntryList());
    mpUI->ClearButton->setEnabled(CanGoUp || CanClear);
    mpUI->ClearButton->setIcon(CanClear ? QIcon(QStringLiteral(":/icons/X_16px.svg")) : QIcon(QStringLiteral(":/icons/ToParentFolder_16px.svg")));
}

void CResourceBrowser::SetResourceTreeView()
{
    mAssetListMode = false;
    RefreshResources();
}

void CResourceBrowser::SetResourceListView()
{
    mAssetListMode = true;
    RefreshResources();
}

void CResourceBrowser::OnClearButtonPressed()
{
    if (!mpUI->SearchBar->text().isEmpty())
    {
        ResetSearch();
    }
    else if (mpModel->IsDisplayingUserEntryList())
    {
        RefreshResources();

        if (mpInspectedEntry)
        {
            SelectResource(mpInspectedEntry);
            mpInspectedEntry = nullptr;
        }
    }
    else
    {
        SelectDirectory(mpSelectedDir);
    }

    UpdateDescriptionLabel();
}

void CResourceBrowser::OnSortModeChanged(int Index)
{
    CResourceProxyModel::ESortMode Mode = (Index == 0 ? CResourceProxyModel::ESortMode::ByName : CResourceProxyModel::ESortMode::BySize);
    mpProxyModel->SetSortMode(Mode);
}

void CResourceBrowser::OnCreateAssetAction()
{
    // Attempt to retrieve the asset type from the sender. If successful, create the asset.
    QAction* pSender = qobject_cast<QAction*>(sender());

    if (pSender)
    {
        bool Ok;
        EResourceType Type = (EResourceType) pSender->property("TypeInfo").toInt(&Ok);

        if (Ok)
        {
            CreateNewResource(Type);
        }
    }
}

bool CResourceBrowser::CreateDirectory()
{
    if (mpSelectedDir)
    {
        TString DirNameBase = "New Folder";
        TString DirName = DirNameBase;
        uint32 AppendNum = 0;

        while (mpSelectedDir->FindChildDirectory(DirName, false) != nullptr)
        {
            AppendNum++;
            DirName = TString::Format("%s (%d)", *DirNameBase, AppendNum);
        }

        // Push create command to actually create the directory
        mUndoStack.beginMacro(tr("Create Directory"));
        mUndoStack.push(new CSaveStoreCommand(mpStore));
        CCreateDirectoryCommand *pCmd = new CCreateDirectoryCommand(mpStore, mpSelectedDir->FullPath(), DirName);
        mUndoStack.push(pCmd);
        mUndoStack.push(new CSaveStoreCommand(mpStore));
        mUndoStack.endMacro();

        // Now fetch the new directory and start editing it so the user can enter a name
        CVirtualDirectory *pNewDir = mpSelectedDir->FindChildDirectory(DirName, false);
        if (!pNewDir) return false;

        // todo: edit in the directory tree view instead if it has focus
        if (!mpUI->DirectoryTreeView->hasFocus())
        {
            QModelIndex Index = mpModel->GetIndexForDirectory(pNewDir);
            ASSERT(Index.isValid());

            QModelIndex ProxyIndex = mpProxyModel->mapFromSource(Index);
            mpUI->ResourceTableView->selectionModel()->select(ProxyIndex, QItemSelectionModel::ClearAndSelect);
            mpUI->ResourceTableView->edit(ProxyIndex);
        }

        return true;
    }

    return false;
}

bool CResourceBrowser::Delete(QVector<CResourceEntry*> Resources, QVector<CVirtualDirectory*> Directories)
{
    // Don't delete any resources/directories that are still referenced.
    // This is kind of a hack but there's no good way to clear out these references right now.
    QString ErrorPaths;

    for (int DirIdx = 0; DirIdx < Directories.size(); DirIdx++)
    {
        if (!Directories[DirIdx]->IsSafeToDelete())
        {
            ErrorPaths += TO_QSTRING(Directories[DirIdx]->FullPath()) + '\n';
            Directories.removeAt(DirIdx);
            DirIdx--;
        }
    }

    for (int ResIdx = 0; ResIdx < Resources.size(); ResIdx++)
    {
        if (Resources[ResIdx]->IsLoaded() && Resources[ResIdx]->Resource()->IsReferenced())
        {
            ErrorPaths += TO_QSTRING(Resources[ResIdx]->CookedAssetPath(true)) + '\n';
            Resources.removeAt(ResIdx);
            ResIdx--;
        }
    }

    if (!ErrorPaths.isEmpty())
    {
        // Remove trailing newline
        ErrorPaths.chop(1);
        UICommon::ErrorMsg(this, tr("The following resources/directories are still referenced and cannot be deleted:\n\n%1")
                                     .arg(ErrorPaths));
    }

    // Gather a complete list of resources in subdirectories
    for (int DirIdx = 0; DirIdx < Directories.size(); DirIdx++)
    {
        CVirtualDirectory* pDir = Directories[DirIdx];
        Resources.reserve(Resources.size() + static_cast<int>(pDir->NumResources()));
        Directories.reserve(Directories.size() + static_cast<int>(pDir->NumSubdirectories()));

        for (size_t ResourceIdx = 0; ResourceIdx < pDir->NumResources(); ResourceIdx++)
            Resources.push_back(pDir->ResourceByIndex(ResourceIdx));

        for (size_t SubdirIdx = 0; SubdirIdx < pDir->NumSubdirectories(); SubdirIdx++)
            Directories.push_back(pDir->SubdirectoryByIndex(SubdirIdx));
    }

    // Exit if we have nothing to do.
    if (Resources.isEmpty() && Directories.isEmpty())
        return false;

    // Allow the user to confirm before proceeding.
    QString ConfirmMsg = tr("Are you sure you want to permanently delete ");

    if (Resources.size() > 0)
    {
        ConfirmMsg += tr("%1 resource%2").arg(Resources.size()).arg(Resources.size() == 1 ? "" : "s");

        if (Directories.size() > 0)
        {
            ConfirmMsg += tr(" and ");
        }
    }
    if (Directories.size() > 0)
    {
        ConfirmMsg += tr("%1 %2").arg(Directories.size()).arg(Directories.size() == 1 ? tr("directory") : tr("directories"));
    }
    ConfirmMsg += tr("?");

    if (UICommon::YesNoQuestion(this, tr("Warning"), ConfirmMsg))
    {
        // Note that the undo stack will undo actions in the reverse order they are pushed
        // So we need to push commands last that we want to be undone first
        // We want to delete subdirectories first, then parent directories, then resources
        mUndoStack.beginMacro(tr("Delete"));
        mUndoStack.push(new CSaveStoreCommand(mpStore));

        // Delete resources first.
        for (CResourceEntry* pEntry : Resources)
            mUndoStack.push(new CDeleteResourceCommand(pEntry));

        // Now delete directories in reverse order (so subdirectories delete first)
        for (int DirIdx = Directories.size()-1; DirIdx >= 0; DirIdx--)
        {
            CVirtualDirectory* pDir = Directories[DirIdx];
            mUndoStack.push(new CDeleteDirectoryCommand(mpStore, pDir->Parent()->FullPath(), pDir->Name()));
        }

        mUndoStack.push(new CSaveStoreCommand(mpStore));
        mUndoStack.endMacro();
        return true;
    }

    return false;
}

void CResourceBrowser::OnSearchStringChanged(const QString& SearchString)
{
    bool WasAssetList = InAssetListMode();
    mSearching = !SearchString.isEmpty();
    bool IsAssetList = InAssetListMode();

    // Check if we need to change to/from asset list mode to display/stop displaying search results
    if (WasAssetList != IsAssetList)
    {
        RefreshResources();
    }

    UpdateFilter();
}

void CResourceBrowser::OnDirectorySelectionChanged(const QModelIndex& rkNewIndex)
{
    CVirtualDirectory *pDir = nullptr;

    if (rkNewIndex.isValid())
        pDir = mpDirectoryModel->IndexDirectory(rkNewIndex);
    else
        pDir = mpStore ? mpStore->RootDirectory() : nullptr;

    SetActiveDirectory(pDir);
}

void CResourceBrowser::OnDoubleClickTable(QModelIndex Index)
{
    QModelIndex SourceIndex = mpProxyModel->mapToSource(Index);

    // Directory - switch to the selected directory
    if (mpModel->IsIndexDirectory(SourceIndex))
    {
        CVirtualDirectory *pDir = mpModel->IndexDirectory(SourceIndex);
        CVirtualDirectory *pOldDir = mpSelectedDir;
        SetActiveDirectory(pDir);

        if (pOldDir->Parent() == pDir)
            SelectDirectory(pOldDir);
    }

    // Resource - open resource for editing
    else
    {
        CResourceEntry *pEntry = mpModel->IndexEntry(SourceIndex);
        gpEdApp->EditResource(pEntry);
    }
}

void CResourceBrowser::OnResourceSelectionChanged(const QModelIndex& rkNewIndex)
{
    QModelIndex SourceIndex = mpProxyModel->mapToSource(rkNewIndex);
    mpSelectedEntry = mpModel->IndexEntry(SourceIndex);
    emit SelectedResourceChanged(mpSelectedEntry);
}

void CResourceBrowser::FindAssetByID()
{
    if (!mpStore)
        return;

    const QString QStringAssetID = QInputDialog::getText(this, tr("Enter Asset ID"), tr("Enter asset ID:"));
    TString StringAssetID = TO_TSTRING(QStringAssetID);
    StringAssetID.RemoveWhitespace();

    if (!StringAssetID.IsEmpty())
    {
        EGame Game = mpStore->Game();
        EIDLength IDLength = CAssetID::GameIDLength(Game);
        bool WasValid = false;

        if (StringAssetID.IsHexString(false, static_cast<int>(IDLength) * 2))
        {
            if (StringAssetID.StartsWith("0x", false))
                StringAssetID = StringAssetID.ChopFront(2);

            // Find the resource entry
            if ((IDLength == EIDLength::k32Bit && StringAssetID.Length() == 8) ||
                (IDLength == EIDLength::k64Bit && StringAssetID.Length() == 16))
            {
                CAssetID ID = (IDLength == EIDLength::k32Bit ? StringAssetID.ToInt32(16) : StringAssetID.ToInt64(16));
                CResourceEntry *pEntry = mpStore->FindEntry(ID);
                WasValid = true;

                if (pEntry)
                {
                    SelectResource(pEntry, true);
                }
                else // User entered valid but unrecognized ID
                {
                    UICommon::ErrorMsg(this, tr("Couldn't find any asset with ID %1").arg(QStringAssetID));
                }
            }
        }

        // User entered invalid string
        if (!WasValid)
        {
            UICommon::ErrorMsg(this, tr("The entered string is not a valid asset ID!"));
        }
    }

    // User entered nothing, don't do anything
}

void CResourceBrowser::SetAssetIDDisplayEnabled(bool Enable)
{
    mpDelegate->SetDisplayAssetIDs(Enable);
    mpModel->RefreshAllIndices();
}

void CResourceBrowser::UpdateStore()
{
    CGameProject *pProj = gpEdApp->ActiveProject();
    CResourceStore *pProjStore = (pProj ? pProj->ResourceStore() : nullptr);
    CResourceStore *pNewStore = (mEditorStore ? gpEditorStore : pProjStore);

    if (mpStore != pNewStore)
    {
        mpStore = pNewStore;

        // Clear search
        mpUI->SearchBar->clear();
        mSearching = false;

        // Refresh project-specific UI
        CreateAddMenu();
        CreateFilterCheckboxes();
        setEnabled(mpStore != nullptr);

        // Refresh directory tree
        mpDirectoryModel->SetRoot(mpStore ? mpStore->RootDirectory() : nullptr);
        QModelIndex RootIndex = mpDirectoryModel->index(0, 0, QModelIndex());
        mpUI->DirectoryTreeView->expand(RootIndex);
        mpUI->DirectoryTreeView->clearSelection();
        OnDirectorySelectionChanged(QModelIndex());
    }
}

void CResourceBrowser::SetProjectStore()
{
    mEditorStore = false;
    UpdateStore();
}

void CResourceBrowser::SetEditorStore()
{
    mEditorStore = true;
    UpdateStore();
}

void CResourceBrowser::ImportPackageContentsList()
{
    const QStringList PathList = UICommon::OpenFilesDialog(this, tr("Open package contents list"), QStringLiteral("*.pak.contents.txt"));
    if (PathList.isEmpty())
        return;

    SetActiveDirectory(nullptr);

    for (const QString& rkPath : PathList)
        mpStore->ImportNamesFromPakContentsTxt(TO_TSTRING(rkPath), false);

    RefreshResources();
    RefreshDirectories();
}

void CResourceBrowser::GenerateAssetNames()
{
    SetActiveDirectory(nullptr);

    CProgressDialog Dialog(tr("Generating asset names"), true, true, this);
    Dialog.DisallowCanceling();
    Dialog.SetOneShotTask("Generating asset names");

    // Temporarily set root to null to ensure the window doesn't access the resource store while we're running.
    mpDirectoryModel->SetRoot(mpStore->RootDirectory());

    QFuture<void> Future = QtConcurrent::run(&::GenerateAssetNames, mpStore->Project());
    Dialog.WaitForResults(Future);

    RefreshResources();
    RefreshDirectories();

    UICommon::InfoMsg(this, tr("Complete"), tr("Asset name generation complete!"));
}

void CResourceBrowser::ImportAssetNameMap()
{
    CAssetNameMap Map(mpStore->Game());
    const bool LoadSuccess = Map.LoadAssetNames();

    if (!LoadSuccess)
    {
        UICommon::ErrorMsg(this, tr("Import failed; couldn't load asset name map!"));
        return;
    }

    if (!Map.IsValid())
    {
        UICommon::ErrorMsg(this, tr("Import failed; the input asset name map is invalid! See the log for details."));
        return;
    }

    SetActiveDirectory(nullptr);

    for (CResourceIterator It(mpStore); It; ++It)
    {
        TString Dir, Name;
        bool AutoDir, AutoName;

        if (Map.GetNameInfo(It->ID(), Dir, Name, AutoDir, AutoName))
            It->MoveAndRename(Dir, Name, AutoDir, AutoName);
    }

    mpStore->ConditionalSaveStore();
    RefreshResources();
    RefreshDirectories();
    UICommon::InfoMsg(this, tr("Success"), tr("New asset names imported successfully!"));
}

void CResourceBrowser::ExportAssetNames()
{
    const QString OutFile = UICommon::SaveFileDialog(this, tr("Export asset name map"), QStringLiteral("*.xml"),
                                                     gResourcesWritable ? *(gDataDir + "resources/gameinfo/") : "");
    if (OutFile.isEmpty())
        return;

    TString OutFileStr = TO_TSTRING(OutFile);
    CAssetNameMap NameMap(mpStore->Game());

    if (FileUtil::Exists(OutFileStr))
    {
        bool LoadSuccess = NameMap.LoadAssetNames(OutFileStr);

        if (!LoadSuccess || !NameMap.IsValid())
        {
            UICommon::ErrorMsg(this, tr("Unable to export; failed to load existing names from the original asset name map file! See the log for details."));
            return;
        }
    }

    NameMap.CopyFromStore(mpStore);
    bool SaveSuccess = NameMap.SaveAssetNames(OutFileStr);

    if (!SaveSuccess)
        UICommon::ErrorMsg(this, tr("Failed to export asset names!"));
    else
        UICommon::InfoMsg(this, tr("Success"), tr("Asset names exported successfully!"));
}

void CResourceBrowser::RebuildResourceDB()
{
    if (UICommon::YesNoQuestion(this, tr("Rebuild resource database"), tr("Are you sure you want to rebuild the resource database? This will take a while.")))
    {
        gpEdApp->RebuildResourceDatabase();
    }
}

void CResourceBrowser::ClearFilters()
{
    ResetSearch();
    ResetTypeFilter();
}

void CResourceBrowser::ResetSearch()
{
    bool WasAssetList = InAssetListMode();
    mpUI->SearchBar->clear();
    mSearching = false;
    bool IsAssetList = InAssetListMode();

    if (IsAssetList != WasAssetList)
        RefreshResources();

    UpdateFilter();
}

void CResourceBrowser::ResetTypeFilter()
{
    mpFilterAllBox->setChecked(true);
    UpdateFilter();
}

void CResourceBrowser::OnFilterTypeBoxTicked(bool Checked)
{
    // NOTE: there should only be one CResourceBrowser; if that ever changes for some reason change this to a member
    static bool ReentrantGuard = false;
    if (ReentrantGuard) return;
    ReentrantGuard = true;

    if (sender() == mpFilterAllBox)
    {
        if (!Checked && !mpProxyModel->HasTypeFilter())
            mpFilterAllBox->setChecked(true);

        else if (Checked)
        {
            for (const SResourceType& rkType : mTypeList)
            {
                rkType.pFilterCheckBox->setChecked(false);
                mpProxyModel->SetTypeFilter(rkType.pTypeInfo, false);
            }
        }
    }

    else
    {
        for (const SResourceType& rkType : mTypeList)
        {
            if (rkType.pFilterCheckBox == sender())
            {
                mpProxyModel->SetTypeFilter(rkType.pTypeInfo, Checked);
                break;
            }
        }

        mpFilterAllBox->setChecked(!mpProxyModel->HasTypeFilter());
    }

    mpProxyModel->invalidate();
    ReentrantGuard = false;
}

void CResourceBrowser::UpdateFilter()
{
    QString SearchText = mpUI->SearchBar->text();
    mSearching = !SearchText.isEmpty();

    UpdateDescriptionLabel();
    mpProxyModel->SetSearchString( TO_TSTRING(mpUI->SearchBar->text()) );
    mpProxyModel->invalidate();

    // not sure why I need to do this here? but the resize mode seems to get reset otherwise
    mpUI->ResourceTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void CResourceBrowser::UpdateUndoActionStates()
{
    // Make sure that the undo actions are only enabled when the table view has focus.
    // This is to prevent them from conflicting with world editor undo/redo actions.
    bool HasFocus = (mpUI->ResourceTableView->hasFocus());
    mpUndoAction->setEnabled( HasFocus && mUndoStack.canUndo() );
    mpRedoAction->setEnabled( HasFocus && mUndoStack.canRedo() );
}

void CResourceBrowser::Undo()
{
    mUndoStack.undo();
    UpdateUndoActionStates();
}

void CResourceBrowser::Redo()
{
    mUndoStack.redo();
    UpdateUndoActionStates();
}
