#include "CScriptLoader.h"
#include "CTemplateLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/CResCache.h"
#include "Core/Log.h"
#include <iostream>
#include <sstream>

CScriptLoader::CScriptLoader()
{
    mpObj = nullptr;
}

void CScriptLoader::ReadProperty(IProperty *pProp, u32 Size, IInputStream& SCLY)
{
    IPropertyTemplate *pTemp = pProp->Template();

    switch (pTemp->Type())
    {

    case eBoolProperty: {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(pProp);
        pBoolCast->Set( (SCLY.ReadByte() != 0) );
        break;
    }

    case eByteProperty: {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(pProp);
        pByteCast->Set(SCLY.ReadByte());
        break;
    }

    case eShortProperty: {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(pProp);
        pShortCast->Set(SCLY.ReadShort());
        break;
    }

    case eLongProperty: {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(pProp);
        pLongCast->Set(SCLY.ReadLong());
        break;
    }

    case eBitfieldProperty: {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(pProp);
        pBitfieldCast->Set(SCLY.ReadLong());

        // Validate
        u32 mask = 0;
        CBitfieldTemplate *pBitfieldTemp = static_cast<CBitfieldTemplate*>(pTemp);
        for (u32 iMask = 0; iMask < pBitfieldTemp->NumFlags(); iMask++)
            mask |= pBitfieldTemp->FlagMask(iMask);

        u32 check = pBitfieldCast->Get() & ~mask;
        if (check != 0)
            Log::FileWarning(SCLY.GetSourceString(), SCLY.Tell() - 4, "Bitfield property \"" + pBitfieldTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has flags set that aren't in the template: " + TString::HexString(check, true, true, 8));

        break;
    }

    case eEnumProperty: {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(pProp);
        CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pTemp);
        u32 ID = SCLY.ReadLong();
        u32 index = pEnumTemp->EnumeratorIndex(ID);
        if (index == -1) Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Enum property \"" + pEnumTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has invalid enumerator value: " + TString::HexString(ID, true, true, 8));
        pEnumCast->Set(index);
        break;
    }

    case eFloatProperty: {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(pProp);
        pFloatCast->Set(SCLY.ReadFloat());
        break;
    }

    case eStringProperty: {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(pProp);
        pStringCast->Set(SCLY.ReadString());
        break;
    }

    case eVector3Property: {
        TVector3Property *pVector3Cast = static_cast<TVector3Property*>(pProp);
        pVector3Cast->Set(CVector3f(SCLY));
        break;
    }

    case eColorProperty: {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(pProp);
        pColorCast->Set(CColor(SCLY));
        break;
    }

    case eFileProperty: {
        TFileProperty *pFileCast = static_cast<TFileProperty*>(pProp);

        CUniqueID ResID = (mVersion < eCorruptionProto ? SCLY.ReadLong() : SCLY.ReadLongLong());
        const TStringList& rkExtensions = static_cast<CFileTemplate*>(pTemp)->Extensions();

        CResourceInfo Info(ResID, CFourCC(!rkExtensions.empty() ? rkExtensions.front() : "UNKN"));

        if (ResID.IsValid())
        {
            CFourCC Type = gResCache.FindResourceType(ResID, rkExtensions);
            Info = CResourceInfo(ResID, Type);
        }

        pFileCast->Set(Info);
        break;
    }

    case eStructProperty: {
        CPropertyStruct *pStructCast = static_cast<CPropertyStruct*>(pProp);

        if (mVersion < eEchoesDemo)
            LoadStructMP1(SCLY, pStructCast, static_cast<CStructTemplate*>(pStructCast->Template()));
        else
            LoadStructMP2(SCLY, pStructCast, static_cast<CStructTemplate*>(pTemp));
        break;
    }

    case eArrayProperty: {
        CArrayProperty *pArrayCast = static_cast<CArrayProperty*>(pProp);
        int Size = SCLY.ReadLong();

        pArrayCast->Resize(Size);

        for (int iElem = 0; iElem < Size; iElem++)
        {
            if (mVersion < eEchoesDemo)
                LoadStructMP1(SCLY, static_cast<CPropertyStruct*>(pArrayCast->PropertyByIndex(iElem)), pArrayCast->SubStructTemplate());
            else
                LoadStructMP2(SCLY, static_cast<CPropertyStruct*>(pArrayCast->PropertyByIndex(iElem)), pArrayCast->SubStructTemplate());
        }
        break;
    }

    case eCharacterProperty: {
        TCharacterProperty *pAnimCast = static_cast<TCharacterProperty*>(pProp);
        pAnimCast->Set(CAnimationParameters(SCLY, mVersion));
        break;
    }

    case eUnknownProperty: {
        TUnknownProperty *pUnknownCast = static_cast<TUnknownProperty*>(pProp);
        std::vector<u8> Buffer(Size);
        SCLY.ReadBytes(Buffer.data(), Buffer.size());
        pUnknownCast->Set(Buffer);
        break;
    }
    }
}

void CScriptLoader::LoadStructMP1(IInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp)
{
    u32 StructStart = SCLY.Tell();

    // Verify property count
    u32 PropCount = pTemp->Count();
    u32 Version = 0;

    if (!pTemp->IsSingleProperty())
    {
        u32 FilePropCount = SCLY.ReadLong();
        Version = pTemp->VersionForPropertyCount(FilePropCount);

        if (Version == -1)
        {
            TIDString IDString = pTemp->IDString(true);
            if (!IDString.IsEmpty()) IDString = " (" + IDString + ")";

            Log::FileWarning(SCLY.GetSourceString(), StructStart, "Struct \"" + pTemp->Name() + "\"" + IDString + " template prop count doesn't match file");
        }
    }

    // Parse properties
    for (u32 iProp = 0; iProp < PropCount; iProp++)
    {
        IPropertyTemplate *pPropTemp = pTemp->PropertyByIndex(iProp);
        IProperty *pProp = pStruct->PropertyByIndex(iProp);

        if (pPropTemp->CookPreference() != eNeverCook && pPropTemp->IsInVersion(Version))
            ReadProperty(pProp, 0, SCLY);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP1(IInputStream& SCLY)
{
    u32 ObjStart = SCLY.Tell();
    u8 Type = SCLY.ReadByte();
    u32 Size = SCLY.ReadLong();
    u32 End = SCLY.Tell() + Size;

    CScriptTemplate *pTemp = mpMaster->TemplateByID((u32) Type);
    if (!pTemp)
    {
        // No valid template for this object; can't load
        Log::FileError(SCLY.GetSourceString(), ObjStart, "Invalid object ID encountered: " + TString::HexString(Type));
        SCLY.Seek(End, SEEK_SET);
        return nullptr;
    }

    mpObj = new CScriptObject(mpArea, mpLayer, pTemp);
    mpObj->mInstanceID = SCLY.ReadLong();

    // Load connections
    u32 NumLinks = SCLY.ReadLong();
    mpObj->mOutConnections.reserve(NumLinks);

    for (u32 iLink = 0; iLink < NumLinks; iLink++)
    {
        SLink Link;
        Link.State = SCLY.ReadLong();
        Link.Message = SCLY.ReadLong();
        Link.ObjectID = SCLY.ReadLong();
        mpObj->mOutConnections.push_back(Link);
    }

    // Load object...
    CPropertyStruct *pBase = mpObj->mpProperties;
    LoadStructMP1(SCLY, pBase, static_cast<CStructTemplate*>(pBase->Template()));

    // Cleanup and return
    SCLY.Seek(End, SEEK_SET);

    mpObj->EvaluateProperties();
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP1(IInputStream &SCLY)
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
            mpLayer->AddInstance(pObj);
    }

    // Layer sizes are always a multiple of 32 - skip end padding before returning
    u32 Remaining = 32 - ((SCLY.Tell() - LayerStart) & 0x1F);
    SCLY.Seek(Remaining, SEEK_CUR);
    return mpLayer;
}

void CScriptLoader::LoadStructMP2(IInputStream& SCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp)
{
    // Verify property count
    u32 StructStart = SCLY.Tell();
    StructStart += 0;
    u32 PropCount = pTemp->Count();
    u32 Version = 0;

    if (!pTemp->IsSingleProperty())
    {
        u16 NumProperties = SCLY.ReadShort();
        Version = pTemp->VersionForPropertyCount(NumProperties);

        if ((NumProperties != PropCount) && (mVersion < eReturns))
        {
            TIDString IDString = pTemp->IDString(true);
            if (!IDString.IsEmpty()) IDString = " (" + IDString + ")";

            Log::FileWarning(SCLY.GetSourceString(), StructStart, "Struct \"" + pTemp->Name() + "\"" + IDString + " template prop count doesn't match file");
        }

        PropCount = NumProperties;
    }

    // Parse properties
    for (u32 iProp = 0; iProp < PropCount; iProp++)
    {
        IProperty *pProp;
        IPropertyTemplate *pPropTemp;
        u32 PropertyStart = SCLY.Tell();
        u32 PropertyID = -1;
        u16 PropertyLength = 0;
        u32 NextProperty = 0;

        if (pTemp->IsSingleProperty())
        {
            pProp = pStruct->PropertyByIndex(iProp);
            pPropTemp = pTemp->PropertyByIndex(iProp);
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
            Log::FileError(SCLY.GetSourceString(), PropertyStart, "Can't find template for property " + TString::HexString(PropertyID) + " - skipping");

        else
            ReadProperty(pProp, PropertyLength, SCLY);

        if (NextProperty > 0)
            SCLY.Seek(NextProperty, SEEK_SET);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP2(IInputStream& SCLY)
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
    mpObj->mInstanceID = SCLY.ReadLong();

    // Load connections
    u32 NumConnections = SCLY.ReadShort();
    mpObj->mOutConnections.reserve(NumConnections);

    for (u32 iCon = 0; iCon < NumConnections; iCon++)
    {
        SLink Link;
        Link.State = SCLY.ReadLong();
        Link.Message = SCLY.ReadLong();
        Link.ObjectID = SCLY.ReadLong();
        mpObj->mOutConnections.push_back(Link);
    }

    // Load object
    SCLY.Seek(0x6, SEEK_CUR); // Skip base struct ID + size
    LoadStructMP2(SCLY, mpObj->mpProperties, mpObj->mpTemplate->BaseStruct());

    // Cleanup and return
    SCLY.Seek(ObjEnd, SEEK_SET);
    mpObj->EvaluateProperties();
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP2(IInputStream& SCLY)
{
    bool IsSCGN = false;

    if (mVersion >= eEchoes)
    {
        CFourCC SCLY_Magic(SCLY);

        if (SCLY_Magic == "SCLY")
        {
            SCLY.Seek(0x6, SEEK_CUR);
        }
        else if (SCLY_Magic == "SCGN")
        {
            SCLY.Seek(0x2, SEEK_CUR);
            IsSCGN = true;
        }
        else
        {
            Log::FileError(SCLY.GetSourceString(), SCLY.Tell() - 4, "Invalid script layer magic: " + TString::HexString((u32) SCLY_Magic.ToLong()));
            return nullptr;
        }
    }
    else
    {
        SCLY.Seek(0x1, SEEK_CUR);
    }

    u32 NumObjects = SCLY.ReadLong();

    mpLayer = new CScriptLayer();
    mpLayer->Reserve(NumObjects);

    for (u32 iObj = 0; iObj < NumObjects; iObj++)
    {
        CScriptObject *pObj = LoadObjectMP2(SCLY);
        if (pObj)
            mpLayer->AddInstance(pObj);
    }

    if (IsSCGN)
    {
        mpLayer->SetName("Generated");
        mpLayer->SetActive(true);
    }
    return mpLayer;
}

CScriptLayer* CScriptLoader::LoadLayer(IInputStream &SCLY, CGameArea *pArea, EGame Version)
{
    if (!SCLY.IsValid()) return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = Version;
    Loader.mpMaster = CMasterTemplate::GetMasterForGame(Version);
    Loader.mpArea = pArea;

    if (!Loader.mpMaster)
    {
        Log::Write("This game doesn't have a master template; couldn't load script layer");
        return nullptr;
    }

    if (!Loader.mpMaster->IsLoadedSuccessfully())
        CTemplateLoader::LoadGameTemplates(Version);

    if (Version <= ePrime)
        return Loader.LoadLayerMP1(SCLY);
    else
        return Loader.LoadLayerMP2(SCLY);
}
