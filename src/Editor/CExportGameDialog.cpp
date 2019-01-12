#include "CExportGameDialog.h"
#include "ui_CExportGameDialog.h"
#include "CProgressDialog.h"
#include "UICommon.h"

#include <Common/Macros.h>
#include <Core/GameProject/CAssetNameMap.h>
#include <Core/GameProject/CGameExporter.h>
#include <Core/GameProject/CGameInfo.h>
#include <Core/Resource/Script/CGameTemplate.h>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

#include <nod/nod.hpp>

CExportGameDialog::CExportGameDialog(const QString& rkIsoPath, const QString& rkExportDir, QWidget *pParent /*= 0*/)
    : QDialog(pParent)
    , mpUI(new Ui::CExportGameDialog)
    , mpDisc(nullptr)
    , mpExporter(nullptr)
    , mDiscType(EDiscType::Normal)
    , mGame(EGame::Invalid)
    , mRegion(ERegion::Unknown)
    , mBuildVer(0.f)
    , mWiiFrontend(false)
    , mExportSuccess(false)
{
    mpUI->setupUi(this);

    // Set up disc
    mpDisc = nod::OpenDiscFromImage(TO_WCHAR(rkIsoPath)).release();

    if (ValidateGame())
    {
        mBuildVer = FindBuildVersion();
        mpExporter = new CGameExporter(mDiscType, mGame, mWiiFrontend, mRegion, mGameTitle, mGameID, mBuildVer);
        InitUI(rkExportDir);

        TString IsoName = TO_TSTRING(rkIsoPath).GetFileName();
        setWindowTitle(QString("Export Settings - %1").arg( TO_QSTRING(IsoName) ));
    }
    else
    {
        delete mpDisc;
        mpDisc = nullptr;
    }
}

CExportGameDialog::~CExportGameDialog()
{
    delete mpUI;
    delete mpDisc;
    delete mpExporter;
}

void RecursiveAddToTree(const nod::Node *pkNode, QTreeWidgetItem *pParent);

void CExportGameDialog::InitUI(QString ExportDir)
{
    ASSERT(mpDisc != nullptr);

    // Export settings
    CGameInfo GameInfo;
    GameInfo.LoadGameInfo(mGame);

    ExportDir.replace('\\', '/');

    TString DefaultNameMapPath = CAssetNameMap::DefaultNameMapPath(mGame);
    if (!FileUtil::Exists(DefaultNameMapPath)) DefaultNameMapPath = "";

    TString DefaultGameInfoPath = CGameInfo::GetDefaultGameInfoPath(mGame);
    if (!FileUtil::Exists(DefaultGameInfoPath)) DefaultGameInfoPath = "";

    mpUI->OutputDirectoryLineEdit->setText(ExportDir);
    mpUI->AssetNameMapLineEdit->setText(TO_QSTRING(DefaultNameMapPath));
    mpUI->GameEditorInfoLineEdit->setText(TO_QSTRING(DefaultGameInfoPath));

    // Info boxes
    mpUI->GameTitleLineEdit->setText( TO_QSTRING(mGameTitle) );
    mpUI->GameIdLineEdit->setText( TO_QSTRING(mGameID) );
    mpUI->BuildVersionLineEdit->setText( QString("%1 (%2)").arg(mBuildVer).arg( TO_QSTRING(GameInfo.GetBuildName(mBuildVer, mRegion)) ));
    mpUI->RegionLineEdit->setText( TEnumReflection<ERegion>::ConvertValueToString(mRegion) );

    // Disc tree widget
    nod::IPartition *pPartition = mpDisc->getDataPartition();
    ASSERT(pPartition);

    QTreeWidgetItem *pTreeRoot = new QTreeWidgetItem((QTreeWidgetItem*) nullptr, QStringList(QString("Disc")));
    mpUI->DiscFstTreeWidget->addTopLevelItem(pTreeRoot);

    const nod::Node *pkDiscRoot = &pPartition->getFSTRoot();
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

    // The MP2 ISO doesn't have a colon in the game name and it kinda annoys me
    if (mGameTitle == "Metroid Prime 2 Echoes")
        mGameTitle = "Metroid Prime 2: Echoes";

    // Check region byte
    switch (mGameID[3])
    {
    case 'E':
        mRegion = ERegion::NTSC;
        break;

    case 'P':
        mRegion = ERegion::PAL;
        break;

    case 'J':
        mRegion = ERegion::JPN;
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
    case FOURCC('GM8X'):
        // This ID is normally MP1, but it's used by the MP1 NTSC demo and the MP2 bonus disc demo as well
        if (strcmp(rkHeader.m_gameTitle, "Long Game Name") == 0)
        {
            // Calculate the CRC of the apploader to figure out which game this is.
            std::unique_ptr<uint8_t[]> pApploaderData = mpDisc->getDataPartition()->getApploaderBuf();
            uint ApploaderSize = (uint) mpDisc->getDataPartition()->getApploaderSize();
            uint ApploaderHash = CCRC32::StaticHashData(pApploaderData.get(), ApploaderSize);

            if (ApploaderHash == 0x21B7AFF5)
            {
                // This is the hash for the NTSC MP1 demo.
                mGame = EGame::PrimeDemo;
            }
            else
            {
                // Hash is different, so this is most likely an Echoes demo build
                mGame = EGame::EchoesDemo;
            }

            break;
        }
        else
        {
            // This could be either Metroid Prime, or the PAL demo of it...
            // In either case, the PAL demo is based on a later build of the game than the NTSC demo
            // So the PAL demo should be configured the same way as the release build of the game anyway
            mGame = EGame::Prime;
            break;
        }

    case FOURCC('G2MX'):
        // Echoes, but also appears in the MP3 proto
        if (mGameID[4] == 'A' && mGameID[5] == 'B')
            mGame = EGame::CorruptionProto;
        else
            mGame = EGame::Echoes;
        break;

    case FOURCC('RM3X'):
        mGame = EGame::Corruption;
        break;

    case FOURCC('SF8X'):
        mGame = EGame::DKCReturns;
        break;

    case FOURCC('R3MX'):
        // Trilogy
        mDiscType = EDiscType::Trilogy;
        if (!RequestWiiPortGame()) return false;

        // Force change game name so it isn't "Metroid Prime Trilogy"
        if (!mWiiFrontend)
            mGameTitle = GetGameName(mGame);

        break;

    case FOURCC('R3IX'):
        // MP1 Wii de Asobu
        mGame = EGame::Prime;
        mDiscType = EDiscType::WiiDeAsobu;
        if (!RequestWiiPortGame()) return false;
        break;

    case FOURCC('R32X'):
        mGame = EGame::Echoes;
        mDiscType = EDiscType::WiiDeAsobu;
        if (!RequestWiiPortGame()) return false;
        break;

    default:
        // Unrecognized game ID
        return false;
    }

    // The demo builds are not supported. The MP1 demo does not have script templates currently.
    // Additionally, a lot of file format loaders currently don't support the demo variants of the
    // file formats, meaning that attempting to export results in crashes.
    if (mGame == EGame::PrimeDemo || mGame == EGame::EchoesDemo || mGame == EGame::CorruptionProto)
    {
        // we cannot parent the error message box to ourselves because this window hasn't been shown
        UICommon::ErrorMsg(parentWidget(), "The demo builds are currently not supported.");
        return false;
    }

    return true;
}

bool CExportGameDialog::RequestWiiPortGame()
{
    QDialog Dialog;
    Dialog.setWindowTitle("Select Game");

    bool IsTrilogy = (mGame == EGame::Invalid);
    bool HasMP1 = (IsTrilogy || mGame == EGame::Prime);
    bool HasMP2 = (IsTrilogy || mGame == EGame::Echoes);
    bool HasMP3 = IsTrilogy;

    QString GameName = (IsTrilogy ? "Metroid Prime: Trilogy" : "Wii de Asobu");
    QString LabelText = QString("You have selected a %1 ISO. Please pick a game to export:").arg(GameName);
    QLabel Label(LabelText, &Dialog);

    QComboBox ComboBox(&Dialog);
    ComboBox.addItem("Front End");
    if (HasMP1) ComboBox.addItem("Metroid Prime");
    if (HasMP2) ComboBox.addItem("Metroid Prime 2: Echoes");
    if (HasMP3) ComboBox.addItem("Metroid Prime 3: Corruption");
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
        case 0:
            mGame = EGame::Corruption;
            mWiiFrontend = true;
            break;

        case 1:
            mGame = (HasMP1 ? EGame::Prime : EGame::Echoes);
            break;

        case 2:
            mGame = EGame::Echoes;
            break;

        case 3:
            mGame = EGame::Corruption;
            break;
        }

        return true;
    }
    else return false;
}

float CExportGameDialog::FindBuildVersion()
{
    ASSERT(mpDisc != nullptr);

    // MP1 demo build doesn't have a build version
    if (mGame == EGame::PrimeDemo) return 0.f;

    // Get DOL buffer
    std::unique_ptr<uint8_t[]> pDolData = mpDisc->getDataPartition()->getDOLBuf();
    uint32 DolSize = (uint32) mpDisc->getDataPartition()->getDOLSize();

    // Find build info string
    const char *pkSearchText = "!#$MetroidBuildInfo!#$";
    const int SearchTextSize = strlen(pkSearchText);

    for (uint32 SearchIdx = 0; SearchIdx < DolSize - SearchTextSize + 1; SearchIdx++)
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

    errorf("Failed to find MetroidBuildInfo string. Build Version will be set to 0.");
    return 0.f;
}

void CExportGameDialog::RecursiveAddToTree(const nod::Node *pkNode, QTreeWidgetItem *pParent)
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
            return TString(pkLeft->getName().data()).ToUpper() < TString(pkRight->getName().data()).ToUpper();
    });

    // Add nodes to tree
    static const QIcon skFileIcon = QIcon(":/icons/New_16px.png");
    static const QIcon skDirIcon = QIcon(":/icons/Open_16px.png");

    for (auto Iter = NodeList.begin(); Iter != NodeList.end(); Iter++)
    {
        const nod::Node *pkNode = *Iter;

        if (!mpExporter->ShouldExportDiscNode(pkNode, pParent->parent() == nullptr))
            continue;

        bool IsDir = pkNode->getKind() == nod::Node::Kind::Directory;

        QTreeWidgetItem *pItem = new QTreeWidgetItem(pParent, QStringList(QString::fromStdString(pkNode->getName().data())) );
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
        UICommon::ErrorMsg(this, "Please specify an empty output directory!");
        return;
    }

    else if (!FileUtil::IsEmpty( TO_TSTRING(ExportDir) ))
    {
        UICommon::ErrorMsg(this, "The output directory is not empty!");
        return;
    }

    // Verify name map is valid
    if (!NameMapPath.isEmpty() && !FileUtil::Exists(TO_TSTRING(NameMapPath)))
    {
        UICommon::ErrorMsg(this, "The Asset Name Map path is invalid!");
        return;
    }

    CAssetNameMap NameMap(mGame);

    if (!NameMapPath.isEmpty())
    {
        bool LoadSuccess = NameMap.LoadAssetNames( TO_TSTRING(NameMapPath) );

        if (!LoadSuccess)
        {
            UICommon::ErrorMsg(this, "Failed to load the asset name map!");
            return;
        }

        else if (!NameMap.IsValid())
        {
            UICommon::ErrorMsg(this, "The Asset Name Map is invalid and cannot be used! See the log for more information.");
            return;
        }
    }

    // Verify game info is valid
    if (!GameInfoPath.isEmpty() && !FileUtil::Exists(TO_TSTRING(GameInfoPath)))
    {
        UICommon::ErrorMsg(this, "The Game Editor Info path is invalid!");
        return;
    }

    CGameInfo GameInfo;
    if (!GameInfoPath.isEmpty())
    {
        bool LoadSuccess = GameInfo.LoadGameInfo( TO_TSTRING(GameInfoPath) );

        if (!LoadSuccess)
        {
            UICommon::ErrorMsg(this, "Failed to load game info!");
            return;
        }
    }

    // Do export
    close();

    TString StrExportDir = TO_TSTRING(ExportDir);
    StrExportDir.EnsureEndsWith('/');

    CProgressDialog Dialog("Creating new game project", false, true, parentWidget());
    QFuture<bool> Future = QtConcurrent::run(mpExporter, &CGameExporter::Export, mpDisc, StrExportDir, &NameMap, &GameInfo, &Dialog);
    mExportSuccess = Dialog.WaitForResults(Future);

    if (!mExportSuccess)
    {
        if (!Dialog.ShouldCancel())
            UICommon::ErrorMsg(this, "Export failed!");
    }
    else
        mNewProjectPath = TO_QSTRING(mpExporter->ProjectPath());
}
