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
    , mOriginalNameWasValid(true)
{
    mpUI->setupUi(this);

    mpUI->IDDisplayLabel->setText(TO_QSTRING(pProperty->IDString(false)));
    mpUI->PathDisplayLabel->setText(TO_QSTRING(pProperty->IDString(true)));
    mpUI->NameLineEdit->setText(TO_QSTRING(pProperty->Name()));
    mpUI->DescriptionTextEdit->setPlainText(TO_QSTRING(pProperty->Description()));

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

        //@FIXME
#if 0
        std::vector<TString> TemplateList;
        CGameTemplate::XMLsUsingID(pProperty->ID(), TemplateList);

        for (u32 iTemp = 0; iTemp < TemplateList.size(); iTemp++)
            mpUI->TemplatesListWidget->addItem(TO_QSTRING(TemplateList[iTemp]));
#endif

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

    TString NewName = TO_TSTRING(mpUI->NameLineEdit->text());
    if (NewName.IsEmpty()) NewName = "Unknown";

    if (mOriginalName != NewName)
    {
        // Rename properties
        if (RenameAll && (mGame >= EGame::EchoesDemo || mpProperty->Archetype() != nullptr))
        {
            NPropertyMap::SetPropertyName(mpProperty, *NewName);
        }
    }

    TString NewDescription = TO_TSTRING(mpUI->DescriptionTextEdit->toPlainText());
    UpdateDescription(NewDescription);

    // Resave templates
    NPropertyMap::SaveMap();
    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate( mpProperty->Game() );
    pGameTemplate->SaveGameTemplates();

    close();
}

// ************ PROTECTED ************
void CTemplateEditDialog::UpdateDescription(const TString& rkNewDesc)
{
    mpProperty->SetDescription(rkNewDesc);

    // Update all copies of this property in memory with the new description
    //@FIXME
#if 0
    TString SourceFile = mpProperty->GetTemplateFileName();

    if (!SourceFile.IsEmpty())
    {
        const std::vector<IProperty*>* pkTemplates = CGameTemplate::TemplatesWithMatchingID(mpProperty);

        if (pkTemplates)
        {
            for (u32 TemplateIdx = 0; TemplateIdx < pkTemplates->size(); TemplateIdx++)
            {
                IProperty* pProp = pkTemplates->at(TemplateIdx);

                if (pProp->GetTemplateFileName() == SourceFile && pProp->Description() == mOriginalDescription)
                    pProp->SetDescription(rkNewDesc);
            }
        }
    }

    // Update equivalent properties with new description
    foreach (IProperty* pProperty, mEquivalentProperties)
    {
        pProperty->SetDescription(rkNewDesc);
    }
#endif
}

void CTemplateEditDialog::FindEquivalentProperties(IProperty *pTemp)
{
    //FIXME
    /*
    if (mGame <= EGame::Prime) return;

    // Find the equivalent version of this property in other games.
    CScriptTemplate *pScript = pTemp->ScriptTemplate();
    TString Source = pTemp->FindStructSource();

    // Determine struct-relative ID string
    TIDString IDString;

    if (Source.IsEmpty())
        IDString = pTemp->IDString(true);

    else
    {
        IDString = pTemp->IDString(false);
        CStructTemplate *pParent = pTemp->Parent();

        while (pParent)
        {
            if (!pParent->SourceFile().IsEmpty()) break;
            IDString.Prepend(pParent->IDString(false) + ":");
            pParent = pParent->Parent();
        }
    }

    QList<CGameTemplate*> GameList = QList<CGameTemplate*>::fromStdList(CGameTemplate::GameList());

    if (Source.IsEmpty())
    {
        u32 ObjectID = pScript->ObjectID();

        foreach (CGameTemplate *pGame, GameList)
        {
            if (pGame == pTemp->GameTemplate() || pGame->Game() <= EGame::Prime) continue;
            CScriptTemplate *pNewScript = pGame->TemplateByID(ObjectID);

            if (pNewScript)
            {
                IPropertyTemplate *pNewTemp = pNewScript->BaseStruct()->PropertyByIDString(IDString);

                if (pNewTemp)
                    mEquivalentProperties << pNewTemp;
            }
        }
    }

    else
    {
        foreach (CGameTemplate *pGame, GameList)
        {
            if (pGame == pTemp->GameTemplate() || pGame->Game() <= EGame::Prime) continue;
            CStructTemplate *pStruct = pGame->StructAtSource(Source);

            if (pStruct)
            {
                IPropertyTemplate *pNewTemp = pStruct->PropertyByIDString(IDString);

                if (pNewTemp)
                    mEquivalentProperties << pNewTemp;
            }
        }
    }*/
}
