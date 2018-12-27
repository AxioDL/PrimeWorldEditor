#include "CStringEditor.h"
#include "ui_CStringEditor.h"

#include "CStringDelegate.h"
#include "Editor/UICommon.h"

#include <QSettings>

/** Settings strings */
const char* gkpLanguageSetting = "StringEditor/EditLanguage";

/** Constructor */
CStringEditor::CStringEditor(CStringTable* pStringTable, QWidget* pParent)
    : IEditor(pParent)
    , mpUI(new Ui::CStringEditor)
    , mpStringTable(pStringTable)
    , mCurrentLanguage(ELanguage::English)
    , mCurrentStringIndex(0)
{
    mpListModel = new CStringListModel(pStringTable, this);
    mpUI->setupUi(this);

    InitUI();
    LoadSettings();
}

CStringEditor::~CStringEditor()
{
    delete mpUI;
}

void CStringEditor::InitUI()
{
    mpUI->StringNameListView->setModel(mpListModel);
    mpUI->StringNameListView->setItemDelegate( new CStringDelegate(this) );

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

    connect( mpUI->EditLanguageTabBar, SIGNAL(currentChanged(int)), this, SLOT(OnLanguageChanged(int)) );

    // Update window title
    QString WindowTitle = "%APP_FULL_NAME% - String Editor - %1[*]";
    WindowTitle = WindowTitle.arg( TO_QSTRING(mpStringTable->Entry()->CookedAssetPath(true).GetFileName()) );
    SET_WINDOWTITLE_APPVARS(WindowTitle);

    // Initialize the splitter so top split takes as much space as possible
    QList<int> SplitterSizes;
    SplitterSizes << (height() * 0.95) << (height() * 0.05);
    mpUI->splitter->setSizes(SplitterSizes);

    // Initialize status bar
    UpdateStatusBar();
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

        // Force UI to update with the correct string for the new language
        SetActiveString( mCurrentStringIndex );
        SaveSettings();
    }
}

void CStringEditor::SetActiveString(int StringIndex)
{
    mCurrentStringIndex = StringIndex;
    TString StringName = mpStringTable->StringNameByIndex(mCurrentStringIndex);
    TString StringData = mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex);
    mpUI->StringNameLineEdit->setText( TO_QSTRING(StringName) );
    mpUI->StringTextEdit->setPlainText( TO_QSTRING(StringData) );
    UpdateStatusBar();
}

void CStringEditor::LoadSettings()
{
    QSettings Settings;

    // Set language
    mCurrentLanguage = (ELanguage) Settings.value(gkpLanguageSetting, (int) ELanguage::English).toInt();

    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        if (mpStringTable->LanguageByIndex(LanguageIdx) == mCurrentLanguage)
        {
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
void CStringEditor::OnStringSelected(const QModelIndex& kIndex)
{
    SetActiveString( kIndex.row() );
}

void CStringEditor::OnLanguageChanged(int LanguageIndex)
{
    ASSERT( LanguageIndex >= 0 && LanguageIndex < (int) mpStringTable->NumLanguages() );
    ELanguage Language = mpStringTable->LanguageByIndex(LanguageIndex);
    SetActiveLanguage(Language);
}
