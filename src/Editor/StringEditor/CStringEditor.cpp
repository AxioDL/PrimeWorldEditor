#include "CStringEditor.h"
#include "ui_CStringEditor.h"

#include "CStringDelegate.h"
#include "Editor/UICommon.h"
#include "Editor/Undo/TSerializeUndoCommand.h"

#include <QSettings>
#include <QShortcut>

/** Settings strings */
constexpr char gkpLanguageSetting[] = "StringEditor/EditLanguage";

/** Command classes */
class CSetStringIndexCommand : public IUndoCommand
{
    CStringEditor* mpEditor;
    int mOldIndex, mNewIndex;
public:
    CSetStringIndexCommand(CStringEditor* pEditor, int OldIndex, int NewIndex)
        : IUndoCommand("Select String"), mpEditor(pEditor), mOldIndex(OldIndex), mNewIndex(NewIndex)
    {}

    void undo() override { mpEditor->SetActiveString(mOldIndex); }
    void redo() override { mpEditor->SetActiveString(mNewIndex); }
    bool AffectsCleanState() const override { return false; }
};

class CSetLanguageCommand : public IUndoCommand
{
    CStringEditor* mpEditor;
    ELanguage mOldLanguage, mNewLanguage;
public:
    CSetLanguageCommand(CStringEditor* pEditor, ELanguage OldLanguage, ELanguage NewLanguage)
        : IUndoCommand("Select Language"), mpEditor(pEditor), mOldLanguage(OldLanguage), mNewLanguage(NewLanguage)
    {}

    void undo() override { mpEditor->SetActiveLanguage(mOldLanguage); }
    void redo() override { mpEditor->SetActiveLanguage(mNewLanguage); }
    bool AffectsCleanState() const override { return false; }
};

/** Constructor */
CStringEditor::CStringEditor(CStringTable* pStringTable, QWidget* pParent)
    : IEditor(pParent)
    , mpUI(std::make_unique<Ui::CStringEditor>())
    , mpStringTable(pStringTable)
{
    InitUI();
//  LoadSettings(); // Disabled for now
}

CStringEditor::~CStringEditor() = default;

bool CStringEditor::Save()
{
    if (!mpStringTable->Entry()->Save())
    {
        UICommon::ErrorMsg(this, tr("Failed to save!"));
        return false;
    }
    else
    {
        UndoStack().setClean();
        setWindowModified(false);
        return true;
    }
}

bool CStringEditor::eventFilter(QObject* pWatched, QEvent* pEvent)
{
    if (pEvent->type() == QEvent::FocusOut)
    {
        if (pWatched == mpUI->StringNameLineEdit && mIsEditingStringName)
        {
            IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Edit String Name"), mpStringTable, true);
            UndoStack().push(pCommand);
            mIsEditingStringName = false;
            return true;
        }
        else if (pWatched == mpUI->StringTextEdit && mIsEditingStringData)
        {
            IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Edit String"), mpStringTable, true);
            UndoStack().push(pCommand);
            mIsEditingStringData = false;
            return true;
        }
    }
    return false;
}

void CStringEditor::InitUI()
{
    mpUI->setupUi(this);
    mpListModel = new CStringListModel(this);
    mpUI->StringNameListView->setModel(mpListModel);
    mpUI->StringNameListView->setItemDelegate(new CStringDelegate(this));
    mpUI->AddStringButton->setShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Equal));
    mpUI->RemoveStringButton->setShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Minus));

    // Register shortcuts
    new QShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Down), this, SLOT(IncrementStringIndex()));
    new QShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Up), this, SLOT(DecrementStringIndex()));
    new QShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Right), this, SLOT(IncrementLanguageIndex()));
    new QShortcut(QKeySequence(Qt::Key_Alt, Qt::Key_Left), this, SLOT(DecrementLanguageIndex()));

    // Set up language tabs
    mpUI->EditLanguageTabBar->setExpanding(false);

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        const ELanguage Language = mpStringTable->LanguageByIndex(LanguageIdx);
        const char* pkLanguageName = TEnumReflection<ELanguage>::ConvertValueToString(Language);
        mpUI->EditLanguageTabBar->addTab(pkLanguageName);
    }

    // Connect signals & slots
    connect(mpUI->StringNameListView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &CStringEditor::OnStringSelected);

    connect(mpUI->StringNameLineEdit, &QLineEdit::textChanged, this, &CStringEditor::OnStringNameEdited);
    connect(mpUI->StringTextEdit, &QPlainTextEdit::textChanged, this, &CStringEditor::OnStringTextEdited);
    connect(mpUI->EditLanguageTabBar, &QTabBar::currentChanged, this, &CStringEditor::OnLanguageChanged);
    connect(mpUI->AddStringButton, &QPushButton::pressed, this, &CStringEditor::OnAddString);
    connect(mpUI->RemoveStringButton, &QPushButton::pressed, this, &CStringEditor::OnRemoveString);

    connect(mpUI->ActionSave, &QAction::triggered, this, &CStringEditor::Save);
    connect(mpUI->ActionSaveAndCook, &QAction::triggered, this, &CStringEditor::SaveAndRepack);

    connect(&UndoStack(), &QUndoStack::indexChanged, this, &CStringEditor::UpdateUI);

    mpUI->ToolBar->addSeparator();
    AddUndoActions(mpUI->ToolBar);

    // Install event filters
    mpUI->StringNameLineEdit->installEventFilter(this);
    mpUI->StringTextEdit->installEventFilter(this);

    // Update window title
    const QString WindowTitle = tr("%APP_FULL_NAME% - String Editor - %1[*]")
                                    .arg(TO_QSTRING(mpStringTable->Entry()->CookedAssetPath(true).GetFileName()));
    SET_WINDOWTITLE_APPVARS(WindowTitle);

    // Initialize the splitter so top split takes as much space as possible
    const QList<int> SplitterSizes{static_cast<int>(height() * 0.95), static_cast<int>(height() * 0.05)};
    mpUI->splitter->setSizes(SplitterSizes);

    // Initialize UI
    UpdateUI();
}

void CStringEditor::UpdateStatusBar()
{
    // Update status bar
    QString StatusText = tr("%1 languages, %2 strings")
            .arg(mpStringTable->NumLanguages())
            .arg(mpStringTable->NumStrings());

    if (mCurrentStringIndex >= 0)
    {
        StatusText += tr("; editing string #%1 in %2")
                .arg(mCurrentStringIndex + 1)
                .arg(TEnumReflection<ELanguage>::ConvertValueToString(mCurrentLanguage));
    }

    mpUI->StatusBar->showMessage(StatusText);
}

void CStringEditor::SetActiveLanguage(ELanguage Language)
{
    if (mCurrentLanguage == Language)
        return;

    mCurrentLanguage = Language;
    mpListModel->SetPreviewLanguage(Language);
    UpdateUI();
    SaveSettings();
}

void CStringEditor::SetActiveString(int StringIndex)
{
    mCurrentStringIndex = StringIndex;
    UpdateUI();
}

void CStringEditor::LoadSettings()
{
    QSettings Settings;

    // Set language
    const auto Language = static_cast<ELanguage>(Settings.value(gkpLanguageSetting, static_cast<int>(ELanguage::English)).toInt());

    for (size_t LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        if (mpStringTable->LanguageByIndex(LanguageIdx) == Language)
        {
            SetActiveLanguage(Language);
            mpUI->EditLanguageTabBar->setCurrentIndex(static_cast<int>(LanguageIdx));
            break;
        }
    }
}

void CStringEditor::SaveSettings()
{
    QSettings Settings;
    Settings.setValue(gkpLanguageSetting, static_cast<int>(mCurrentLanguage));
}

/** Slots */
void CStringEditor::UpdateUI()
{
    // Update string list
    if (mCurrentStringCount != mpStringTable->NumStrings())
    {
        mpUI->StringNameListView->model()->layoutChanged();
        mCurrentStringCount = mpStringTable->NumStrings();
    }

    // Update selection in string list
    QItemSelectionModel* pSelectionModel = mpUI->StringNameListView->selectionModel();

    const QModelIndex OldStringIndex = pSelectionModel->hasSelection()
                                     ? pSelectionModel->currentIndex()
                                     : QModelIndex();

    const QModelIndex NewStringIndex = mpUI->StringNameListView->model()->index(mCurrentStringIndex,0);

    if (OldStringIndex != NewStringIndex)
    {
        {
            [[maybe_unused]] const QSignalBlocker blocker{pSelectionModel};
            pSelectionModel->setCurrentIndex(NewStringIndex, QItemSelectionModel::ClearAndSelect);
        }

        mpUI->StringNameListView->scrollTo(NewStringIndex);
        mpUI->StringNameListView->update(OldStringIndex);
    }
    mpUI->StringNameListView->update(NewStringIndex);
    mpUI->RemoveStringButton->setEnabled( pSelectionModel->hasSelection() );

    // Update language tabs
    const auto LanguageIndex = static_cast<size_t>(mpUI->EditLanguageTabBar->currentIndex());
    const ELanguage TabLanguage = mpStringTable->LanguageByIndex(LanguageIndex);

    if (TabLanguage != mCurrentLanguage)
    {
        for (size_t LangIdx = 0; LangIdx < mpStringTable->NumLanguages(); LangIdx++)
        {
            if (mpStringTable->LanguageByIndex(LangIdx) == mCurrentLanguage)
            {
                [[maybe_unused]] const QSignalBlocker blocker{mpUI->EditLanguageTabBar};
                mpUI->EditLanguageTabBar->setCurrentIndex(static_cast<int>(LangIdx));
                break;
            }
        }
    }

    // Update string name/data fields
    const QString StringName = TO_QSTRING(mpStringTable->StringNameByIndex(mCurrentStringIndex));
    const QString StringData = TO_QSTRING(mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex));

    if (StringName != mpUI->StringNameLineEdit->text())
    {
        [[maybe_unused]] const QSignalBlocker blocker{mpUI->StringNameLineEdit};
        mpUI->StringNameLineEdit->setText(StringName);
    }

    if (StringData != mpUI->StringTextEdit->toPlainText())
    {
        [[maybe_unused]] const QSignalBlocker blocker{mpUI->StringTextEdit};
        mpUI->StringTextEdit->setPlainText(StringData);
    }

    UpdateStatusBar();
}

void CStringEditor::OnStringSelected(const QModelIndex& kIndex)
{
    if (mCurrentStringIndex == static_cast<uint32>(kIndex.row()))
        return;

    IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, kIndex.row());
    UndoStack().push(pCommand);
}

void CStringEditor::OnLanguageChanged(int LanguageIndex)
{
    ASSERT(LanguageIndex >= 0 && LanguageIndex < static_cast<int>(mpStringTable->NumLanguages()));
    const ELanguage Language = mpStringTable->LanguageByIndex(LanguageIndex);

    if (Language == mCurrentLanguage)
        return;

    IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, Language);
    UndoStack().push(pCommand);
}

void CStringEditor::OnStringNameEdited()
{
    TString NewName = TO_TSTRING(mpUI->StringNameLineEdit->text());
    
    if (mpStringTable->StringNameByIndex(mCurrentStringIndex) == NewName)
        return;

    IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Edit String Name"), mpStringTable, false);
    mpStringTable->SetStringName(mCurrentStringIndex, std::move(NewName));
    mIsEditingStringName = true;
    UndoStack().push(pCommand);
}

void CStringEditor::OnStringTextEdited()
{
    TString NewText = TO_TSTRING(mpUI->StringTextEdit->toPlainText());
    
    if (mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex) == NewText)
        return;

    IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Edit String"), mpStringTable, false);
    mpStringTable->SetString(mCurrentLanguage, mCurrentStringIndex, std::move(NewText));
    mIsEditingStringData = true;
    UndoStack().push(pCommand);
}

void CStringEditor::OnAddString()
{
    UndoStack().beginMacro(tr("Add String"));

    // Add string
    IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Add String"), mpStringTable, true);
    const uint32 Index = mCurrentStringIndex + 1;
    mpStringTable->AddString(Index);
    UndoStack().push(pCommand);

    // Select new string
    pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, Index);
    UndoStack().push(pCommand);
    UndoStack().endMacro();

    // Give focus to the text edit so the user can edit the string
    mpUI->StringTextEdit->setFocus();
}

void CStringEditor::OnRemoveString()
{
    if (!mpUI->StringNameListView->selectionModel()->hasSelection())
        return;

    UndoStack().beginMacro(tr("Remove String"));
    const size_t Index = mCurrentStringIndex;

    // Change selection to a new string.
    // Do this before actually removing the string so that if the action is undone, the
    // editor will not attempt to re-select the string before it gets readded to the table.
    const size_t NewStringCount = mpStringTable->NumStrings() - 1;
    const size_t NewIndex = (Index >= NewStringCount ? NewStringCount - 1 : Index);
    IUndoCommand* pCommand = new CSetStringIndexCommand(this, Index, NewIndex);
    UndoStack().push(pCommand);

    // Remove the string
    pCommand = new TSerializeUndoCommand<CStringTable>(tr("Remove String"), mpStringTable, true);
    mpStringTable->RemoveString(Index);
    UndoStack().push(pCommand);
    UndoStack().endMacro();
}

void CStringEditor::OnMoveString(int StringIndex, int NewIndex)
{
    if (StringIndex == NewIndex)
        return;

    ASSERT(StringIndex >= 0 && StringIndex < static_cast<int>(mpStringTable->NumStrings()));
    ASSERT(NewIndex >= 0 && NewIndex < static_cast<int>(mpStringTable->NumStrings()));
    UndoStack().beginMacro(tr("Move String"));

    // Move string
    IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>(tr("Move String"), mpStringTable, true);
    mpStringTable->MoveString(StringIndex, NewIndex);
    UndoStack().push(pCommand);

    // Select new string index
    pCommand = new CSetStringIndexCommand(this, StringIndex, NewIndex);
    UndoStack().push(pCommand);
    UndoStack().endMacro();
}

void CStringEditor::IncrementStringIndex()
{
    const uint32 NewIndex = mCurrentStringIndex + 1;

    if (NewIndex >= mpStringTable->NumStrings())
        return;

    IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, NewIndex);
    UndoStack().push(pCommand);
}

void CStringEditor::DecrementStringIndex()
{
    const uint32 NewIndex = mCurrentStringIndex - 1;

    if (NewIndex == UINT32_MAX)
        return;

    IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, NewIndex);
    UndoStack().push(pCommand);
}

void CStringEditor::IncrementLanguageIndex()
{
    for (size_t i = 0; i < mpStringTable->NumLanguages() - 1; i++)
    {
        if (mpStringTable->LanguageByIndex(i) != mCurrentLanguage)
            continue;

        const ELanguage NewLanguage = mpStringTable->LanguageByIndex(i + 1);
        IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, NewLanguage);
        UndoStack().push(pCommand);
        break;
    }
}

void CStringEditor::DecrementLanguageIndex()
{
    for (size_t i = mpStringTable->NumLanguages() - 1; i > 0; i--)
    {
        if (mpStringTable->LanguageByIndex(i) != mCurrentLanguage)
            continue;

        const ELanguage NewLanguage = mpStringTable->LanguageByIndex(i - 1);
        IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, NewLanguage);
        UndoStack().push(pCommand);
        break;
    }
}
