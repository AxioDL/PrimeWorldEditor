#include "CScriptObject.h"
#include "../CAnimSet.h"
#include "CMasterTemplate.h"

CScriptObject::CScriptObject(CGameArea *pArea, CScriptLayer *pLayer, CScriptTemplate *pTemplate)
{
    mpArea = pArea;
    mpLayer = pLayer;
    mpTemplate = pTemplate;
    mpProperties = nullptr;
    mAttribFlags = 0;
    mpTemplate->AddObject(this);
}

CScriptObject::~CScriptObject()
{
    if (mpProperties) delete mpProperties;
    mpTemplate->RemoveObject(this);
}

// ************ DATA MANIPULATION ************
void CScriptObject::EvalutateXForm()
{
    // Reset XForm values to defaults
    mPosition    = CVector3f(0);
    mRotation    = CVector3f(0);
    mScale       = CVector3f(1);
    mVolumeSize  = CVector3f(0);
    mVolumeShape = -1;

    // Look for PRS attribs
    for (u32 a = 0; a < mAttribs.size(); a++)
    {
        if ((mAttribs[a].Type == ePositionAttrib) ||
            (mAttribs[a].Type == eRotationAttrib) ||
            (mAttribs[a].Type == eScaleAttrib)    ||
            (mAttribs[a].Type == eVolumeAttrib))
        {
            CVector3Property *attrib = static_cast<CVector3Property*>(mAttribs[a].Prop);

            if (mAttribs[a].Type == ePositionAttrib)
                mPosition = attrib->Get();
            else if (mAttribs[a].Type == eRotationAttrib)
                mRotation = attrib->Get();
            else if (mAttribs[a].Type == eScaleAttrib)
                mScale = attrib->Get();
            else if (mAttribs[a].Type == eVolumeAttrib) {
                mVolumeSize = attrib->Get();
                mVolumeShape = mAttribs[a].Settings;
            }
        }
    }
}

void CScriptObject::EvaluateInstanceName()
{
    // Reset instance name to default
    mInstanceName = mpTemplate->TemplateName();

    // Simply look for an instance name - set if we find it
    for (u32 a = 0; a < mAttribs.size(); a++)
    {
        if (mAttribs[a].Type == eNameAttrib)
        {
            CStringProperty *str = static_cast<CStringProperty*>(mAttribs[a].Prop);
            mInstanceName = str->Get();
            return;
        }
    }
}

void CScriptObject::EvaluateTevColor()
{
    // Evaluate the TEV color initializer - this is used for beam troopers
    mTevColor = CColor::skWhite; // Initialize to white in case there's no vulnerability attrib

    for (u32 a = 0; a < mAttribs.size(); a++)
    {
        if (mAttribs[a].Type == eVulnerabilityAttrib)
        {
            CPropertyStruct* vuln = static_cast<CPropertyStruct*>(mAttribs[a].Prop);

            u32 Power = static_cast<CLongProperty*>(vuln->PropertyByIndex(0))->Get();
            u32 Ice = static_cast<CLongProperty*>(vuln->PropertyByIndex(1))->Get();
            u32 Wave = static_cast<CLongProperty*>(vuln->PropertyByIndex(2))->Get();
            u32 Plasma = static_cast<CLongProperty*>(vuln->PropertyByIndex(3))->Get();

            if (Plasma != 2)     mTevColor = CColor::skRed;
            else if (Ice != 2)   mTevColor = CColor::skWhite;
            else if (Power != 2) mTevColor = CColor::skYellow;
            else if (Wave != 2)  mTevColor = CColor::skPurple;
            else                 mTevColor = CColor::skWhite;

            break;
        }
    }
}

void CScriptObject::EvaluateDisplayModel()
{
    // Look for animset or model
    for (u32 a = 0; a < mAttribs.size(); a++)
    {
        // Evaluate AnimSet attrib
        if (mAttribs[a].Type == eAnimSetAttrib)
        {
            // Get the AnimationParameters struct so we can fetch relevant values from it...
            SAttrib *Attrib = &mAttribs[a];
            CPropertyStruct *AnimParams = static_cast<CPropertyStruct*>(Attrib->Prop);
            EGame game = mpTemplate->MasterTemplate()->GetGame();

            CResource *ANCS;
            if (Attrib->Res)
                ANCS = Attrib->Res;
            else if (game <= eCorruption)
                ANCS = static_cast<CFileProperty*>( (*AnimParams)[0] )->Get();
            else
                ANCS = static_cast<CFileProperty*>( (*AnimParams)[1] )->Get();

            if ((ANCS) && (ANCS->Type() == eCharacter))
            {
                // Get animset + node index and return the relevant model
                CAnimSet *set = static_cast<CAnimSet*>(ANCS);
                u32 node;

                if (mpTemplate->MasterTemplate()->GetGame() >= eCorruptionProto)
                    node = 0;
                else if (Attrib->Settings == -1)
                    node = static_cast<CLongProperty*>( (*AnimParams)[1] )->Get();
                else
                    node = Attrib->Settings;

                CModel *model = set->getNodeModel(node);
                if (model && (model->Type() == eModel))
                {
                    mpDisplayModel = model;
                    return;
                }
            }
        }

        // Evaluate Model attrib
        else if (mAttribs[a].Type == eModelAttrib)
        {
            SAttrib *Attrib = &mAttribs[a];
            CResource *CMDL;

            if (Attrib->Res)
                CMDL = Attrib->Res;
            else
                CMDL = static_cast<CFileProperty*>(Attrib->Prop)->Get();

            if (CMDL && (CMDL->Type() == eModel))
            {
                mpDisplayModel = static_cast<CModel*>(CMDL);
                return;
            }
        }
    }

    // No valid display asset
    mpDisplayModel = nullptr;
    return;
}

// ************ GETTERS ************
CPropertyBase* CScriptObject::PropertyByIndex(u32 index)
{
    return mpProperties->PropertyByIndex(index);
}

CPropertyBase* CScriptObject::PropertyByName(std::string name)
{
    return mpProperties->PropertyByName(name);
}

CScriptTemplate* CScriptObject::Template()
{
    return mpTemplate;
}

CMasterTemplate* CScriptObject::MasterTemplate()
{
    return mpTemplate->MasterTemplate();
}

CGameArea* CScriptObject::Area()
{
    return mpArea;
}

CScriptLayer* CScriptObject::Layer()
{
    return mpLayer;
}

CPropertyStruct* CScriptObject::Properties()
{
    return mpProperties;
}

u32 CScriptObject::ObjectTypeID() const
{
    return mpTemplate->ObjectID();
}

u32 CScriptObject::InstanceID() const
{
    return mInstanceID;
}

u32 CScriptObject::NumInLinks() const
{
    return mInConnections.size();
}

u32 CScriptObject::NumOutLinks() const
{
    return mOutConnections.size();
}

const SLink& CScriptObject::InLink(u32 index) const
{
    return mInConnections[index];
}

const SLink& CScriptObject::OutLink(u32 index) const
{
    return mOutConnections[index];
}

// Attribs
CVector3f CScriptObject::GetPosition() const
{
    return mPosition;
}

CVector3f CScriptObject::GetRotation() const
{
    return mRotation;
}

CVector3f CScriptObject::GetScale() const
{
    return mScale;
}

CVector3f CScriptObject::GetVolume() const
{
    return mVolumeSize;
}

u32 CScriptObject::GetVolumeShape() const
{
    return mVolumeShape;
}

std::string CScriptObject::GetInstanceName() const
{
    return mInstanceName;
}

CColor CScriptObject::GetTevColor() const
{
    return mTevColor;
}

CModel* CScriptObject::GetDisplayModel() const
{
    return mpDisplayModel;
}

int CScriptObject::GetAttribFlags() const
{
    return mAttribFlags;
}

// ************ STATIC ************
CScriptObject* CScriptObject::CopyFromTemplate(CScriptTemplate *pTemp, CGameArea *pArea, CScriptLayer *pLayer)
{
    CScriptObject *pObj = new CScriptObject(pArea, pLayer, pTemp);

    CStructTemplate *pBaseStruct = pTemp->BaseStruct();
    pObj->mpProperties = CPropertyStruct::CopyFromTemplate(pBaseStruct);

    return pObj;
}
