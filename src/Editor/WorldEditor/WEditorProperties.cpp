#include "WEditorProperties.h"
#include "Editor/Undo/CEditScriptPropertyCommand.h"
#include <Core/Resource/Script/CScriptLayer.h>

WEditorProperties::WEditorProperties(QWidget *pParent)
    : QWidget(pParent)
{
    mpInstanceInfoLabel = new QLabel;
    mpInstanceInfoLabel->setText(tr("<i>[No selection]</i>"));
    mpInstanceInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mpInstanceInfoLayout = new QHBoxLayout;
    mpInstanceInfoLayout->addWidget(mpInstanceInfoLabel);

    mpActiveCheckBox = new QCheckBox;
    mpActiveCheckBox->setToolTip(tr("Active"));
    mpInstanceNameLineEdit = new QLineEdit;
    mpInstanceNameLineEdit->setToolTip(tr("Instance Name"));
    mpNameLayout = new QHBoxLayout;
    mpNameLayout->addWidget(mpActiveCheckBox);
    mpNameLayout->addWidget(mpInstanceNameLineEdit);

    mpLayersLabel = new QLabel;
    mpLayersLabel->setText(tr("Layer:"));
    mpLayersLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mpLayersComboBox = new QComboBox;
    mpLayersComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mpLayersLayout = new QHBoxLayout;
    mpLayersLayout->addWidget(mpLayersLabel);
    mpLayersLayout->addWidget(mpLayersComboBox);

    mpMainLayout = new QVBoxLayout;
    mpMainLayout->addLayout(mpInstanceInfoLayout);
    mpMainLayout->addLayout(mpNameLayout);
    mpMainLayout->addLayout(mpLayersLayout);
    mpMainLayout->setContentsMargins(3, 3, 3, 0);
    mpMainLayout->setSpacing(3);
    setLayout(mpMainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QFont Font = font();
    Font.setPointSize(10);
    setFont(Font);
    mpInstanceInfoLabel->setFont(Font);
    mpInstanceNameLineEdit->setFont(Font);
    mpLayersLabel->setFont(Font);

    QFont ComboFont = mpLayersComboBox->font();
    ComboFont.setPointSize(10);
    mpLayersComboBox->setFont(ComboFont);

    connect(mpActiveCheckBox, &QCheckBox::clicked, this, &WEditorProperties::OnActiveChanged);
    connect(mpInstanceNameLineEdit, &QLineEdit::textEdited, this, &WEditorProperties::OnInstanceNameEdited);
    connect(mpInstanceNameLineEdit, &QLineEdit::editingFinished, this, &WEditorProperties::OnInstanceNameEditFinished);
    connect(mpLayersComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &WEditorProperties::OnLayerChanged);
}

void WEditorProperties::SyncToEditor(CWorldEditor *pEditor)
{
    if (mpEditor)
        disconnect(mpEditor, nullptr, this, nullptr);

    mpEditor = pEditor;
    connect(mpEditor, &CWorldEditor::SelectionModified, this, &WEditorProperties::OnSelectionModified);
    connect(mpEditor, &CWorldEditor::LayersModified, this, &WEditorProperties::OnLayersModified);
    connect(mpEditor, &CWorldEditor::InstancesLayerChanged, this, &WEditorProperties::OnInstancesLayerChanged);
    connect(mpEditor, &CWorldEditor::PropertyModified, this, &WEditorProperties::OnPropertyModified);

    OnLayersModified();
}

void WEditorProperties::SetLayerComboBox()
{
    mpLayersComboBox->blockSignals(true);

    if (mpDisplayNode && mpDisplayNode->NodeType() == ENodeType::Script)
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(mpDisplayNode);
        CScriptLayer *pLayer = pScript->Instance()->Layer();
        for (size_t iLyr = 0; iLyr < mpEditor->ActiveArea()->NumScriptLayers(); iLyr++)
        {
            if (mpEditor->ActiveArea()->ScriptLayer(iLyr) == pLayer)
            {
                mpLayersComboBox->setCurrentIndex(static_cast<int>(iLyr));
                break;
            }
        }
        mpLayersComboBox->setEnabled(true);
    }
    else
    {
        mpLayersComboBox->setCurrentIndex(-1);
        mpLayersComboBox->setEnabled(false);
    }

    mpLayersComboBox->blockSignals(false);
}

// ************ PUBLIC SLOTS ************
void WEditorProperties::OnSelectionModified()
{
    CNodeSelection *pSelection = mpEditor->Selection();
    mpDisplayNode = (pSelection->Size() == 1 ? pSelection->Front() : nullptr);

    if (pSelection->IsEmpty() || pSelection->Size() != 1 || mpDisplayNode->NodeType() != ENodeType::Script)
    {
        mpActiveCheckBox->setChecked(false);
        mpActiveCheckBox->setEnabled(false);
        mpInstanceNameLineEdit->setEnabled(false);

        if (pSelection->IsEmpty())
        {
            mpInstanceInfoLabel->setText(tr("<i>[No selection]</i>"));
            mpInstanceNameLineEdit->clear();
        }
        else if (mpDisplayNode)
        {
            mpInstanceInfoLabel->setText(tr("[Light]"));
            mpInstanceNameLineEdit->setText(TO_QSTRING(mpDisplayNode->Name()));
        }
        else
        {
            mpInstanceInfoLabel->setText(QString("<i>[%1 objects selected]</i>").arg(pSelection->Size()));
            mpInstanceNameLineEdit->clear();
        }

        mpInstanceInfoLabel->setToolTip({});
    }

    else
    {
        CScriptNode *pScript = static_cast<CScriptNode*>(mpDisplayNode);
        CInstanceID InstanceID = pScript->Instance()->InstanceID();
        TString ObjectType = pScript->Template()->Name();
        mpInstanceInfoLabel->setText(tr("[%1] [%2]")
                                         .arg(TO_QSTRING(ObjectType))
                                         .arg(TO_QSTRING(TString::HexString(InstanceID, 8, false))));
        mpInstanceInfoLabel->setToolTip(tr("[Layer: %1] [Area: %2] [ID: %3]")
                                            .arg(InstanceID.Layer())
                                            .arg(InstanceID.Area())
                                            .arg(TO_QSTRING(TString::HexString(InstanceID.Id(), 4, false))));

        UpdatePropertyValues();
    }

    SetLayerComboBox();
}

void WEditorProperties::OnPropertyModified(IProperty* pProp, CScriptObject* pInstance)
{
    if (!mpInstanceNameLineEdit->hasFocus())
    {
        if (mpDisplayNode->NodeType() == ENodeType::Script && pInstance->IsEditorProperty(pProp))
            UpdatePropertyValues();
    }
}

void WEditorProperties::OnInstancesLayerChanged(const QList<CScriptNode*>& rkNodeList)
{
    if (rkNodeList.contains((CScriptNode*) mpDisplayNode))
        SetLayerComboBox();
}

void WEditorProperties::OnLayersModified()
{
    CGameArea *pArea = mpEditor->ActiveArea();
    mpLayersComboBox->clear();

     if (pArea)
     {
         for (size_t iLyr = 0; iLyr < pArea->NumScriptLayers(); iLyr++)
             mpLayersComboBox->addItem(TO_QSTRING(pArea->ScriptLayer(iLyr)->Name()));
     }

     SetLayerComboBox();
}

void WEditorProperties::UpdatePropertyValues()
{
    CScriptNode *pScript = static_cast<CScriptNode*>(mpDisplayNode);
    CScriptObject *pInst = pScript->Instance();

    mpActiveCheckBox->setChecked(pInst->IsActive());
    mpActiveCheckBox->setEnabled(pInst->HasActive());

    mpInstanceNameLineEdit->blockSignals(true);
    mpInstanceNameLineEdit->setText(TO_QSTRING(pInst->InstanceName()));
    mpInstanceNameLineEdit->setEnabled(pInst->HasInstanceName());
    mpInstanceNameLineEdit->blockSignals(false);
}

// ************ PROTECTED SLOTS ************
void WEditorProperties::OnActiveChanged()
{
    mpEditor->SetSelectionActive(mpActiveCheckBox->isChecked());
}

void WEditorProperties::OnInstanceNameEdited()
{
    // This function triggers when the user is actively editing the Instance Name line edit.
    mHasEditedName = true;
    mpEditor->SetSelectionInstanceNames(mpInstanceNameLineEdit->text(), false);
}

void WEditorProperties::OnInstanceNameEditFinished()
{
    // This function triggers when the user is finished editing the Instance Name line edit.
    if (mHasEditedName)
        mpEditor->SetSelectionInstanceNames(mpInstanceNameLineEdit->text(), true);

    mHasEditedName = false;
}

void WEditorProperties::OnLayerChanged()
{
    const int Index = mpLayersComboBox->currentIndex();

    if (Index < 0)
        return;

    CScriptLayer *pLayer = mpEditor->ActiveArea()->ScriptLayer(static_cast<size_t>(Index));
    mpEditor->SetSelectionLayer(pLayer);
}

