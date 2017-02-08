#include "CExportGameDialog.h"
#include "ui_CExportGameDialog.h"
#include "UICommon.h"

#include <Common/AssertMacro.h>
#include <Core/GameProject/CAssetNameMap.h>
#include <Core/GameProject/CGameExporter.h>
#include <Core/GameProject/CGameInfo.h>
#include <Core/Resource/Script/CMasterTemplate.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>

#include <nod/nod.hpp>

CExportGameDialog::CExportGameDialog(const QString& rkIsoPath, const QString& rkExportDir, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , mpUI(new Ui::CExportGameDialog)
    , mGame(eUnknownGame)
    , mRegion(eRegion_Unknown)
    , mTrilogy(false)
    , mExportSuccess(false)
{
    mpUI->setupUi(this);

    // Set up disc
    TWideString StrPath = TO_TWIDESTRING(rkIsoPath);
    mpDisc = nod::OpenDiscFromImage(*StrPath).release();

    if (ValidateGame())
    {
        mBuildVer = FindBuildVersion();
        InitUI(rkExportDir);

        TString IsoName = TO_TSTRING(rkIsoPath).GetFileName();
        setWindowTitle(QString("Export Settings - %1").arg( TO_QSTRING(IsoName) ));
    }
    else
    {
        if (!mTrilogy)
            UICommon::ErrorMsg(this, "Invalid ISO!");

        delete mpDisc;
        mpDisc = nullptr;
    }
}

CExportGameDialog::~CExportGameDialog()
{
    delete mpUI;
    delete mpDisc;
}

void RecursiveAddToTree(const nod::Node *pkNode, QTreeWidgetItem *pParent);

void CExportGameDialog::InitUI(QString ExportDir)
{
    ASSERT(mpDisc != nullptr);

    // Export settings
    ExportDir.replace('/', '\\');

    TWideString DefaultNameMapPath = CAssetNameMap::DefaultNameMapPath();
    if (!FileUtil::Exists(DefaultNameMapPath)) DefaultNameMapPath = L"";

    TWideString DefaultGameInfoPath = CGameInfo::GetDefaultGameInfoPath(mGame);
    if (!FileUtil::Exists(DefaultGameInfoPath)) DefaultGameInfoPath = L"";

    mpUI->OutputDirectoryLineEdit->setText(ExportDir);
    mpUI->AssetNameMapLineEdit->setText(TO_QSTRING(DefaultNameMapPath));
    mpUI->GameEditorInfoLineEdit->setText(TO_QSTRING(DefaultGameInfoPath));

    // Info boxes
    mpUI->GameTitleLineEdit->setText( TO_QSTRING(mGameTitle) );
    mpUI->GameIdLineEdit->setText( TO_QSTRING(mGameID) );
    mpUI->BuildVersionLineEdit->setText( QString::number(mBuildVer) );
    mpUI->RegionLineEdit->setText( mRegion == eRegion_NTSC ? "NTSC" :
                                   mRegion == eRegion_PAL ? "PAL" : "JPN" );

    // Disc tree widget
    nod::Partition *pPartition = mpDisc->getDataPartition();
    ASSERT(pPartition);

    const nod::Node *pkDiscRoot = &pPartition->getFSTRoot();
    if (mTrilogy)
        pkDiscRoot = &*pkDiscRoot->find( GetGameShortName(mGame).ToStdString() );

    QTreeWidgetItem *pTreeRoot = new QTreeWidgetItem((QTreeWidgetItem*) nullptr, QStringList(QString("Disc")));
    mpUI->DiscFstTreeWidget->addTopLevelItem(pTreeRoot);
    RecursiveAddToTree(pkDiscRoot, pTreeRoot);
    pTreeRoot->setIcon(0, QIcon(":/icons/Disc_16px.png"));
    pTreeRoot->setExpanded(true);

    // Signals and slots
    connect(mpUI->OutputDirectoryBrowseButton, SIGNAL(pressed()), this, SLOT(BrowseOutputDirectory()));
    connect(mpUI->AssetNameMapBrowseButton, SIGNAL(pressed()), this, SLOT(BrowseAssetNameMap()));
    connect(mpUI->GameEditorInfoBrowseButton, SIGNAL(pressed()), this, SLOT(BrowseGameEditorInfo()));
    connect(mpUI->CancelButton, SIGNAL(pressed()), this, SLOT(close()));
    connect(mpUI->ExportButton, SIGNAL(pressed()), this, SLOT(Export()));
}

bool CExportGameDialog::ValidateGame()
{
    if (!mpDisc) return false;

    const nod::Header& rkHeader = mpDisc->getHeader();
    mGameTitle = rkHeader.m_gameTitle;
    mGameID = TString(6, 0);
    memcpy(&mGameID[0], rkHeader.m_gameID, 6);

    // Check region byte
    switch (mGameID[3])
    {
    case 'E':
        mRegion = eRegion_NTSC;
        break;

    case 'P':
        mRegion = eRegion_PAL;
        break;

    case 'J':
        mRegion = eRegion_JPN;
        break;

    default:
        return false;
    }

    // Set region byte to X so we don't need to compare every regional variant of the ID
    // Then figure out what game this is
    CFourCC GameID(&mGameID[0]);
    GameID[3] = 'X';

    switch (GameID.ToLong())
    {
    case IFOURCC('GM8X'):
        // This ID is normally MP1, but it's used by the MP1 NTSC demo and the MP2 bonus disc demo as well
        if (strcmp(rkHeader.m_gameTitle, "Long Game Name") == 0)
        {
            // todo - not handling demos yet
            return false;
        }

        mGame = ePrime;
        break;

    case IFOURCC('G2MX'):
        // Echoes, but also appears in the MP3 proto
        if (mGameID[4] == 'A' && mGameID[5] == 'B')
            mGame = eCorruptionProto;
        else
            mGame = eEchoes;
        break;

    case IFOURCC('RM3X'):
        mGame = eCorruption;
        break;

    case IFOURCC('SF8X'):
        mGame = eReturns;
        break;

    case IFOURCC('R3MX'):
        // Trilogy
        mTrilogy = true;
        if (!RequestTrilogyGame()) return false;
        break;

    case IFOURCC('R3IX'):
        // MP1 Wii de Asobu
    case IFOURCC('R32X'):
        // MP2 Wii de Asobu
    default:
        // Unrecognized game ID
        return false;
    }

    return true;
}

bool CExportGameDialog::RequestTrilogyGame()
{
    QDialog Dialog;
    Dialog.setWindowTitle("Select Trilogy Game");

    QLabel Label("You have selected a Metroid Prime: Trilogy ISO. Please pick a game to export:", &Dialog);
    QComboBox ComboBox(&Dialog);
    ComboBox.addItem("Metroid Prime");
    ComboBox.addItem("Metroid Prime 2: Echoes");
    ComboBox.addItem("Metroid Prime 3: Corruption");
    QDialogButtonBox ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &Dialog);
    connect(&ButtonBox, SIGNAL(accepted()), &Dialog, SLOT(accept()));
    connect(&ButtonBox, SIGNAL(rejected()), &Dialog, SLOT(reject()));

    QVBoxLayout Layout;
    Layout.addWidget(&Label);
    Layout.addWidget(&ComboBox);
    Layout.addWidget(&ButtonBox);
    Dialog.setLayout(&Layout);

    int Result = Dialog.exec();

    if (Result == QDialog::Accepted)
    {
        switch (ComboBox.currentIndex())
        {
        case 0: mGame = ePrime;         break;
        case 1: mGame = eEchoes;        break;
        case 2: mGame = eCorruption;    break;
        }
        return true;
    }
    else return false;
}

float CExportGameDialog::FindBuildVersion()
{
    ASSERT(mpDisc != nullptr);

    // MP1 demo build doesn't have a build version
    if (mGame == ePrimeDemo) return 0.f;

    // Get DOL buffer
    std::unique_ptr<uint8_t[]> pDolData = mpDisc->getDataPartition()->getDOLBuf();
    u32 DolSize = (u32) mpDisc->getDataPartition()->getDOLSize();

    // Find build info string
    const char *pkSearchText = "!#$MetroidBuildInfo!#$";
    const int SearchTextSize = strlen(pkSearchText);

    for (u32 SearchIdx = 0; SearchIdx < DolSize - SearchTextSize + 1; SearchIdx++)
    {
        int Match = 0;

        while (pDolData[SearchIdx + Match] == pkSearchText[Match] && Match < SearchTextSize)
            Match++;

        if (Match == SearchTextSize)
        {
            // Found the build info string; extract version number
            TString BuildInfo = (char*) &pDolData[SearchIdx + SearchTextSize];
            int BuildVerStart = BuildInfo.IndexOfPhrase("Build v") + 7;
            ASSERT(BuildVerStart != 6);

            return BuildInfo.SubString(BuildVerStart, 5).ToFloat();
        }
    }

    Log::Error("Failed to find MetroidBuildInfo string. Build Version will be set to 0.");
    return 0.f;
}

void RecursiveAddToTree(const nod::Node *pkNode, QTreeWidgetItem *pParent)
{
    // Get sorted list of nodes
    std::list<const nod::Node*> NodeList;
    for (nod::Node::DirectoryIterator Iter = pkNode->begin(); Iter != pkNode->end(); ++Iter)
        NodeList.push_back(&*Iter);

    NodeList.sort([](const nod::Node *pkLeft, const nod::Node *pkRight) -> bool
    {
        if (pkLeft->getKind() != pkRight->getKind())
            return pkLeft->getKind() == nod::Node::Kind::Directory;
        else
            return TString(pkLeft->getName()).ToUpper() < TString(pkRight->getName()).ToUpper();
    });

    // Add nodes to tree
    static const QIcon skFileIcon = QIcon(":/icons/New_16px.png");
    static const QIcon skDirIcon = QIcon(":/icons/Open_16px.png");

    for (auto Iter = NodeList.begin(); Iter != NodeList.end(); Iter++)
    {
        const nod::Node *pkNode = *Iter;
        bool IsDir = pkNode->getKind() == nod::Node::Kind::Directory;

        QTreeWidgetItem *pItem = new QTreeWidgetItem(pParent, QStringList(QString::fromStdString(pkNode->getName())) );
        pItem->setIcon(0, QIcon(IsDir ? skDirIcon : skFileIcon));

        if (IsDir)
            RecursiveAddToTree(pkNode, pItem);
    }
}

void CExportGameDialog::BrowseOutputDirectory()
{
    QString NewOutputDir = UICommon::OpenDirDialog(this, "Choose export directory");
    if (!NewOutputDir.isEmpty()) mpUI->OutputDirectoryLineEdit->setText(NewOutputDir);
}

void CExportGameDialog::BrowseAssetNameMap()
{
    QString Filter = "*." + TO_QSTRING(CAssetNameMap::GetExtension());
    QString NewNameMap = UICommon::OpenFileDialog(this, "Choose Asset Name Map", Filter);
    if (!NewNameMap.isEmpty()) mpUI->AssetNameMapLineEdit->setText(NewNameMap);
}

void CExportGameDialog::BrowseGameEditorInfo()
{
    QString Filter = "*." + TO_QSTRING(CGameInfo::GetExtension());
    QString NewGameInfo = UICommon::OpenFileDialog(this, "Choose Game Editor Info", Filter);
    if (!NewGameInfo.isEmpty()) mpUI->GameEditorInfoLineEdit->setText(NewGameInfo);
}

void CExportGameDialog::Export()
{
    QString ExportDir = mpUI->OutputDirectoryLineEdit->text();
    QString NameMapPath = mpUI->AssetNameMapLineEdit->text();
    QString GameInfoPath = mpUI->GameEditorInfoLineEdit->text();

    // Validate export dir
    if (ExportDir.isEmpty())
    {
        UICommon::ErrorMsg(this, "Please specify an output directory!");
        return;
    }

    else if (!FileUtil::IsEmpty( TO_TSTRING(ExportDir) ))
    {
        QMessageBox::Button Button = QMessageBox::warning(this, "Warning", "<b>Warning:</b> The specified directory is not empty. Export anyway?", QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::NoButton);
        if (Button != QMessageBox::Ok) return;
    }

    // Verify name map is valid
    if (!NameMapPath.isEmpty() && !FileUtil::Exists(TO_TSTRING(NameMapPath)))
    {
        UICommon::ErrorMsg(this, "The Asset Name Map path is invalid!");
        return;
    }

    CAssetNameMap NameMap;

    if (!NameMapPath.isEmpty())
        NameMap.LoadAssetNames( TO_TSTRING(NameMapPath) );

    if (!NameMap.IsValid())
    {
        UICommon::ErrorMsg(this, "The Asset Name Map is invalid and cannot be used! See the log for more information.");
        return;
    }

    // Verify game info is valid
    if (!GameInfoPath.isEmpty() && !FileUtil::Exists(TO_TSTRING(GameInfoPath)))
    {
        UICommon::ErrorMsg(this, "The Game Editor Info path is invalid!");
        return;
    }

    // Do export
    close();

    CGameInfo GameInfo;
    if (!GameInfoPath.isEmpty())
        GameInfo.LoadGameInfo( TO_TSTRING(GameInfoPath) );

    CGameExporter Exporter(mGame, mRegion, mGameTitle, mGameID, mBuildVer);
    TString StrExportDir = TO_TSTRING(ExportDir);
    StrExportDir.EnsureEndsWith('\\');
    mExportSuccess = Exporter.Export(mpDisc, StrExportDir, &NameMap, &GameInfo);

    if (!mExportSuccess)
        UICommon::ErrorMsg(this, "Export failed!");
    else
        mNewProjectPath = TO_QSTRING(Exporter.ProjectPath());
}
