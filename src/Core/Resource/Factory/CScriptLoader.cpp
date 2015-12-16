#include "CScriptLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/CResCache.h"
#include "Core/Log.h"
#include <iostream>
#include <sstream>

CScriptLoader::CScriptLoader()
{
    mpObj = nullptr;
}

CPropertyStruct* CScriptLoader::LoadStructMP1(CInputStream& SCLY, CStructTemplate *pTemp)
{
    u32 structStart = SCLY.Tell();
    CPropertyStruct *propStruct = new CPropertyStruct();
    propStruct->mpTemplate = pTemp;

    // Verify property count
    u32 propCount = pTemp->Count();

    if (!pTemp->IsSingleProperty())
    {
        u32 filePropCount = SCLY.ReadLong();
        if (propCount != filePropCount)
            Log::FileWarning(SCLY.GetSourceString(), structStart, "Struct \"" + pTemp->Name() + "\" template prop count doesn't match file");
    }

    // Parse properties
    propStruct->Reserve(propCount);

    for (u32 iProp = 0; iProp < propCount; iProp++)
    {
        CPropertyBase *pProp = nullptr;
        CPropertyTemplate *pPropTmp = pTemp->PropertyByIndex(iProp);
        EPropertyType type = pPropTmp->Type();

        switch (type)
        {

        case eBoolProperty: {
            bool v = (SCLY.ReadByte() == 1);
            pProp = new CBoolProperty(v);
            break;
        }
        case eByteProperty: {
            char v = SCLY.ReadByte();
            pProp = new CByteProperty(v);
            break;
        }
        case eShortProperty: {
            short v = SCLY.ReadShort();
            pProp = new CShortProperty(v);
            break;
        }
        case eLongProperty: {
            long v = SCLY.ReadLong();
            pProp = new CLongProperty(v);
            break;
        }
        case eBitfieldProperty: {
            long v = SCLY.ReadLong();
            pProp = new CBitfieldProperty(v);

            // Validate
            u32 mask = 0;
            CBitfieldTemplate *pBitfieldTemp = static_cast<CBitfieldTemplate*>(pPropTmp);
            for (u32 iMask = 0; iMask < pBitfieldTemp->NumFlags(); iMask++)
                mask |= pBitfieldTemp->FlagMask(iMask);

            u32 check = v & ~mask;
            if (check != 0) Log::FileWarning(SCLY.GetSourceString(), SCLY.Tell() - 4, "Bitfield property \"" + pBitfieldTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has flags set that aren't in the template: " + TString::HexString(check, true, true, 8));

            break;
        }
        case eEnumProperty: {
            CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pPropTmp);
            u32 ID = SCLY.ReadLong();
            u32 index = pEnumTemp->EnumeratorIndex(ID);
            if (index == -1) Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Enum property \"" + pEnumTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has invalid enumerator value: " + TString::HexString(ID, true, true, 8));
            pProp = new CEnumProperty(index);
            break;
        }
        case eFloatProperty: {
            float v = SCLY.ReadFloat();
            pProp = new CFloatProperty(v);
            break;
        }
        case eStringProperty: {
            TString v = SCLY.ReadString();
            pProp = new CStringProperty(v);
            break;
        }
        case eVector3Property: {
            CVector3f v(SCLY);
            pProp = new CVector3Property(v);
            break;
        }
        case eColorProperty: {
            CColor v(SCLY);
            pProp = new CColorProperty(v);
            break;
        }
        case eFileProperty: {
            u32 ResID = SCLY.ReadLong();
            const TStringList& Extensions = static_cast<CFileTemplate*>(pPropTmp)->Extensions();

            CResource *pRes = nullptr;

            for (auto it = Extensions.begin(); it != Extensions.end(); it++)
            {
                const TString& ext = *it;
                if ((ext != "MREA") && (ext != "MLVL")) // Let's avoid recursion please
                    pRes = gResCache.GetResource(ResID, ext);

                if (pRes) break;
            }

            pProp = new CFileProperty(pRes);
            break;
        }
        case eStructProperty: {
            CStructTemplate *StructTmp = pTemp->StructByIndex(iProp);
            pProp = LoadStructMP1(SCLY, StructTmp);
            break;
        }
        case eAnimParamsProperty: {
            pProp = new CAnimParamsProperty(CAnimationParameters(SCLY, mVersion));
            break;
        }
        default:
            pProp = new CUnknownProperty();
            break;
        }

        if (pProp)
        {
            pProp->mpTemplate = pPropTmp;
            propStruct->mProperties.push_back(pProp);
        }
    }

    return propStruct;
}

CScriptObject* CScriptLoader::LoadObjectMP1(CInputStream& SCLY)
{
    u32 objStart = SCLY.Tell();
    u8 type = SCLY.ReadByte();
    u32 size = SCLY.ReadLong();
    u32 end = SCLY.Tell() + size;

    CScriptTemplate *pTemp = mpMaster->TemplateByID((u32) type);
    if (!pTemp)
    {
        // No valid template for this object; can't load
        Log::FileError(SCLY.GetSourceString(), objStart, "Invalid object ID encountered: " + TString::HexString(type));
        SCLY.Seek(end, SEEK_SET);
        return nullptr;
    }

    mpObj = new CScriptObject(mpArea, mpLayer, pTemp);
    mpObj->mInstanceID = SCLY.ReadLong();

    // Load connections
    u32 numLinks = SCLY.ReadLong();
    mpObj->mOutConnections.reserve(numLinks);

    for (u32 iLink = 0; iLink < numLinks; iLink++)
    {
        SLink link;
        link.State = SCLY.ReadLong();
        link.Message = SCLY.ReadLong();
        link.ObjectID = SCLY.ReadLong();
        mpObj->mOutConnections.push_back(link);
    }

    // Load object...
    u32 count = SCLY.PeekLong();
    CStructTemplate *pBase = pTemp->BaseStructByCount(count);

    if (!pBase) {
        Log::Error(pTemp->TemplateName() + " template doesn't match file property count (" + TString::FromInt32(count) + ")");
        pBase = pTemp->BaseStructByIndex(0);
    }
    mpObj->mpProperties = LoadStructMP1(SCLY, pBase);

    // Cleanup and return
    SCLY.Seek(end, SEEK_SET);

    mpObj->EvaluateProperties();
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
    u32 propCount = pTemp->Count();

    if (!pTemp->IsSingleProperty())
    {
        u16 numProperties = SCLY.ReadShort();
        if ((numProperties != propCount) && (mVersion < eReturns))
           Log::FileWarning(SCLY.GetSourceString(), SCLY.Tell() - 2, "Struct \"" + pTemp->Name() + "\" template property count doesn't match file");
        propCount = numProperties;
    }

    // Parse properties
    pStruct->Reserve(propCount);

    for (u32 iProp = 0; iProp < propCount; iProp++)
    {
        CPropertyBase *pProp;
        CPropertyTemplate *pPropTemp;
        u32 propertyStart = SCLY.Tell();
        u32 propertyID = -1;
        u16 PropertyLength = 0;
        u32 NextProperty = 0;

        if (pTemp->IsSingleProperty())
        {
            pProp = pStruct->PropertyByIndex(iProp);
            pPropTemp = pTemp->PropertyByIndex(iProp);
        }
        else
        {
            propertyID = SCLY.ReadLong();
            PropertyLength = SCLY.ReadShort();
            NextProperty = SCLY.Tell() + PropertyLength;

            pProp = pStruct->PropertyByID(propertyID);
            pPropTemp = pTemp->PropertyByID(propertyID);
        }

        if (!pPropTemp)
            Log::FileError(SCLY.GetSourceString(), propertyStart, "Can't find template for property " + TString::HexString(propertyID) + " - skipping");

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

            case eBitfieldProperty: {
                CBitfieldProperty *pBitfieldCast = static_cast<CBitfieldProperty*>(pProp);
                pBitfieldCast->Set(SCLY.ReadLong());

                // Validate
                u32 mask = 0;
                CBitfieldTemplate *pBitfieldTemp = static_cast<CBitfieldTemplate*>(pPropTemp);
                for (u32 iMask = 0; iMask < pBitfieldTemp->NumFlags(); iMask++)
                    mask |= pBitfieldTemp->FlagMask(iMask);

                u32 check = pBitfieldCast->Get() & ~mask;
                if (check != 0) Log::FileWarning(SCLY.GetSourceString(), SCLY.Tell() - 4, "Bitfield property \"" + pBitfieldTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has flags set that aren't in the template: " + TString::HexString(check, true, true, 8));

                break;
            }

            case eEnumProperty: {
                CEnumProperty *pEnumCast = static_cast<CEnumProperty*>(pProp);
                CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pPropTemp);
                u32 ID = SCLY.ReadLong();
                u32 index = pEnumTemp->EnumeratorIndex(ID);
                if (index == -1) Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Enum property \"" + pEnumTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has invalid enumerator value: " + TString::HexString(ID, true, true, 8));
                pEnumCast->Set(index);
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
                pColorCast->Set(CColor(SCLY));
                break;
            }

            case eFileProperty: {
                CFileProperty *pFileCast = static_cast<CFileProperty*>(pProp);

                CUniqueID ResID = (mVersion < eCorruptionProto ? SCLY.ReadLong() : SCLY.ReadLongLong());
                const TStringList& Extensions = static_cast<CFileTemplate*>(pPropTemp)->Extensions();

                CResource *pRes = nullptr;

                // Check for each extension individually until we find a match
                // This could be done better with a function to fetch the extension given the resource ID
                // and a "does resource exist" function, but this will do for now
                bool hasIgnoredExt = false;

                if (ResID.IsValid())
                {
                    for (auto it = Extensions.begin(); it != Extensions.end(); it++)
                    {
                        const TString& ext = *it;

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
                    TString ExtList;
                    for (auto it = Extensions.begin(); it != Extensions.end(); it++)
                    {
                        if (it != Extensions.begin()) ExtList += "/";
                        ExtList += *it;
                    }
                    Log::FileWarning(SCLY.GetSourceString(), "Incorrect resource type? " + ExtList + " " + TString::HexString(propertyID));
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

            case eArrayProperty: {
                CArrayProperty *pArrayCast = static_cast<CArrayProperty*>(pProp);
                std::vector<u8> buf(PropertyLength);
                SCLY.ReadBytes(buf.data(), buf.size());
                pArrayCast->Set(buf);
                break;
            }

            case eAnimParamsProperty: {
                CAnimParamsProperty *pAnimCast = static_cast<CAnimParamsProperty*>(pProp);
                pAnimCast->Set(CAnimationParameters(SCLY, mVersion));
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

    mpObj = new CScriptObject(mpArea, mpLayer, pTemplate);
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
    SCLY.Seek(0x6, SEEK_CUR); // Skip base struct ID + size
    u16 numProps = SCLY.PeekShort();
    mpObj->CopyFromTemplate(pTemplate, (u32) numProps);

    CStructTemplate *pBase = pTemplate->BaseStructByCount(numProps);
    LoadStructMP2(SCLY, mpObj->mpProperties, pBase);

    // Cleanup and return
    SCLY.Seek(ObjEnd, SEEK_SET);
    mpObj->EvaluateProperties();
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP2(CInputStream& SCLY)
{
    CFourCC SCLY_Magic(SCLY);

    if      (SCLY_Magic == "SCLY") SCLY.Seek(0x6, SEEK_CUR);
    else if (SCLY_Magic == "SCGN") SCLY.Seek(0x2, SEEK_CUR);
    else
    {
        Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Invalid script layer magic: " + TString::HexString((u32) SCLY_Magic.ToLong()));
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
