#include "CStringEditor.h"
#include "ui_CStringEditor.h"

#include "CStringDelegate.h"
#include "Editor/UICommon.h"
#include "Editor/Undo/TSerializeUndoCommand.h"

#include <QSettings>
#include <QShortcut>

/** Settings strings */
const char* gkpLanguageSetting = "StringEditor/EditLanguage";

/** Command classes */
class CSetStringIndexCommand : public IUndoCommand
{
    CStringEditor* mpEditor;
    int mOldIndex, mNewIndex;
public:
    CSetStringIndexCommand(CStringEditor* pEditor, int OldIndex, int NewIndex)
        : IUndoCommand("Select String"), mpEditor(pEditor), mOldIndex(OldIndex), mNewIndex(NewIndex)
    {}

    virtual void undo() override { mpEditor->SetActiveString(mOldIndex); }
    virtual void redo() override { mpEditor->SetActiveString(mNewIndex); }
    virtual bool AffectsCleanState() const override { return false; }
};

class CSetLanguageCommand : public IUndoCommand
{
    CStringEditor* mpEditor;
    ELanguage mOldLanguage, mNewLanguage;
public:
    CSetLanguageCommand(CStringEditor* pEditor, ELanguage OldLanguage, ELanguage NewLanguage)
        : IUndoCommand("Select Language"), mpEditor(pEditor), mOldLanguage(OldLanguage), mNewLanguage(NewLanguage)
    {}

    virtual void undo() override { mpEditor->SetActiveLanguage(mOldLanguage); }
    virtual void redo() override { mpEditor->SetActiveLanguage(mNewLanguage); }
    virtual bool AffectsCleanState() const override { return false; }
};

/** Constructor */
CStringEditor::CStringEditor(CStringTable* pStringTable, QWidget* pParent)
    : IEditor(pParent)
    , mpUI(new Ui::CStringEditor)
    , mpStringTable(pStringTable)
    , mCurrentLanguage(ELanguage::English)
    , mCurrentStringIndex(0)
    , mCurrentStringCount(0)
    , mIsEditingStringName(false)
    , mIsEditingStringData(false)
{
    InitUI();
//  LoadSettings(); // Disabled for now
}

CStringEditor::~CStringEditor()
{
    delete mpUI;
}

bool CStringEditor::Save()
{
    if (!mpStringTable->Entry()->Save())
    {
        UICommon::ErrorMsg(this, "Failed to save!");
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
            IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Edit String Name", mpStringTable, true);
            UndoStack().push(pCommand);
            mIsEditingStringName = false;
            return true;
        }
        else if (pWatched == mpUI->StringTextEdit && mIsEditingStringData)
        {
            IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Edit String", mpStringTable, true);
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
    mpUI->StringNameListView->setItemDelegate( new CStringDelegate(this) );
    mpUI->AddStringButton->setShortcut( QKeySequence("Alt+=") );
    mpUI->RemoveStringButton->setShortcut( QKeySequence("Alt+-") );

    // Register shortcuts
    new QShortcut( QKeySequence("Alt+Down"),  this, SLOT(IncrementStringIndex()) );
    new QShortcut( QKeySequence("Alt+Up"),    this, SLOT(DecrementStringIndex()) );
    new QShortcut( QKeySequence("Alt+Right"), this, SLOT(IncrementLanguageIndex()) );
    new QShortcut( QKeySequence("Alt+Left"),  this, SLOT(DecrementLanguageIndex()) );

    // Set up language tabs
    mpUI->EditLanguageTabBar->setExpanding(false);

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        ELanguage Language = mpStringTable->LanguageByIndex(LanguageIdx);
        const char* pkLanguageName = TEnumReflection<ELanguage>::ConvertValueToString(Language);
        mpUI->EditLanguageTabBar->addTab(pkLanguageName);
    }

    // Connect signals & slots
    connect( mpUI->StringNameListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(OnStringSelected(QModelIndex)) );

    connect( mpUI->StringNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(OnStringNameEdited()) );
    connect( mpUI->StringTextEdit, SIGNAL(textChanged()), this, SLOT(OnStringTextEdited()) );
    connect( mpUI->EditLanguageTabBar, SIGNAL(currentChanged(int)), this, SLOT(OnLanguageChanged(int)) );
    connect( mpUI->AddStringButton, SIGNAL(pressed()), this, SLOT(OnAddString()) );
    connect( mpUI->RemoveStringButton, SIGNAL(pressed()), this, SLOT(OnRemoveString()) );

    connect( mpUI->ActionSave, SIGNAL(triggered(bool)), this, SLOT(Save()) );
    connect( mpUI->ActionSaveAndCook, SIGNAL(triggered(bool)), this, SLOT(SaveAndRepack()) );

    connect( &UndoStack(), SIGNAL(indexChanged(int)), this, SLOT(UpdateUI()) );

    mpUI->ToolBar->addSeparator();
    AddUndoActions(mpUI->ToolBar);

    // Install event filters
    mpUI->StringNameLineEdit->installEventFilter(this);
    mpUI->StringTextEdit->installEventFilter(this);

    // Update window title
    QString WindowTitle = "%APP_FULL_NAME% - String Editor - %1[*]";
    WindowTitle = WindowTitle.arg( TO_QSTRING(mpStringTable->Entry()->CookedAssetPath(true).GetFileName()) );
    SET_WINDOWTITLE_APPVARS(WindowTitle);

    // Initialize the splitter so top split takes as much space as possible
    QList<int> SplitterSizes;
    SplitterSizes << (height() * 0.95) << (height() * 0.05);
    mpUI->splitter->setSizes(SplitterSizes);

    // Initialize UI
    UpdateUI();
}

void CStringEditor::UpdateStatusBar()
{
    // Update status bar
    QString StatusText = QString("%1 languages, %2 strings")
            .arg(mpStringTable->NumLanguages())
            .arg(mpStringTable->NumStrings());

    if (mCurrentStringIndex >= 0)
    {
        StatusText += QString("; editing string #%1 in %2")
                .arg(mCurrentStringIndex + 1)
                .arg(TEnumReflection<ELanguage>::ConvertValueToString(mCurrentLanguage));
    }

    mpUI->StatusBar->showMessage(StatusText);
}

void CStringEditor::SetActiveLanguage(ELanguage Language)
{
    if (mCurrentLanguage != Language)
    {
        mCurrentLanguage = Language;
        mpListModel->SetPreviewLanguage(Language);
        UpdateUI();
        SaveSettings();
    }
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
    ELanguage Language = (ELanguage) Settings.value(gkpLanguageSetting, (int) ELanguage::English).toInt();

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        if (mpStringTable->LanguageByIndex(LanguageIdx) == Language)
        {
            SetActiveLanguage(Language);
            mpUI->EditLanguageTabBar->setCurrentIndex(LanguageIdx);
            break;
        }
    }
}

void CStringEditor::SaveSettings()
{
    QSettings Settings;
    Settings.setValue(gkpLanguageSetting, (int) mCurrentLanguage);
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

    QModelIndex OldStringIndex = pSelectionModel->hasSelection() ?
                pSelectionModel->currentIndex() : QModelIndex();

    QModelIndex NewStringIndex = mpUI->StringNameListView->model()->index(mCurrentStringIndex,0);

    if (OldStringIndex != NewStringIndex)
    {
        pSelectionModel->blockSignals(true);
        pSelectionModel->setCurrentIndex(NewStringIndex, QItemSelectionModel::ClearAndSelect);
        pSelectionModel->blockSignals(false);
        mpUI->StringNameListView->scrollTo(NewStringIndex);
        mpUI->StringNameListView->update(OldStringIndex);
    }
    mpUI->StringNameListView->update(NewStringIndex);
    mpUI->RemoveStringButton->setEnabled( pSelectionModel->hasSelection() );

    // Update language tabs
    uint LanguageIndex = mpUI->EditLanguageTabBar->currentIndex();
    ELanguage TabLanguage = mpStringTable->LanguageByIndex(LanguageIndex);

    if (TabLanguage != mCurrentLanguage)
    {
        for (uint LangIdx = 0; LangIdx < mpStringTable->NumLanguages(); LangIdx++)
        {
            if (mpStringTable->LanguageByIndex(LangIdx) == mCurrentLanguage)
            {
                mpUI->EditLanguageTabBar->blockSignals(true);
                mpUI->EditLanguageTabBar->setCurrentIndex(LangIdx);
                mpUI->EditLanguageTabBar->blockSignals(false);
                break;
            }
        }
    }

    // Update string name/data fields
    QString StringName = TO_QSTRING( mpStringTable->StringNameByIndex(mCurrentStringIndex) );
    QString StringData = TO_QSTRING( mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex) );

    if (StringName != mpUI->StringNameLineEdit->text())
    {
        mpUI->StringNameLineEdit->blockSignals(true);
        mpUI->StringNameLineEdit->setText( StringName );
        mpUI->StringNameLineEdit->blockSignals(false);
    }

    if (StringData != mpUI->StringTextEdit->toPlainText())
    {
        mpUI->StringTextEdit->blockSignals(true);
        mpUI->StringTextEdit->setPlainText( StringData );
        mpUI->StringTextEdit->blockSignals(false);
    }

    UpdateStatusBar();
}

void CStringEditor::OnStringSelected(const QModelIndex& kIndex)
{
    if (mCurrentStringIndex != kIndex.row())
    {
        IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, kIndex.row());
        UndoStack().push(pCommand);
    }
}

void CStringEditor::OnLanguageChanged(int LanguageIndex)
{
    ASSERT( LanguageIndex >= 0 && LanguageIndex < (int) mpStringTable->NumLanguages() );
    ELanguage Language = mpStringTable->LanguageByIndex(LanguageIndex);

    if (Language != mCurrentLanguage)
    {
        IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, Language);
        UndoStack().push(pCommand);
    }
}

void CStringEditor::OnStringNameEdited()
{
    TString NewName = TO_TSTRING( mpUI->StringNameLineEdit->text() );
    
    if (mpStringTable->StringNameByIndex(mCurrentStringIndex) != NewName)
    {
        IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Edit String Name", mpStringTable, false);
        mpStringTable->SetStringName( mCurrentStringIndex, NewName );
        mIsEditingStringName = true;
        UndoStack().push(pCommand);
    }
}

void CStringEditor::OnStringTextEdited()
{
    TString NewText = TO_TSTRING( mpUI->StringTextEdit->toPlainText() );
    
    if (mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex) != NewText)
    {
        IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Edit String", mpStringTable, false);
        mpStringTable->SetString(mCurrentLanguage, mCurrentStringIndex, NewText);
        mIsEditingStringData = true;
        UndoStack().push(pCommand);
    }
}

void CStringEditor::OnAddString()
{
    UndoStack().beginMacro("Add String");

    // Add string
    IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Add String", mpStringTable, true);
    uint Index = mCurrentStringIndex + 1;
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
    if (mpUI->StringNameListView->selectionModel()->hasSelection())
    {
        UndoStack().beginMacro("Remove String");
        uint Index = mCurrentStringIndex;

        // Change selection to a new string.
        // Do this before actually removing the string so that if the action is undone, the
        // editor will not attempt to re-select the string before it gets readded to the table.
        uint NewStringCount = mpStringTable->NumStrings() - 1;
        uint NewIndex = (Index >= NewStringCount ? NewStringCount - 1 : Index);
        IUndoCommand* pCommand = new CSetStringIndexCommand(this, Index, NewIndex);
        UndoStack().push(pCommand);

        // Remove the string
        pCommand = new TSerializeUndoCommand<CStringTable>("Remove String", mpStringTable, true);
        mpStringTable->RemoveString(Index);
        UndoStack().push(pCommand);
        UndoStack().endMacro();
    }
}

void CStringEditor::OnMoveString(int StringIndex, int NewIndex)
{
    if (StringIndex != NewIndex)
    {
        ASSERT( StringIndex >= 0 && StringIndex < (int) mpStringTable->NumStrings() );
        ASSERT( NewIndex >= 0 && NewIndex < (int) mpStringTable->NumStrings() );
        UndoStack().beginMacro("Move String");

        // Move string
        IUndoCommand* pCommand = new TSerializeUndoCommand<CStringTable>("Move String", mpStringTable, true);
        mpStringTable->MoveString(StringIndex, NewIndex);
        UndoStack().push(pCommand);

        // Select new string index
        pCommand = new CSetStringIndexCommand(this, StringIndex, NewIndex);
        UndoStack().push(pCommand);
        UndoStack().endMacro();
    }
}

void CStringEditor::IncrementStringIndex()
{
    uint NewIndex = mCurrentStringIndex + 1;

    if (NewIndex < mpStringTable->NumStrings())
    {
        IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, NewIndex);
        UndoStack().push(pCommand);
    }
}

void CStringEditor::DecrementStringIndex()
{
    uint NewIndex = mCurrentStringIndex - 1;

    if (NewIndex != -1)
    {
        IUndoCommand* pCommand = new CSetStringIndexCommand(this, mCurrentStringIndex, NewIndex);
        UndoStack().push(pCommand);
    }
}

void CStringEditor::IncrementLanguageIndex()
{
    for (uint i=0; i<mpStringTable->NumLanguages() - 1; i++)
    {
        if (mpStringTable->LanguageByIndex(i) == mCurrentLanguage)
        {
            ELanguage NewLanguage = mpStringTable->LanguageByIndex(i+1);
            IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, NewLanguage);
            UndoStack().push(pCommand);
            break;
        }
    }
}

void CStringEditor::DecrementLanguageIndex()
{
    for (uint i=mpStringTable->NumLanguages() - 1; i>0; i--)
    {
        if (mpStringTable->LanguageByIndex(i) == mCurrentLanguage)
        {
            ELanguage NewLanguage = mpStringTable->LanguageByIndex(i-1);
            IUndoCommand* pCommand = new CSetLanguageCommand(this, mCurrentLanguage, NewLanguage);
            UndoStack().push(pCommand);
            break;
        }
    }
}
