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
    explicit CDolphinValidator(QObject* pParent = nullptr) : QValidator(pParent) {}

    QValidator::State validate(QString& Input, int& Pos) const override
    {
        return PathValid(Input) ? QValidator::Acceptable : QValidator::Invalid;
    }

    static bool PathValid(const QString& kPath)
    {
        QFileInfo FileInfo(kPath);
        return FileInfo.exists() && FileInfo.isExecutable();
    }
};

/** CQuickplayPropertyEditor functions */
CQuickplayPropertyEditor::CQuickplayPropertyEditor(SQuickplayParameters& Parameters, QWidget* pParent /*= 0*/)
    : QMenu(pParent)
    , mpUI(std::make_unique<Ui::CQuickplayPropertyEditor>())
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

    connect(mpUI->DolphinPathLineEdit, &QLineEdit::textChanged,
            this, &CQuickplayPropertyEditor::OnDolphinPathChanged);

    connect(mpUI->DolphinBrowseButton, &QPushButton::pressed,
            this, &CQuickplayPropertyEditor::BrowseForDolphin);

    connect(mpUI->BootToAreaCheckBox, &QCheckBox::toggled,
            this, &CQuickplayPropertyEditor::OnBootToAreaToggled);

    connect(mpUI->SpawnAtCameraLocationCheckBox, &QCheckBox::toggled,
            this, &CQuickplayPropertyEditor::OnSpawnAtCameraLocationToggled);

    connect(mpUI->GiveAllItemsCheckBox, &QCheckBox::toggled,
            this, &CQuickplayPropertyEditor::OnGiveAllItemsToggled);

    connect(mpUI->LayerList, &QListWidget::itemChanged,
            this, &CQuickplayPropertyEditor::OnLayerListItemChanged);

    // Connect to World Editor signals
    CWorldEditor* pWorldEditor = qobject_cast<CWorldEditor*>(pParent);

    if (pWorldEditor)
    {
        connect(pWorldEditor, &CWorldEditor::MapChanged,
                this, &CQuickplayPropertyEditor::OnWorldEditorAreaChanged);
    }
}

CQuickplayPropertyEditor::~CQuickplayPropertyEditor() = default;

void CQuickplayPropertyEditor::BrowseForDolphin()
{
    QString Path = NDolphinIntegration::AskForDolphinPath(this);

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
        for (size_t LayerIdx = 0; LayerIdx < pArea->NumScriptLayers(); LayerIdx++)
        {
            CScriptLayer* pLayer = pArea->ScriptLayer(LayerIdx);
            bool bActive = pLayer->IsActive();

            QListWidgetItem* pItem = new QListWidgetItem();
            pItem->setText(TO_QSTRING(pLayer->Name()));
            pItem->setCheckState(bActive ? Qt::Checked : Qt::Unchecked);
            mpUI->LayerList->addItem(pItem);

            if (bActive)
            {
                mParameters.BootAreaLayerFlags |= (1ULL << LayerIdx);
            }
        }
    }

    mpUI->LayerList->blockSignals(false);
}
