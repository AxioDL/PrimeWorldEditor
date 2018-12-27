#include "CStringEditor.h"
#include "ui_CStringEditor.h"

#include "CStringDelegate.h"
#include "Editor/UICommon.h"
#include "Editor/Widgets/CSizeableTabBar.h"

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
    UpdateUI();
}

CStringEditor::~CStringEditor()
{
    delete mpUI;
}

void CStringEditor::InitUI()
{
    mpUI->StringNameListView->setModel(mpListModel);
    mpUI->StringNameListView->setItemDelegate( new CStringDelegate(this) );

#if 0
    // Set up language combo box
    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        ELanguage Language = mpStringTable->LanguageByIndex(LanguageIdx);
        const char* pkLanguageName = TEnumReflection<ELanguage>::ConvertValueToString(Language);
        mpUI->LanguageComboBox->addItem(pkLanguageName);
    }
#else
    QTabBar* pTabBar = new QTabBar(this);
    pTabBar->setExpanding(false);

    // Set up language combo box
    for (uint LanguageIdx = 0; LanguageIdx < mpStringTable->NumLanguages(); LanguageIdx++)
    {
        ELanguage Language = mpStringTable->LanguageByIndex(LanguageIdx);
        const char* pkLanguageName = TEnumReflection<ELanguage>::ConvertValueToString(Language);
        pTabBar->addTab(pkLanguageName);
    }

    QVBoxLayout* pTabLayout = new QVBoxLayout(mpUI->TabsContainerWidget);
    pTabLayout->setContentsMargins(0,0,0,0);
    pTabLayout->addWidget(pTabBar);

#endif

    // Connect signals & slots
    connect( mpUI->StringNameListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(OnStringSelected(QModelIndex)) );

//    connect( mpUI->LanguageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnLanguageChanged(int)) );
    connect( pTabBar, SIGNAL(currentChanged(int)), this, SLOT(OnLanguageChanged(int)) );

    // Update window title
    QString WindowTitle = "%APP_FULL_NAME% - String Editor - %1[*]";
    WindowTitle = WindowTitle.arg( TO_QSTRING(mpStringTable->Entry()->CookedAssetPath(true).GetFileName()) );
    SET_WINDOWTITLE_APPVARS(WindowTitle);

    // Update status bar
    QString StatusText = QString("%1 languages, %2 strings")
            .arg(mpStringTable->NumLanguages())
            .arg(mpStringTable->NumStrings());

    mpUI->StatusBar->setStatusTip(StatusText);
}

void CStringEditor::UpdateUI()
{
}

void CStringEditor::SetActiveLanguage(ELanguage Language)
{
    if (mCurrentLanguage != Language)
    {
        mCurrentLanguage = Language;

        // Force UI to update with the correct string for the new language
        SetActiveString( mCurrentStringIndex );
    }
}

void CStringEditor::SetActiveString(int StringIndex)
{
    mCurrentStringIndex = StringIndex;
    TString StringName = mpStringTable->StringNameByIndex(mCurrentStringIndex);
    TString StringData = mpStringTable->GetString(mCurrentLanguage, mCurrentStringIndex);
    mpUI->StringNameLineEdit->setText( TO_QSTRING(StringName) );
    mpUI->StringTextEdit->setPlainText( TO_QSTRING(StringData) );
}

void CStringEditor::LoadSettings()
{
    QSettings Settings;
    mCurrentLanguage = (ELanguage) Settings.value(gkpLanguageSetting, (int) ELanguage::English).toInt();
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
