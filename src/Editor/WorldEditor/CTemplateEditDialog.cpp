#include "CTemplateEditDialog.h"
#include "ui_CTemplateEditDialog.h"

#include "Editor/UICommon.h"
#include <Core/Resource/Factory/CTemplateLoader.h>
#include <Core/Resource/Script/CGameTemplate.h>
#include <Core/Resource/Script/NGameList.h>
#include <Core/Resource/Script/NPropertyMap.h>

CTemplateEditDialog::CTemplateEditDialog(IProperty *pProperty, QWidget *pParent)
    : QDialog(pParent)
    , mpUI(new Ui::CTemplateEditDialog)
    , mpValidator(new CPropertyNameValidator(this))
    , mpProperty(pProperty)
    , mGame(pProperty->Game())
    , mOriginalName(pProperty->Name())
    , mOriginalDescription(pProperty->Description())
    , mOriginalAllowTypeNameOverride(false)
    , mOriginalNameWasValid(true)
{
    mpUI->setupUi(this);

    mpUI->IDDisplayLabel->setText(TO_QSTRING(pProperty->IDString(false)));
    mpUI->PathDisplayLabel->setText(TO_QSTRING(pProperty->IDString(true)));
    mpUI->NameLineEdit->setText(TO_QSTRING(pProperty->Name()));
    mpUI->DescriptionTextEdit->setPlainText(TO_QSTRING(pProperty->Description()));

    EPropertyType Type = pProperty->Type();

    // Configure type name
    if (Type == EPropertyType::Struct || Type == EPropertyType::Choice || Type == EPropertyType::Enum || Type == EPropertyType::Flags)
    {
        connect( mpUI->TypenameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(RefreshTypeNameOverride()) );
        mOriginalTypeName = pProperty->RootArchetype()->Name();
        mpUI->TypenameLineEdit->setText( TO_QSTRING(mOriginalTypeName) );
    }
    else
    {
        mpUI->TypenameLabel->setHidden(true);
        mpUI->TypenameLineEdit->setHidden(true);
    }

    // Configure type name override option
    if (Type == EPropertyType::Enum || Type == EPropertyType::Choice)
    {
        CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProperty);
        mOriginalAllowTypeNameOverride = pEnum->OverridesTypeName();
        mpUI->OverrideTypeNameCheckBox->setChecked( mOriginalAllowTypeNameOverride );
        connect( mpUI->OverrideTypeNameCheckBox, SIGNAL(toggled(bool)), this, SLOT(RefreshTypeNameOverride()) );
    }
    else
    {
        mpUI->OverrideTypeNameCheckBox->setHidden(true);
        mpUI->OverrideTypeNameCheckBox->setChecked(true);
    }
    RefreshTypeNameOverride();

    // Hide templates list for MP1
    if (mGame <= EGame::Prime)
    {
        mpUI->TemplatesGroupBox->hide();
        mpUI->RenameAllCheckBox->setText("Rename all copies of this property");
        mpUI->ValidityLabel->hide();
        resize(width(), minimumHeight());
    }

    else
    {
        NGameList::LoadAllGameTemplates();

        std::set<TString> Templates;
        NPropertyMap::RetrieveXMLsWithProperty( pProperty->ID(), pProperty->HashableTypeName(), Templates );

        for (auto Iter = Templates.begin(); Iter != Templates.end(); Iter++)
            mpUI->TemplatesListWidget->addItem(TO_QSTRING(*Iter));

        mpUI->ValidityLabel->SetValidityText("Hash match! Property name is likely correct.", "Hash mismatch! Property name is likely wrong.");
        connect(mpUI->NameLineEdit, SIGNAL( SoftValidityChanged(bool) ), mpUI->ValidityLabel, SLOT( SetValid(bool) ) );

        mpValidator->SetProperty(pProperty);
        mpUI->NameLineEdit->SetSoftValidator(mpValidator);
        mOriginalNameWasValid = mpUI->NameLineEdit->IsInputValid();
    }

    TString Source = mpProperty->GetTemplateFileName();

    if (Source.IsEmpty())
        Source = "None";

    mpUI->SourceFileDisplayLabel->setText(TO_QSTRING(Source));

    connect(mpUI->ButtonBox, SIGNAL(accepted()), this, SLOT(ApplyChanges()));
    connect(mpUI->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

CTemplateEditDialog::~CTemplateEditDialog()
{
    delete mpUI;
}

// ************ PUBLIC SLOTS ************
void CTemplateEditDialog::ApplyChanges()
{
    // Make sure the user *really* wants to change the property if the hash used to be correct and now isn't...
    if (mOriginalNameWasValid && !mpUI->NameLineEdit->IsInputValid())
    {
        bool ReallyApply = UICommon::YesNoQuestion(this, "Name mismatch",
            "The new property name does not match the property ID. It is very likely that the original name was correct and the new one isn't. Are you sure you want to change it?");

        if (!ReallyApply)
            return;
    }

    FindEquivalentProperties(mpProperty);

    bool RenameAll = mpUI->RenameAllCheckBox->isChecked();

    // Update name
    TString NewName = TO_TSTRING(mpUI->NameLineEdit->text());
    if (NewName.IsEmpty()) NewName = "Unknown";

    if (mOriginalName != NewName)
    {
        // Rename properties
        if (RenameAll && (mGame >= EGame::EchoesDemo || mpProperty->Archetype() != nullptr))
        {
            NPropertyMap::SetPropertyName(mpProperty->ID(), mpProperty->HashableTypeName(), *NewName);
        }
    }

    // Update description
    TString NewDescription = TO_TSTRING(mpUI->DescriptionTextEdit->toPlainText());
    UpdateDescription(NewDescription);

    // Update type name
    TString NewTypeName = TO_TSTRING(mpUI->TypenameLineEdit->text());
    bool AllowTypeNameOverride = mpUI->OverrideTypeNameCheckBox->isChecked();
    UpdateTypeName(NewTypeName, AllowTypeNameOverride);

    // Resave templates
    NGameList::SaveTemplates();
    NPropertyMap::SaveMap();
    close();
}

void CTemplateEditDialog::RefreshTypeNameOverride()
{
    if (mpUI->OverrideTypeNameCheckBox->isChecked())
    {
        QString OverrideName = mpUI->TypenameLineEdit->text();
        mpValidator->SetTypeNameOverride(OverrideName);
    }
    else
    {
        mpValidator->SetTypeNameOverride("");
    }
}

// ************ PROTECTED ************
void CTemplateEditDialog::UpdateDescription(const TString& rkNewDesc)
{
    mpProperty->SetDescription(rkNewDesc);

    // Update all copies of this property in memory with the new description
    TString SourceFile = mpProperty->GetTemplateFileName();

    if (!SourceFile.IsEmpty())
    {
        std::list<IProperty*> Templates;
        NPropertyMap::RetrievePropertiesWithID(mpProperty->ID(), mpProperty->HashableTypeName(), Templates);

        for (auto Iter = Templates.begin(); Iter != Templates.end(); Iter++)
        {
            IProperty* pProperty = *Iter;

            if (pProperty->GetTemplateFileName() == SourceFile && pProperty->Description() == mOriginalDescription)
                pProperty->SetDescription(rkNewDesc);
        }
    }

    // Update equivalent properties with new description
    foreach (IProperty* pProperty, mEquivalentProperties)
    {
        pProperty->SetDescription(rkNewDesc);
    }
}

void CTemplateEditDialog::UpdateTypeName(const TString& kNewTypeName, bool AllowOverride)
{
    if (mOriginalTypeName != kNewTypeName || mOriginalAllowTypeNameOverride != AllowOverride)
    {
        // Get a list of properties to update.
        for (int GameIdx = 0; GameIdx < (int) EGame::Max; GameIdx++)
        {
            CGameTemplate* pGame = NGameList::GetGameTemplate( (EGame) GameIdx );

            if (pGame)
            {
                IProperty* pArchetype = pGame->FindPropertyArchetype(mOriginalTypeName);

                if (pArchetype)
                {
                    pGame->RenamePropertyArchetype(mOriginalTypeName, kNewTypeName);

                    if (pArchetype->Type() == EPropertyType::Enum || pArchetype->Type() == EPropertyType::Choice)
                    {
                        CEnumProperty* pEnum = TPropCast<CEnumProperty>(pArchetype);
                        pEnum->SetOverrideTypeName(AllowOverride);
                    }
                }
            }
        }
    }
}

void CTemplateEditDialog::FindEquivalentProperties(IProperty* pProperty)
{
    // This function creates a list of properties in other games that are equivalent to this one.
    // In this case "equivalent" means same template file and same ID string.
    // Since MP1 doesn't have property IDs, we don't apply this to MP1.
    if (mGame <= EGame::Prime) return;

    // Find the lowest-level archetype and retrieve the ID string relative to that archetype's XML file.
    while (pProperty->Archetype())
    {
        pProperty = pProperty->Archetype();
    }
    TString Name = pProperty->Name();
    TIDString IDString = pProperty->IDString(true);
    CScriptTemplate* pScript = pProperty->ScriptTemplate();

    // Now iterate over all games, check for an equivalent property in an equivalent XML file.
    for (int GameIdx = 0; GameIdx < (int) EGame::Max; GameIdx++)
    {
        EGame Game = (EGame) GameIdx;
        if (Game <= EGame::Prime || Game == mGame) continue;

        CGameTemplate* pGame = NGameList::GetGameTemplate(Game);

        // Check for equivalent properties in a script template
        CStructProperty* pStruct = nullptr;

        if (pScript)
        {
            u32 ObjectID = pScript->ObjectID();
            CScriptTemplate* pEquivalentScript = pGame->TemplateByID(ObjectID);

            if (pEquivalentScript)
            {
                pStruct = pEquivalentScript->Properties();
            }
        }
        // Check for equivalent properties in a property template
        else
        {
            pStruct = TPropCast<CStructProperty>( pGame->FindPropertyArchetype(Name) );
        }

        // If we have a struct, check if thestruct contains an equivalent property.
        if (pStruct)
        {
            IProperty* pEquivalentProperty = pStruct->ChildByIDString( IDString );

            if (pEquivalentProperty)
            {
                mEquivalentProperties << pEquivalentProperty;
            }
        }
    }
}
