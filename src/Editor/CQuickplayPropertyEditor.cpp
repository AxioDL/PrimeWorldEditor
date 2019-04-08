#include "CQuickplayPropertyEditor.h"
#include "ui_CQuickplayPropertyEditor.h"
#include "UICommon.h"
#include "WorldEditor/CWorldEditor.h"
#include <Core/Resource/Script/CScriptLayer.h>
#include <QFileInfo>

/** Validator class for Dolphin line edit */
class CDolphinValidator : public QValidator
{
public:
    CDolphinValidator(QObject* pParent = 0) : QValidator(pParent) {}

    virtual QValidator::State validate(QString& Input, int& Pos) const override
    {
        return PathValid(Input) ? QValidator::Acceptable : QValidator::Invalid;
    }

    static bool PathValid(const QString& kPath)
    {
        QFileInfo FileInfo(kPath);
        return FileInfo.exists() && FileInfo.suffix() == "exe";
    }
};

/** CQuickplayPropertyEditor functions */
CQuickplayPropertyEditor::CQuickplayPropertyEditor(SQuickplayParameters& Parameters, QWidget* pParent /*= 0*/)
    : QMenu(pParent)
    , mpUI(new Ui::CQuickplayPropertyEditor)
    , mParameters(Parameters)
{
    mpUI->setupUi(this);
    setMinimumWidth(300);

    NDolphinIntegration::LoadQuickplayParameters(Parameters);
    mpUI->DolphinPathLineEdit->setText( NDolphinIntegration::GetDolphinPath() );
    mpUI->DolphinPathLineEdit->setValidator( new CDolphinValidator(this) );
    mpUI->BootToAreaCheckBox->setChecked( Parameters.Features.HasFlag(EQuickplayFeature::JumpToArea) );
    mpUI->SpawnAtCameraLocationCheckBox->setChecked( Parameters.Features.HasFlag(EQuickplayFeature::SetSpawnPosition) );
    mpUI->GiveAllItemsCheckBox->setChecked( Parameters.Features.HasFlag(EQuickplayFeature::GiveAllItems) );

    connect(mpUI->DolphinPathLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(OnDolphinPathChanged(QString)));

    connect(mpUI->DolphinBrowseButton, SIGNAL(pressed()),
            this, SLOT(BrowseForDolphin()));

    connect(mpUI->BootToAreaCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(OnBootToAreaToggled(bool)));

    connect(mpUI->SpawnAtCameraLocationCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(OnSpawnAtCameraLocationToggled(bool)));

    connect(mpUI->GiveAllItemsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(OnGiveAllItemsToggled(bool)));

    connect(mpUI->LayerList, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(OnLayerListItemChanged(QListWidgetItem*)));

    // Connect to World Editor signals
    CWorldEditor* pWorldEditor = qobject_cast<CWorldEditor*>(pParent);

    if (pWorldEditor)
    {
        connect(pWorldEditor, SIGNAL(MapChanged(CWorld*,CGameArea*)),
                this, SLOT(OnWorldEditorAreaChanged(CWorld*,CGameArea*)));
    }
}

CQuickplayPropertyEditor::~CQuickplayPropertyEditor()
{
    delete mpUI;
}

void CQuickplayPropertyEditor::BrowseForDolphin()
{
    QString Path = UICommon::OpenFileDialog(this, "Open Dolphin", "Dolphin.exe");

    if (!Path.isEmpty())
    {
        mpUI->DolphinPathLineEdit->setText(Path);
    }
}

void CQuickplayPropertyEditor::OnDolphinPathChanged(const QString& kNewPath)
{
    if (CDolphinValidator::PathValid(kNewPath))
    {
        NDolphinIntegration::SetDolphinPath(parentWidget(), kNewPath);
    }
}

void CQuickplayPropertyEditor::OnBootToAreaToggled(bool Enabled)
{
    if (Enabled)
    {
        mParameters.Features.SetFlag(EQuickplayFeature::JumpToArea);
    }
    else
    {
        mParameters.Features.ClearFlag(EQuickplayFeature::JumpToArea);
    }

    NDolphinIntegration::SaveQuickplayParameters(mParameters);
}

void CQuickplayPropertyEditor::OnSpawnAtCameraLocationToggled(bool Enabled)
{
    if (Enabled)
    {
        mParameters.Features.SetFlag(EQuickplayFeature::SetSpawnPosition);
    }
    else
    {
        mParameters.Features.ClearFlag(EQuickplayFeature::SetSpawnPosition);
    }

    NDolphinIntegration::SaveQuickplayParameters(mParameters);
}

void CQuickplayPropertyEditor::OnGiveAllItemsToggled(bool Enabled)
{
    if (Enabled)
    {
        mParameters.Features.SetFlag(EQuickplayFeature::GiveAllItems);
    }
    else
    {
        mParameters.Features.ClearFlag(EQuickplayFeature::GiveAllItems);
    }

    NDolphinIntegration::SaveQuickplayParameters(mParameters);
}

void CQuickplayPropertyEditor::OnLayerListItemChanged(QListWidgetItem* pItem)
{
    int LayerIdx = mpUI->LayerList->row(pItem);
    uint64 LayerBit = 1ULL << LayerIdx;
    mParameters.BootAreaLayerFlags &= ~LayerBit;

    if (pItem->checkState() == Qt::Checked)
    {
        mParameters.BootAreaLayerFlags |= LayerBit;
    }
}

void CQuickplayPropertyEditor::OnWorldEditorAreaChanged(CWorld* pWorld, CGameArea* pArea)
{
    mParameters.BootAreaLayerFlags = 0;
    mpUI->LayerList->blockSignals(true);
    mpUI->LayerList->clear();

    if (pArea)
    {
        for (uint LayerIdx = 0; LayerIdx < pArea->NumScriptLayers(); LayerIdx++)
        {
            CScriptLayer* pLayer = pArea->ScriptLayer(LayerIdx);
            bool bActive = pLayer->IsActive();

            QListWidgetItem* pItem = new QListWidgetItem();
            pItem->setText( TO_QSTRING(pLayer->Name()) );
            pItem->setCheckState( bActive ? Qt::Checked : Qt::Unchecked );
            mpUI->LayerList->addItem( pItem );

            if (bActive)
            {
                mParameters.BootAreaLayerFlags |= (1ULL << LayerIdx);
            }
        }
    }

    mpUI->LayerList->blockSignals(false);
}
