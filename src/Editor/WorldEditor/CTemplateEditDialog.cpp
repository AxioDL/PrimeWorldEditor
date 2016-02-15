#include "CTemplateEditDialog.h"
#include "ui_CTemplateEditDialog.h"

#include "Editor/UICommon.h"
#include <Core/Resource/Cooker/CTemplateWriter.h>
#include <Core/Resource/Factory/CTemplateLoader.h>
#include <Core/Resource/Script/CMasterTemplate.h>

CTemplateEditDialog::CTemplateEditDialog(IPropertyTemplate *pTemplate, QWidget *pParent)
    : QDialog(pParent)
    , ui(new Ui::CTemplateEditDialog)
    , mpTemplate(pTemplate)
{
    ui->setupUi(this);

    mGame = pTemplate->Game();
    mOriginalName = pTemplate->Name();
    mOriginalDescription = pTemplate->Description();

    ui->IDDisplayLabel->setText(TO_QSTRING(pTemplate->IDString(false)));
    ui->PathDisplayLabel->setText(TO_QSTRING(pTemplate->IDString(true)));
    ui->NameLineEdit->setText(TO_QSTRING(pTemplate->Name()));
    ui->DescriptionTextEdit->setPlainText(TO_QSTRING(pTemplate->Description()));

    if (mGame <= ePrime)
    {
        ui->TemplatesGroupBox->hide();
        ui->RenameAllCheckBox->setText("Rename all copies of this property");
        resize(width(), minimumHeight());
    }

    else
    {
        CTemplateLoader::LoadAllGames();
        std::vector<TString> TemplateList = CMasterTemplate::GetXMLsUsingID(pTemplate->PropertyID());

        for (u32 iTemp = 0; iTemp < TemplateList.size(); iTemp++)
            ui->TemplatesListWidget->addItem(TO_QSTRING(TemplateList[iTemp]));
    }

    TString Source;

    if (mpTemplate->Type() == eStructProperty || mpTemplate->Type() == eArrayProperty)
        Source = static_cast<CStructTemplate*>(mpTemplate)->SourceFile();
    else if (mpTemplate->Type() == eEnumProperty)
        Source = static_cast<CEnumTemplate*>(mpTemplate)->SourceFile();
    else if (mpTemplate->Type() == eBitfieldProperty)
        Source = static_cast<CBitfieldTemplate*>(mpTemplate)->SourceFile();

    if (Source.IsEmpty())
    {
        CStructTemplate *pParent = mpTemplate->Parent();
        while (pParent)
        {
            Source = pParent->SourceFile();
            if (!Source.IsEmpty()) break;
            pParent = pParent->Parent();
        }
    }

    if (Source.IsEmpty())
    {
        if (mpTemplate->ScriptTemplate())
            Source = mpTemplate->ScriptTemplate()->SourceFile();
        if (Source.IsEmpty())
            Source = "None";
    }

    ui->SourceFileDisplayLabel->setText(TO_QSTRING(Source));

    connect(ui->ButtonBox, SIGNAL(accepted()), this, SLOT(ApplyChanges()));
    connect(ui->ButtonBox, SIGNAL(rejected()), this, SLOT(close()));
}

CTemplateEditDialog::~CTemplateEditDialog()
{
    delete ui;
}

// ************ PUBLIC SLOTS ************
void CTemplateEditDialog::ApplyChanges()
{
    FindEquivalentProperties(mpTemplate);

    bool NeedsListResave = false;
    bool RenameAll = ui->RenameAllCheckBox->isChecked();

    TString NewName = TO_TSTRING(ui->NameLineEdit->text());
    if (NewName.IsEmpty()) NewName = "Unknown";

    if (mOriginalName != NewName)
    {
        // Rename properties
        if (RenameAll && (mGame >= eEchoesDemo || mpTemplate->IsFromStructTemplate()))
        {
            CMasterTemplate::RenameProperty(mpTemplate, NewName);

            // Add modified templates to pending resave list
            const std::vector<IPropertyTemplate*> *pList = CMasterTemplate::GetTemplatesWithMatchingID(mpTemplate);

            if (pList)
            {
                for (u32 iTemp = 0; iTemp < pList->size(); iTemp++)
                    AddTemplate( pList->at(iTemp) );
            }
        }

        mpTemplate->SetName(NewName); // If mpTemplate has an overridden name then CMasterTemplate::RenameProperty won't touch it

        if (RenameAll && mGame >= eEchoesDemo)
            NeedsListResave = true;
    }

    TString NewDescription = TO_TSTRING(ui->DescriptionTextEdit->toPlainText());
    UpdateDescription(NewDescription);

    // Resave templates
    foreach (CScriptTemplate *pScript, mScriptTemplatesToResave)
        CTemplateWriter::SaveScriptTemplate(pScript);

    foreach (CStructTemplate *pStruct, mStructTemplatesToResave)
        CTemplateWriter::SaveStructTemplate(pStruct);

    if (NeedsListResave)
        CTemplateWriter::SavePropertyList();

    close();
}

// ************ PROTECTED ************
void CTemplateEditDialog::AddTemplate(IPropertyTemplate *pTemp)
{
    if (pTemp->IsFromStructTemplate())
    {
        TString Source = pTemp->FindStructSource();

        if (!Source.IsEmpty())
        {
            CStructTemplate *pStruct = pTemp->MasterTemplate()->GetStructAtSource(Source);

            if (!mStructTemplatesToResave.contains(pStruct))
                mStructTemplatesToResave << pStruct;
        }
    }

    else
    {
        CScriptTemplate *pScript = pTemp->ScriptTemplate();

        if (pScript)
        {
            if (!mScriptTemplatesToResave.contains(pScript))
                mScriptTemplatesToResave << pScript;
        }

        else
        {
            Log::Error("Can't determine where property " + pTemp->IDString(true) + " comes from");
        }
    }
}

void CTemplateEditDialog::UpdateDescription(const TString& rkNewDesc)
{
    mpTemplate->SetDescription(rkNewDesc);
    AddTemplate(mpTemplate);

    // Update all copies of this property in memory with the new description
    TString SourceFile = mpTemplate->FindStructSource();

    if (!SourceFile.IsEmpty())
    {
        const std::vector<IPropertyTemplate*> *pkTemplates = CMasterTemplate::GetTemplatesWithMatchingID(mpTemplate);

        if (pkTemplates)
        {
            for (u32 iTemp = 0; iTemp < pkTemplates->size(); iTemp++)
            {
                IPropertyTemplate *pTemp = pkTemplates->at(iTemp);

                if (pTemp->FindStructSource() == SourceFile && pTemp->Description() == mOriginalDescription)
                    pTemp->SetDescription(rkNewDesc);
            }
        }
    }

    // Update equivalent properties with new description
    foreach (IPropertyTemplate *pTemp, mEquivalentProperties)
    {
        pTemp->SetDescription(rkNewDesc);
        AddTemplate(pTemp);
    }
}

void CTemplateEditDialog::FindEquivalentProperties(IPropertyTemplate *pTemp)
{
    if (mGame <= ePrime) return;

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

    QList<CMasterTemplate*> MasterList = QList<CMasterTemplate*>::fromStdList(CMasterTemplate::GetMasterList());

    if (Source.IsEmpty())
    {
        u32 ObjectID = pScript->ObjectID();

        foreach (CMasterTemplate *pMaster, MasterList)
        {
            if (pMaster == pTemp->MasterTemplate() || pMaster->GetGame() <= ePrime) continue;
            CScriptTemplate *pNewScript = pMaster->TemplateByID(ObjectID);

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
        foreach (CMasterTemplate *pMaster, MasterList)
        {
            if (pMaster == pTemp->MasterTemplate() || pMaster->GetGame() <= ePrime) continue;
            CStructTemplate *pStruct = pMaster->GetStructAtSource(Source);

            if (pStruct)
            {
                IPropertyTemplate *pNewTemp = pStruct->PropertyByIDString(IDString);

                if (pNewTemp)
                    mEquivalentProperties << pNewTemp;
            }
        }
    }
}
