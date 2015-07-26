#include "WResourceSelector.h"

#include "UICommon.h"
#include "WTexturePreviewPanel.h"
#include <Core/CResCache.h>

#include <QApplication>
#include <QCompleter>
#include <QDesktopWidget>
#include <QDirModel>
#include <QEvent>
#include <QFileDialog>

WResourceSelector::WResourceSelector(QWidget *parent) : QWidget(parent)
{
    // Initialize Members
    mHasMultipleExtensions = false;
    mShowEditButton = false;
    mShowExportButton = false;

    mpPreviewPanel = nullptr;
    mEnablePreviewPanel = true;
    mPreviewPanelValid = false;
    mShowingPreviewPanel = false;
    mAdjustPreviewToParent = false;

    mpResource = nullptr;
    mResType = eInvalidResType;

    // Create Widgets
    mUI.LineEdit = new QLineEdit(this);
    mUI.BrowseButton = new QPushButton(this);
    mUI.EditButton = new QPushButton("Edit", this);
    mUI.ExportButton = new QPushButton("Export", this);

    // Create Layout
    mUI.Layout = new QHBoxLayout(this);
    setLayout(mUI.Layout);
    mUI.Layout->addWidget(mUI.LineEdit);
    mUI.Layout->addWidget(mUI.BrowseButton);
    mUI.Layout->addWidget(mUI.EditButton);
    mUI.Layout->addWidget(mUI.ExportButton);
    mUI.Layout->setContentsMargins(0,0,0,0);
    mUI.Layout->setSpacing(1);

    // Set Up Widgets
    mUI.LineEdit->installEventFilter(this);
    mUI.LineEdit->setMouseTracking(true);
    mUI.LineEdit->setMaximumHeight(23);
    mUI.LineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mUI.BrowseButton->installEventFilter(this);
    mUI.BrowseButton->setMouseTracking(true);
    mUI.BrowseButton->setText("...");
    mUI.BrowseButton->setMaximumSize(25, 23);
    mUI.BrowseButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mUI.EditButton->installEventFilter(this);
    mUI.EditButton->setMouseTracking(true);
    mUI.EditButton->setMaximumSize(50, 23);
    mUI.EditButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mUI.EditButton->hide();
    mUI.ExportButton->installEventFilter(this);
    mUI.ExportButton->setMouseTracking(true);
    mUI.ExportButton->setMaximumSize(50, 23);
    mUI.ExportButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mUI.ExportButton->hide();

    QCompleter *pCompleter = new QCompleter(this);
    pCompleter->setModel(new QDirModel(pCompleter));
    mUI.LineEdit->setCompleter(pCompleter);

    connect(mUI.LineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditTextEdited()));
    connect(mUI.BrowseButton, SIGNAL(clicked()), this, SLOT(OnBrowseButtonClicked()));
}

WResourceSelector::~WResourceSelector()
{
    delete mpPreviewPanel;
}

bool WResourceSelector::event(QEvent *pEvent)
{
    if ((pEvent->type() == QEvent::Leave) || (pEvent->type() == QEvent::WindowDeactivate))
        HidePreviewPanel();

    return false;
}

bool WResourceSelector::eventFilter(QObject *pObj, QEvent *pEvent)
{
    if (pEvent->type() == QEvent::MouseMove)
        if (mEnablePreviewPanel)
            ShowPreviewPanel();

    return false;
}

// ************ GETTERS ************
EResType WResourceSelector::GetResType()
{
    return mResType;
}

QString WResourceSelector::GetText()
{
    return mUI.LineEdit->text();
}

bool WResourceSelector::IsEditButtonEnabled()
{
    return mShowEditButton;
}

bool WResourceSelector::IsExportButtonEnabled()
{
    return mShowExportButton;
}

bool WResourceSelector::IsPreviewPanelEnabled()
{
    return mEnablePreviewPanel;
}


// ************ SETTERS ************
void WResourceSelector::SetResource(CResource *pRes)
{
    mpResource = pRes;
    mResToken = CToken(pRes);

    if (pRes)
    {
        CFourCC ext = StringUtil::GetExtension(pRes->Source());
        mResType = CResource::ResTypeForExtension(ext);
        mUI.LineEdit->setText(QString::fromStdString(pRes->Source()));
    }

    else
    {
        mResType = eInvalidResType;
        mUI.LineEdit->clear();
    }

    CreatePreviewPanel();
    SetButtonsBasedOnResType();
}

void WResourceSelector::SetResType(EResType Type)
{
    mResType = Type;
    mpResource = nullptr;
    mResToken.Unlock();
    mUI.LineEdit->clear();
    CreatePreviewPanel();
    SetButtonsBasedOnResType();
}

void WResourceSelector::SetResTypes(const CStringList &ExtensionList)
{
}

void WResourceSelector::SetText(const QString& ResPath)
{
    mUI.LineEdit->setText(ResPath);
    LoadResource(ResPath);
}

void WResourceSelector::SetEditButtonEnabled(bool Enabled)
{
    mShowEditButton = Enabled;
    if (Enabled) mUI.EditButton->show();
    else mUI.EditButton->hide();
}

void WResourceSelector::SetExportButtonEnabled(bool Enabled)
{
    mShowExportButton = Enabled;
    if (Enabled) mUI.ExportButton->show();
    else mUI.ExportButton->hide();
}

void WResourceSelector::SetPreviewPanelEnabled(bool Enabled)
{
    mEnablePreviewPanel = Enabled;
    if (!mPreviewPanelValid) CreatePreviewPanel();
}

void WResourceSelector::AdjustPreviewToParent(bool adjust)
{
    mAdjustPreviewToParent = adjust;
}

// ************ SLOTS ************
void WResourceSelector::OnLineEditTextEdited()
{
    LoadResource(mUI.LineEdit->text());
}

void WResourceSelector::OnBrowseButtonClicked()
{
    QString Filter = gskResourceFilters[mResType];

    std::string ResTypeStr = Filter.toStdString();
    size_t EndName = ResTypeStr.find_last_of("(") - 1;
    ResTypeStr = ResTypeStr.substr(0, EndName);
    QString ResType = QString::fromStdString(ResTypeStr);

    QString NewRes = QFileDialog::getOpenFileName(this, "Select " + ResType, "", Filter);

    if (!NewRes.isEmpty())
    {
        mUI.LineEdit->setText(NewRes);
        LoadResource(NewRes);
    }
}

void WResourceSelector::OnEditButtonClicked()
{
    Edit();
}

void WResourceSelector::OnExportButtonClicked()
{
    Export();
}

// ************ PRIVATE ************
// Should the resource selector handle edit/export itself
// or delegate it entirely to the signals?
void WResourceSelector::Edit()
{
    emit EditResource(mpResource);
}

void WResourceSelector::Export()
{
    emit ExportResource(mpResource);
}

void WResourceSelector::CreatePreviewPanel()
{
    delete mpPreviewPanel;
    mpPreviewPanel = IPreviewPanel::CreatePanel(mResType, this);

    if (!mpPreviewPanel) mPreviewPanelValid = false;

    else
    {
        mPreviewPanelValid = true;
        mpPreviewPanel->setWindowFlags(Qt::ToolTip);
        if (mpResource) mpPreviewPanel->SetResource(mpResource);
    }
}

void WResourceSelector::ShowPreviewPanel()
{
    if ((mPreviewPanelValid) && (mpResource != nullptr))
    {
        // Preferred panel point is lower-right, but can move if there's not enough room
        QPoint Position = parentWidget()->mapToGlobal(pos());
        QRect ScreenResolution = QApplication::desktop()->screenGeometry();
        QSize PanelSize = mpPreviewPanel->size();
        QPoint PanelPoint = Position;

        // Calculate parent adjustment with 9 pixels of buffer
        int ParentAdjustLeft = (mAdjustPreviewToParent ? pos().x() + 9 : 0);
        int ParentAdjustRight = (mAdjustPreviewToParent ? parentWidget()->width() - pos().x() + 9 : 0);

        // Is there enough space on the right?
        if (Position.x() + width() + PanelSize.width() + ParentAdjustRight >= ScreenResolution.width())
            PanelPoint.rx() -= PanelSize.width() + ParentAdjustLeft;
        else
            PanelPoint.rx() += width() + ParentAdjustRight;

        // Is there enough space on the bottom?
        if (Position.y() + PanelSize.height() >= ScreenResolution.height() - 30)
        {
            int Difference = Position.y() + PanelSize.height() - ScreenResolution.height() + 30;
            PanelPoint.ry() -= Difference;
        }

        mpPreviewPanel->move(PanelPoint);
        mpPreviewPanel->show();
        mShowingPreviewPanel = true;
    }
}

void WResourceSelector::HidePreviewPanel()
{
    if (mPreviewPanelValid && mShowingPreviewPanel)
    {
        mpPreviewPanel->hide();
        mShowingPreviewPanel = false;
    }
}

void WResourceSelector::LoadResource(const QString& ResPath)
{
    mpResource = nullptr;
    mResToken.Unlock();

    if ((mResType != eArea) && (mResType != eWorld))
    {
        std::string PathStr = ResPath.toStdString();
        CFourCC ResExt = StringUtil::GetExtension(PathStr).c_str();

        if (CResource::ResTypeForExtension(ResExt) == mResType)
        {
            mpResource = gResCache.GetResource(PathStr);
            mResToken = CToken(mpResource);

            if (mPreviewPanelValid) mpPreviewPanel->SetResource(mpResource);
        }
    }

    emit ResourceChanged(ResPath);
}

void WResourceSelector::SetButtonsBasedOnResType()
{
    // Basically this function sets whether the "Export" and "Edit"
    // buttons are present based on the resource type.
    switch (mResType)
    {
    // Export button should be enabled here because CTexture already has a DDS export function
    // However, need to figure out what sort of interface to create to do it. Disabling until then.
    case eTexture:
        SetEditButtonEnabled(false);
        SetExportButtonEnabled(false);
        break;
    default:
        SetEditButtonEnabled(false);
        SetExportButtonEnabled(false);
        break;
    }
}
