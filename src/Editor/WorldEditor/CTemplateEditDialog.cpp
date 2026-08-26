#include "Editor/WorldEditor/CTemplateEditDialog.h"
#include "ui_CTemplateEditDialog.h"

#include "Editor/CPropertyNameValidator.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/Property/IProperty.h>
#include <Common/FileUtil.h>
#include <Core/Resource/Script/CGameTemplate.h>
#include <Core/Resource/Script/NGameList.h>
#include <Core/Resource/Script/NPropertyMap.h>

#include <QMenu>

CTemplateEditDialog::CTemplateEditDialog(IProperty *pProperty, QWidget *pParent)
    : QDialog(pParent)
    , mpUI(std::make_unique<Ui::CTemplateEditDialog>())
    , mpValidator(new CPropertyNameValidator(this))
    , mpProperty(pProperty)
    , mGame(pProperty->Game())
    , mOriginalName(pProperty->Name())
    , mOriginalDescription(pProperty->Description())
{
    mpUI->setupUi(this);

    mpUI->IDDisplayLabel->setText(TO_QSTRING(pProperty->IDString(false)));
    mpUI->PathDisplayLabel->setText(TO_QSTRING(pProperty->IDString(true)));
    mpUI->NameLineEdit->setText(TO_QSTRING(pProperty->Name()));
    mpUI->DescriptionTextEdit->setPlainText(TO_QSTRING(pProperty->Description()));

    EPropertyType Type = pProperty->Type();

    // Configure type name. Type name overrides are sourced from the name of the property archetype,
    // so this field should only be editable for properties that have an archetype.
    bool AllowTypeNameEdit = (pProperty->RootArchetype()->IsRootParent());

    if (AllowTypeNameEdit)
    {
        connect(mpUI->TypenameLineEdit, &QLineEdit::textChanged, this, &CTemplateEditDialog::RefreshTypeNameOverride);
        mOriginalTypeName = pProperty->RootArchetype()->Name();
        mpUI->TypenameLineEdit->setText(TO_QSTRING(mOriginalTypeName));
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
        mpUI->OverrideTypeNameCheckBox->setChecked(mOriginalAllowTypeNameOverride);
        connect(mpUI->OverrideTypeNameCheckBox, &QCheckBox::toggled, this, &CTemplateEditDialog::RefreshTypeNameOverride);
    }
    else
    {
        mpUI->OverrideTypeNameCheckBox->setHidden(true);
        mpUI->OverrideTypeNameCheckBox->setChecked(true);
    }
    RefreshTypeNameOverride();

    // Configure convert button
    if (Type == EPropertyType::Int || Type == EPropertyType::Choice || Type == EPropertyType::Flags || Type == EPropertyType::Sound)
    {
        QMenu* pConvertMenu = new QMenu(this);

        if (Type != EPropertyType::Int)
            pConvertMenu->addAction(tr("Int"), this, &CTemplateEditDialog::ConvertToInt);

        if (Type != EPropertyType::Choice)
            pConvertMenu->addAction(tr("Choice"), this, &CTemplateEditDialog::ConvertToChoice);

        if (Type != EPropertyType::Flags)
            pConvertMenu->addAction(tr("Flags"), this, &CTemplateEditDialog::ConvertToFlags);

        if (Type != EPropertyType::Sound)
            pConvertMenu->addAction(tr("Sound"), this, &CTemplateEditDialog::ConvertToSound);

        mpUI->TypeConversionButton->setMenu(pConvertMenu);
    }
    else
    {
        mpUI->TypeConversionWidget->setHidden(true);
    }

    // Hide templates list for MP1
    if (mGame <= EGame::Prime)
    {
        mpUI->TemplatesGroupBox->hide();
        mpUI->RenameAllCheckBox->setText(tr("Rename all copies of this property"));
        mpUI->ValidityLabel->hide();
        resize(width(), minimumHeight());
    }
    else
    {
        NGameList::LoadAllGameTemplates();

        const auto Templates = NPropertyMap::RetrieveXMLsWithProperty(pProperty->ID(), pProperty->HashableTypeName());
        for (const auto& Template : Templates)
            mpUI->TemplatesListWidget->addItem(TO_QSTRING(Template));

        mpUI->ValidityLabel->SetValidityText(tr("Hash match! Property name is likely correct."), tr("Hash mismatch! Property name is likely wrong."));
        connect(mpUI->NameLineEdit, &CSoftValidatorLineEdit::SoftValidityChanged, mpUI->ValidityLabel, &CValidityLabel::SetValid);

        mpValidator->SetProperty(pProperty);
        mpUI->NameLineEdit->SetSoftValidator(mpValidator);
        mOriginalNameWasValid = mpUI->NameLineEdit->IsInputValid();
    }

    TString Source = mpProperty->GetTemplateFileName();

    if (Source.IsEmpty())
        Source = "None";

    mpUI->SourceFileDisplayLabel->setText(TO_QSTRING(Source));

    connect(mpUI->ButtonBox, &QDialogButtonBox::accepted, this, &CTemplateEditDialog::ApplyChanges);
    connect(mpUI->ButtonBox, &QDialogButtonBox::rejected, this, &CTemplateEditDialog::close);
}

CTemplateEditDialog::~CTemplateEditDialog() = default;

// ************ PUBLIC SLOTS ************
void CTemplateEditDialog::ApplyChanges()
{
    // Make sure the user *really* wants to change the property if the hash used to be correct and now isn't...
    if (mOriginalNameWasValid && !mpUI->NameLineEdit->IsInputValid())
    {
        const bool ReallyApply = UICommon::YesNoQuestion(this, tr("Name mismatch"),
                                                         tr("The new property name does not match the property ID. It is very likely that the original name was correct and the new one isn't. Are you sure you want to change it?"));

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
            NPropertyMap::SetPropertyName(mpProperty->ID(), mpProperty->HashableTypeName(), NewName);
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
        mpValidator->SetTypeNameOverride(std::move(OverrideName));
    }
    else
    {
        mpValidator->SetTypeNameOverride(QString());
    }
}

void CTemplateEditDialog::ConvertPropertyType(EPropertyType Type)
{
    const char* pkCurType = TEnumReflection<EPropertyType>::ConvertValueToString(mpProperty->Type());
    const char* pkNewType = TEnumReflection<EPropertyType>::ConvertValueToString(Type);

    if (UICommon::YesNoQuestion(this, tr("Warning"),
                                tr("You are converting a %1 property to %2. This cannot be undone. Are you sure?")
                                    .arg(pkCurType)
                                    .arg(pkNewType)))
    {
        if (mpProperty->ConvertType(Type))
        {
            mpProperty = nullptr;
            emit PerformedTypeConversion();
            close();
        }
        else
        {
            UICommon::ErrorMsg(this, tr("Type conversion failed; conversion between these types is not supported."));
        }
    }
}

void CTemplateEditDialog::ConvertToInt()
{
    ConvertPropertyType( EPropertyType::Int );
}

void CTemplateEditDialog::ConvertToChoice()
{
    ConvertPropertyType( EPropertyType::Choice );
}

void CTemplateEditDialog::ConvertToSound()
{
    ConvertPropertyType( EPropertyType::Sound );
}

void CTemplateEditDialog::ConvertToFlags()
{
    ConvertPropertyType( EPropertyType::Flags );
}

// ************ PROTECTED ************
void CTemplateEditDialog::UpdateDescription(const TString& rkNewDesc)
{
    mpProperty->SetDescription(rkNewDesc);

    // Update all copies of this property in memory with the new description
    const TString SourceFile = mpProperty->GetTemplateFileName();

    if (!SourceFile.IsEmpty())
    {
        auto Templates = NPropertyMap::RetrievePropertiesWithID(mpProperty->ID(), mpProperty->HashableTypeName());

        for (auto* property : Templates)
        {
            if (property->GetTemplateFileName() == SourceFile && property->Description() == mOriginalDescription)
                property->SetDescription(rkNewDesc);
        }
    }

    // Update equivalent properties with new description
    for (IProperty* pProperty : mEquivalentProperties)
    {
        pProperty->SetDescription(rkNewDesc);
    }
}

// TODO: For whatever reason, the templates for Metroid Prime: Remastered were added to the
//       templates folder without actually adding support for any of the new data types (STRP, etc)
//       so actually trying to use the templates window will crash.
//
//       I really don't like stubbing out support like this, but it would've been really cool if
//       support was added by those that actually know the formats at the time!
//
//       When removing, make sure to do the same to the constant in NGameList.cpp as well.
constexpr auto GamesMinusMP1R = static_cast<int>(EGame::Max) - 1;

void CTemplateEditDialog::UpdateTypeName(const TString& kNewTypeName, bool AllowOverride)
{
    if (mOriginalTypeName != kNewTypeName || mOriginalAllowTypeNameOverride != AllowOverride)
    {
        if (FileUtil::IsValidName(kNewTypeName, false))
        {
            const bool WasUnknown = mOriginalTypeName.Contains("Unknown") || mOriginalTypeName.Contains("Struct");

            // Get a list of properties to update.
            for (int GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
            {
                const auto Game = EGame(GameIdx);
                if (WasUnknown && Game != mpProperty->Game())
                    continue;

                CGameTemplate* pGame = NGameList::GetGameTemplate(Game);
                if (!pGame)
                    continue;

                IProperty* pArchetype = pGame->FindPropertyArchetype(mOriginalTypeName);
                if (!pArchetype)
                    continue;

                pGame->RenamePropertyArchetype(mOriginalTypeName, kNewTypeName);

                if (pArchetype->Type() == EPropertyType::Enum || pArchetype->Type() == EPropertyType::Choice)
                {
                    auto* pEnum = TPropCast<CEnumProperty>(pArchetype);
                    pEnum->SetOverrideTypeName(AllowOverride);
                }
            }
        }
        else if (mOriginalTypeName != kNewTypeName)
        {
            UICommon::ErrorMsg(this, tr("Type rename failed because the name you entered \"%1\" is invalid.").arg(TO_QSTRING(kNewTypeName)));
        }
    }
}

void CTemplateEditDialog::FindEquivalentProperties(IProperty* pProperty)
{
    // This function creates a list of properties in other games that are equivalent to this one.
    // In this case "equivalent" means same template file and same ID string.
    // Since MP1 doesn't have property IDs, we don't apply this to MP1.
    if (mGame <= EGame::Prime)
        return;

    // Find the lowest-level archetype and retrieve the ID string relative to that archetype's XML file.
    while (pProperty->Archetype())
    {
        pProperty = pProperty->Archetype();
    }

    const TString& Name = pProperty->Name();
    TIDString IDString = pProperty->IDString(true);
    CScriptTemplate* pScript = pProperty->ScriptTemplate();

    // Now iterate over all games, check for an equivalent property in an equivalent XML file.
    for (int GameIdx = 0; GameIdx < GamesMinusMP1R; GameIdx++)
    {
        const auto Game = static_cast<EGame>(GameIdx);
        if (Game <= EGame::Prime || Game == mGame)
            continue;

        CGameTemplate* pGame = NGameList::GetGameTemplate(Game);

        // Check for equivalent properties in a script template
        CStructProperty* pStruct = nullptr;

        if (pScript)
        {
            const uint32_t ObjectID = pScript->ObjectID();
            CScriptTemplate* pEquivalentScript = pGame->TemplateByID(ObjectID);

            if (pEquivalentScript)
            {
                pStruct = pEquivalentScript->Properties();
            }
        }
        // Check for equivalent properties in a property template
        else
        {
            pStruct = TPropCast<CStructProperty>(pGame->FindPropertyArchetype(Name));
        }

        // If we have a struct, check if thestruct contains an equivalent property.
        if (pStruct)
        {
            IProperty* pEquivalentProperty = pStruct->ChildByIDString(IDString);

            if (pEquivalentProperty)
            {
                mEquivalentProperties.push_back(pEquivalentProperty);
            }
        }
    }
}
