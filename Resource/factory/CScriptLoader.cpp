#include "CScriptLoader.h"
#include "../script/CMasterTemplate.h"
#include <Core/CResCache.h>
#include <Core/Log.h>
#include <iostream>
#include <sstream>

CScriptLoader::CScriptLoader()
{
    mpObj = nullptr;
}

CPropertyStruct* CScriptLoader::LoadStructMP1(CInputStream& SCLY, CStructTemplate *tmp)
{
    u32 StructStart = SCLY.Tell();
    CPropertyStruct *PropStruct = new CPropertyStruct();
    PropStruct->tmp = tmp;

    // Verify property count
    s32 TemplatePropCount = tmp->TemplateCount();
    if (TemplatePropCount >= 0)
    {
        u32 FilePropCount = SCLY.ReadLong();
        if (TemplatePropCount != FilePropCount)
            Log::FileWarning(SCLY.GetSourceString(), StructStart, "Struct \"" + tmp->Name() + "\" template prop count doesn't match file");
    }

    // Parse properties
    u32 PropCount = tmp->Count();
    PropStruct->Reserve(PropCount);

    for (u32 p = 0; p < PropCount; p++)
    {
        CPropertyBase *prop = nullptr;
        CPropertyTemplate *proptmp = tmp->PropertyByIndex(p);
        EPropertyType type = proptmp->Type();

        switch (type)
        {

        case eBoolProperty: {
            bool v = (SCLY.ReadByte() == 1);
            prop = new CBoolProperty(v);
            break;
        }
        case eByteProperty: {
            char v = SCLY.ReadByte();
            prop = new CByteProperty(v);
            break;
        }
        case eShortProperty: {
            short v = SCLY.ReadShort();
            prop = new CShortProperty(v);
            break;
        }
        case eLongProperty: {
            long v = SCLY.ReadLong();
            prop = new CLongProperty(v);
            break;
        }
        case eFloatProperty: {
            float v = SCLY.ReadFloat();
            prop = new CFloatProperty(v);
            break;
        }
        case eStringProperty: {
            std::string v = SCLY.ReadString();
            prop = new CStringProperty(v);
            break;
        }
        case eVector3Property: {
            CVector3f v(SCLY);
            prop = new CVector3Property(v);
            break;
        }
        case eColorProperty: {
            CVector4f color(SCLY);
            CColor v(color.x, color.y, color.z, color.w);
            prop = new CColorProperty(v);
            break;
        }
        case eFileProperty: {
            u32 ResID = SCLY.ReadLong();
            const CStringList& Extensions = static_cast<CFileTemplate*>(proptmp)->Extensions();

            CResource *pRes = nullptr;

            for (auto it = Extensions.begin(); it != Extensions.end(); it++)
            {
                const std::string& ext = *it;
                if ((ext != "MREA") && (ext != "MLVL")) // Let's avoid recursion please
                    pRes = gResCache.GetResource(ResID, ext);

                if (pRes) break;
            }

            prop = new CFileProperty(pRes);
            break;
        }
        case eStructProperty: {
            CStructTemplate *StructTmp = tmp->StructByIndex(p);
            prop = LoadStructMP1(SCLY, StructTmp);
            break;
        }
        }

        if (prop)
        {
            prop->tmp = proptmp;
            PropStruct->Properties.push_back(prop);
        }
    }

    return PropStruct;
}

CScriptObject* CScriptLoader::LoadObjectMP1(CInputStream& SCLY)
{
    u32 ObjStart = SCLY.Tell();
    u8 type = SCLY.ReadByte();
    u32 size = SCLY.ReadLong();
    u32 end = SCLY.Tell() + size;

    CScriptTemplate *tmp = mpMaster->TemplateByID((u32) type);
    if (!tmp)
    {
        // No valid template for this object; can't load
        Log::FileError(SCLY.GetSourceString(), ObjStart, "Invalid object ID encountered  - " + StringUtil::ToHexString(type));
        SCLY.Seek(end, SEEK_SET);
        return nullptr;
    }

    mpObj = new CScriptObject(mpArea, mpLayer, tmp);
    mpObj->mInstanceID = SCLY.ReadLong();

    // Load connections
    u32 numConnections = SCLY.ReadLong();
    mpObj->mOutConnections.reserve(numConnections);

    for (u32 c = 0; c < numConnections; c++)
    {
        SLink con;
        con.State = SCLY.ReadLong();
        con.Message = SCLY.ReadLong();
        con.ObjectID = SCLY.ReadLong();
        mpObj->mOutConnections.push_back(con);
    }

    // Load object...
    CStructTemplate *base = tmp->BaseStruct();
    mpObj->mpProperties = LoadStructMP1(SCLY, base);
    SetupAttribs();

    // Cleanup and return
    SCLY.Seek(end, SEEK_SET);
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP1(CInputStream &SCLY)
{
    u32 LayerStart = SCLY.Tell();

    SCLY.Seek(0x1, SEEK_CUR); // One unknown byte at the start of each layer
    u32 NumObjects = SCLY.ReadLong();

    mpLayer = new CScriptLayer();
    mpLayer->Reserve(NumObjects);

    for (u32 iObj = 0; iObj < NumObjects; iObj++)
    {
        CScriptObject *pObj = LoadObjectMP1(SCLY);
        if (pObj)
            mpLayer->AddObject(pObj);
    }

    // Layer sizes are always a multiple of 32 - skip end padding before returning
    u32 remaining = 32 - ((SCLY.Tell() - LayerStart) & 0x1F);
    SCLY.Seek(remaining, SEEK_CUR);
    return mpLayer;
}

void CScriptLoader::LoadStructMP2(CInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp)
{
    // Verify property count
    if (!pTemp->IsSingleProperty())
    {
        u16 NumProperties = SCLY.ReadShort();
        if ((pTemp->TemplateCount() >= 0) && (NumProperties != pTemp->TemplateCount()))
           Log::FileWarning(SCLY.GetSourceString(), SCLY.Tell() - 2, "Struct \"" + pTemp->Name() + "\" template property count doesn't match file");
    }

    // Parse properties
    u32 PropCount = pTemp->Count();
    pStruct->Reserve(PropCount);

    for (u32 p = 0; p < PropCount; p++)
    {
        CPropertyBase *pProp;
        CPropertyTemplate *pPropTemp;
        u32 PropertyStart = SCLY.Tell();
        u32 PropertyID = -1;
        u16 PropertyLength = 0;
        u32 NextProperty = 0;

        if (pTemp->IsSingleProperty())
        {
            pProp = pStruct->PropertyByIndex(p);
            pPropTemp = pTemp->PropertyByIndex(p);
        }
        else
        {
            PropertyID = SCLY.ReadLong();
            PropertyLength = SCLY.ReadShort();
            NextProperty = SCLY.Tell() + PropertyLength;

            pProp = pStruct->PropertyByID(PropertyID);
            pPropTemp = pTemp->PropertyByID(PropertyID);
        }

        if (!pPropTemp)
            Log::FileError(SCLY.GetSourceString(), PropertyStart, "Can't find template for property " + StringUtil::ToHexString(PropertyID) + " - skipping");

        else
        {
            switch (pPropTemp->Type())
            {

            case eBoolProperty: {
                CBoolProperty *pBoolCast = static_cast<CBoolProperty*>(pProp);
                pBoolCast->Set( (SCLY.ReadByte() != 0) );
                break;
            }

            case eByteProperty: {
                CByteProperty *pByteCast = static_cast<CByteProperty*>(pProp);
                pByteCast->Set(SCLY.ReadByte());
                break;
            }

            case eShortProperty: {
                CShortProperty *pShortCast = static_cast<CShortProperty*>(pProp);
                pShortCast->Set(SCLY.ReadShort());
                break;
            }

            case eLongProperty: {
                CLongProperty *pLongCast = static_cast<CLongProperty*>(pProp);
                pLongCast->Set(SCLY.ReadLong());
                break;
            }

            case eFloatProperty: {
                CFloatProperty *pFloatCast = static_cast<CFloatProperty*>(pProp);
                pFloatCast->Set(SCLY.ReadFloat());
                break;
            }

            case eStringProperty: {
                CStringProperty *pStringCast = static_cast<CStringProperty*>(pProp);
                pStringCast->Set(SCLY.ReadString());
                break;
            }

            case eVector3Property: {
                CVector3Property *pVector3Cast = static_cast<CVector3Property*>(pProp);
                pVector3Cast->Set(CVector3f(SCLY));
                break;
            }

            case eColorProperty: {
                CColorProperty *pColorCast = static_cast<CColorProperty*>(pProp);
                CVector4f Color(SCLY);
                pColorCast->Set(CColor(Color.x, Color.y, Color.z, Color.w));
                break;
            }

            case eFileProperty: {
                CFileProperty *pFileCast = static_cast<CFileProperty*>(pProp);

                CUniqueID ResID = (mVersion < eCorruptionProto ? SCLY.ReadLong() : SCLY.ReadLongLong());
                const CStringList& Extensions = static_cast<CFileTemplate*>(pPropTemp)->Extensions();

                CResource *pRes = nullptr;

                // Check for each extension individually until we find a match
                // This could be done better with a function to fetch the extension given the resource ID
                // and a "does resource exist" function, but this will do for now
                bool hasIgnoredExt = false;

                if (ResID.IsValid())
                {
                    for (auto it = Extensions.begin(); it != Extensions.end(); it++)
                    {
                        const std::string& ext = *it;

                        if ((ext != "MREA") && (ext != "MLVL")) {
                            pRes = gResCache.GetResource(ResID, ext);
                            if (pRes) break;
                        }

                        else
                            hasIgnoredExt = true;
                    }
                }

                // Property may have an incorrect extension listed - print error
                if ((!pRes) && (CUniqueID(ResID).IsValid()) && (!hasIgnoredExt))
                {
                    std::string ExtList;
                    for (auto it = Extensions.begin(); it != Extensions.end(); it++)
                    {
                        if (it != Extensions.begin()) ExtList += "/";
                        ExtList += *it;
                    }
                    Log::FileWarning(SCLY.GetSourceString(), "Incorrect resource type? " + ExtList + " " + StringUtil::ToHexString(PropertyID));
                }

                pFileCast->Set(pRes);
                break;
            }

            case eUnknownProperty: {
                CUnknownProperty *pUnknownCast = static_cast<CUnknownProperty*>(pProp);
                std::vector<u8> buf(PropertyLength);
                SCLY.ReadBytes(buf.data(), buf.size());
                pUnknownCast->Set(buf);
                break;
            }

            case eStructProperty: {
                CPropertyStruct *pStructCast = static_cast<CPropertyStruct*>(pProp);
                LoadStructMP2(SCLY, pStructCast, static_cast<CStructTemplate*>(pPropTemp));
                break;
            }

            }
        }

        if (NextProperty > 0)
            SCLY.Seek(NextProperty, SEEK_SET);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP2(CInputStream& SCLY)
{
    u32 ObjStart = SCLY.Tell();
    u32 ObjectID = SCLY.ReadLong();
    u16 ObjectSize = SCLY.ReadShort();
    u32 ObjEnd = SCLY.Tell() + ObjectSize;

    CScriptTemplate *pTemplate = mpMaster->TemplateByID(ObjectID);

    if (!pTemplate)
    {
        Log::FileError(SCLY.GetSourceString(), ObjStart, "Invalid object ID encountered: " + CFourCC(ObjectID).ToString());
        SCLY.Seek(ObjEnd, SEEK_SET);
        return nullptr;
    }

    mpObj = CScriptObject::CopyFromTemplate(pTemplate, mpArea, mpLayer);
    mpObj->mpTemplate = pTemplate;
    mpObj->mInstanceID = SCLY.ReadLong();

    // Load connections
    u32 NumConnections = SCLY.ReadShort();
    mpObj->mOutConnections.reserve(NumConnections);

    for (u32 iCon = 0; iCon < NumConnections; iCon++)
    {
        SLink con;
        con.State = SCLY.ReadLong();
        con.Message = SCLY.ReadLong();
        con.ObjectID = SCLY.ReadLong();
        mpObj->mOutConnections.push_back(con);
    }

    // Load object
    CStructTemplate *pBase = pTemplate->BaseStruct();
    SCLY.Seek(0x6, SEEK_CUR); // Skip base struct ID + size
    LoadStructMP2(SCLY, mpObj->mpProperties, pBase);
    SetupAttribs();

    SCLY.Seek(ObjEnd, SEEK_SET);
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP2(CInputStream& SCLY)
{
    CFourCC SCLY_Magic(SCLY);

    if      (SCLY_Magic == "SCLY") SCLY.Seek(0x6, SEEK_CUR);
    else if (SCLY_Magic == "SCGN") SCLY.Seek(0x2, SEEK_CUR);
    else
    {
        Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Invalid script layer magic: " + StringUtil::ToHexString((u32) SCLY_Magic.ToLong()));
        return nullptr;
    }

    u32 NumObjects = SCLY.ReadLong();

    mpLayer = new CScriptLayer();
    mpLayer->Reserve(NumObjects);

    for (u32 iObj = 0; iObj < NumObjects; iObj++)
    {
        CScriptObject *pObj = LoadObjectMP2(SCLY);
        if (pObj)
            mpLayer->AddObject(pObj);
    }

    if (SCLY_Magic == "SCGN")
    {
        mpLayer->SetName("Generated");
        mpLayer->SetActive(true);
    }
    return mpLayer;
}

void CScriptLoader::SetupAttribs()
{
    // Add template attributes
    u32 numAttribs = mpObj->mpTemplate->AttribCount();
    for (u32 a = 0; a < numAttribs; a++)
    {
        CAttribTemplate *AttribTmp = mpObj->mpTemplate->Attrib(a);
        CPropertyBase *prop = mpObj->PropertyByName( AttribTmp->Target() );

        // Check for static resource
        CResource *res = nullptr;
        std::string ResStr = AttribTmp->Resource();
        if (!ResStr.empty())
            res = gResCache.GetResource(ResStr);

        mpObj->mAttribs.emplace_back(CScriptObject::SAttrib(AttribTmp->Type(), res, AttribTmp->Settings(), prop) );
        mpObj->mAttribFlags |= AttribTmp->Type();
    }

    // Initial attribute evaluation
    mpObj->EvaluateInstanceName();
    mpObj->EvalutateXForm();
    mpObj->EvaluateTevColor();
    mpObj->EvaluateDisplayModel();
}

CScriptLayer* CScriptLoader::LoadLayer(CInputStream &SCLY, CGameArea *pArea, EGame version)
{
    if (!SCLY.IsValid()) return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = version;
    Loader.mpMaster = CMasterTemplate::GetMasterForGame(version);
    Loader.mpArea = pArea;

    if (!Loader.mpMaster)
    {
        Log::Write("This game doesn't have a master template; couldn't load script layer");
        return nullptr;
    }

    if (version <= ePrime)
        return Loader.LoadLayerMP1(SCLY);
    else
        return Loader.LoadLayerMP2(SCLY);
}
