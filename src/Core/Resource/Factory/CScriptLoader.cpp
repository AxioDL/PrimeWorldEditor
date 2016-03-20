#include "CScriptLoader.h"
#include "CTemplateLoader.h"
#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/CResCache.h"
#include <Common/Log.h>
#include <iostream>
#include <sstream>

CScriptLoader::CScriptLoader()
{
    mpObj = nullptr;
}

void CScriptLoader::ReadProperty(IProperty *pProp, u32 Size, IInputStream& rSCLY)
{
    IPropertyTemplate *pTemp = pProp->Template();

    switch (pTemp->Type())
    {

    case eBoolProperty: {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(pProp);
        pBoolCast->Set( (rSCLY.ReadByte() != 0) );
        break;
    }

    case eByteProperty: {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(pProp);
        pByteCast->Set(rSCLY.ReadByte());
        break;
    }

    case eShortProperty: {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(pProp);
        pShortCast->Set(rSCLY.ReadShort());
        break;
    }

    case eLongProperty: {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(pProp);
        pLongCast->Set(rSCLY.ReadLong());
        break;
    }

    case eBitfieldProperty: {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(pProp);
        pBitfieldCast->Set(rSCLY.ReadLong());

        // Validate
        u32 mask = 0;
        CBitfieldTemplate *pBitfieldTemp = static_cast<CBitfieldTemplate*>(pTemp);
        for (u32 iMask = 0; iMask < pBitfieldTemp->NumFlags(); iMask++)
            mask |= pBitfieldTemp->FlagMask(iMask);

        u32 check = pBitfieldCast->Get() & ~mask;
        if (check != 0)
            Log::FileWarning(rSCLY.GetSourceString(), rSCLY.Tell() - 4, "Bitfield property \"" + pBitfieldTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has flags set that aren't in the template: " + TString::HexString(check, true, true, 8));

        break;
    }

    case eEnumProperty: {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(pProp);
        CEnumTemplate *pEnumTemp = static_cast<CEnumTemplate*>(pTemp);
        u32 ID = rSCLY.ReadLong();

        // Validate
        u32 Index = pEnumTemp->EnumeratorIndex(ID);
        if (Index == -1) Log::FileError(rSCLY.GetSourceString(), rSCLY.Tell() - 4, "Enum property \"" + pEnumTemp->Name() + "\" in struct \"" + pTemp->Name() + "\" has invalid enumerator value: " + TString::HexString(ID, true, true, 8));

        pEnumCast->Set(ID);
        break;
    }

    case eFloatProperty: {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(pProp);
        pFloatCast->Set(rSCLY.ReadFloat());
        break;
    }

    case eStringProperty: {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(pProp);
        pStringCast->Set(rSCLY.ReadString());
        break;
    }

    case eVector3Property: {
        TVector3Property *pVector3Cast = static_cast<TVector3Property*>(pProp);
        pVector3Cast->Set(CVector3f(rSCLY));
        break;
    }

    case eColorProperty: {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(pProp);
        pColorCast->Set(CColor(rSCLY));
        break;
    }

    case eFileProperty: {
        TFileProperty *pFileCast = static_cast<TFileProperty*>(pProp);

        CUniqueID ResID = (mVersion < eCorruptionProto ? rSCLY.ReadLong() : rSCLY.ReadLongLong());
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
            LoadStructMP1(rSCLY, pStructCast, static_cast<CStructTemplate*>(pStructCast->Template()));
        else
            LoadStructMP2(rSCLY, pStructCast, static_cast<CStructTemplate*>(pTemp));
        break;
    }

    case eArrayProperty: {
        CArrayProperty *pArrayCast = static_cast<CArrayProperty*>(pProp);
        int Count = rSCLY.ReadLong();

        pArrayCast->Resize(Count);

        for (int iElem = 0; iElem < Count; iElem++)
        {
            if (mVersion < eEchoesDemo)
                LoadStructMP1(rSCLY, static_cast<CPropertyStruct*>(pArrayCast->PropertyByIndex(iElem)), pArrayCast->SubStructTemplate());
            else
                LoadStructMP2(rSCLY, static_cast<CPropertyStruct*>(pArrayCast->PropertyByIndex(iElem)), pArrayCast->SubStructTemplate());
        }
        break;
    }

    case eCharacterProperty: {
        TCharacterProperty *pAnimCast = static_cast<TCharacterProperty*>(pProp);
        pAnimCast->Set(CAnimationParameters(rSCLY, mpMaster->GetGame()));
        break;
    }

    case eMayaSplineProperty: {
        TMayaSplineProperty *pSplineCast = static_cast<TMayaSplineProperty*>(pProp);
        std::vector<u8> Buffer(Size);
        rSCLY.ReadBytes(Buffer.data(), Buffer.size());
        pSplineCast->Set(Buffer);
        break;
    }

    case eUnknownProperty: {
        TUnknownProperty *pUnknownCast = static_cast<TUnknownProperty*>(pProp);
        std::vector<u8> Buffer(Size);
        rSCLY.ReadBytes(Buffer.data(), Buffer.size());
        pUnknownCast->Set(Buffer);
        break;
    }
    }
}

void CScriptLoader::LoadStructMP1(IInputStream& rSCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp)
{
    u32 StructStart = rSCLY.Tell();

    // Verify property count
    u32 PropCount = pTemp->Count();
    u32 Version = 0;

    if (!pTemp->IsSingleProperty())
    {
        u32 FilePropCount = rSCLY.ReadLong();
        Version = pTemp->VersionForPropertyCount(FilePropCount);

        if (Version == -1)
        {
            TIDString IDString = pTemp->IDString(true);
            if (!IDString.IsEmpty()) IDString = " (" + IDString + ")";

            Log::FileWarning(rSCLY.GetSourceString(), StructStart, "Struct \"" + pTemp->Name() + "\"" + IDString + " template prop count doesn't match file; template is " + TString::HexString(PropCount, true, true, 2) + ", file is " + TString::HexString(FilePropCount, true, true, 2));
            Version = 0;
        }
    }

    // Parse properties
    for (u32 iProp = 0; iProp < PropCount; iProp++)
    {
        IPropertyTemplate *pPropTemp = pTemp->PropertyByIndex(iProp);
        IProperty *pProp = pStruct->PropertyByIndex(iProp);

        if (pPropTemp->CookPreference() != eNeverCook && pPropTemp->IsInVersion(Version))
            ReadProperty(pProp, 0, rSCLY);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP1(IInputStream& rSCLY)
{
    u32 ObjStart = rSCLY.Tell();
    u8 Type = rSCLY.ReadByte();
    u32 Size = rSCLY.ReadLong();
    u32 End = rSCLY.Tell() + Size;

    CScriptTemplate *pTemp = mpMaster->TemplateByID((u32) Type);
    if (!pTemp)
    {
        // No valid template for this object; can't load
        Log::FileError(rSCLY.GetSourceString(), ObjStart, "Unknown object ID encountered: " + TString::HexString(Type));
        rSCLY.Seek(End, SEEK_SET);
        return nullptr;
    }

    u32 InstanceID = rSCLY.ReadLong();
    if (InstanceID == -1) InstanceID = mpArea->FindUnusedInstanceID(mpLayer);
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemp);

    // Load connections
    u32 NumLinks = rSCLY.ReadLong();
    mpObj->mOutLinks.reserve(NumLinks);

    for (u32 iLink = 0; iLink < NumLinks; iLink++)
    {
        u32 State = rSCLY.ReadLong();
        u32 Message = rSCLY.ReadLong();
        u32 ReceiverID = rSCLY.ReadLong();

        CLink *pLink = new CLink(mpArea, State, Message, mpObj->mInstanceID, ReceiverID);
        mpObj->mOutLinks.push_back(pLink);
    }

    // Load object...
    CPropertyStruct *pBase = mpObj->mpProperties;
    LoadStructMP1(rSCLY, pBase, static_cast<CStructTemplate*>(pBase->Template()));

    // Cleanup and return
    rSCLY.Seek(End, SEEK_SET);

    mpObj->EvaluateProperties();
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP1(IInputStream& rSCLY)
{
    u32 LayerStart = rSCLY.Tell();

    rSCLY.Seek(0x1, SEEK_CUR); // One unknown byte at the start of each layer
    u32 NumObjects = rSCLY.ReadLong();

    mpLayer = new CScriptLayer(mpArea);
    mpLayer->Reserve(NumObjects);

    for (u32 iObj = 0; iObj < NumObjects; iObj++)
    {
        CScriptObject *pObj = LoadObjectMP1(rSCLY);
        if (pObj)
            mpLayer->AddInstance(pObj);
    }

    // Layer sizes are always a multiple of 32 - skip end padding before returning
    u32 Remaining = 32 - ((rSCLY.Tell() - LayerStart) & 0x1F);
    rSCLY.Seek(Remaining, SEEK_CUR);
    return mpLayer;
}

void CScriptLoader::LoadStructMP2(IInputStream& rSCLY, CPropertyStruct *pStruct, CStructTemplate *pTemp)
{
    // Verify property count
    u32 StructStart = rSCLY.Tell();
    StructStart += 0;
    u32 PropCount = pTemp->Count();

    if (!pTemp->IsSingleProperty())
        PropCount = rSCLY.ReadShort();

    // Parse properties
    for (u32 iProp = 0; iProp < PropCount; iProp++)
    {
        IProperty *pProp;
        IPropertyTemplate *pPropTemp;
        u32 PropertyStart = rSCLY.Tell();
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
            PropertyID = rSCLY.ReadLong();
            PropertyLength = rSCLY.ReadShort();
            NextProperty = rSCLY.Tell() + PropertyLength;

            pProp = pStruct->PropertyByID(PropertyID);
            pPropTemp = pTemp->PropertyByID(PropertyID);
        }

        if (!pPropTemp)
            Log::FileError(rSCLY.GetSourceString(), PropertyStart, "Can't find template for property " + TString::HexString(PropertyID) + " - skipping");

        else
            ReadProperty(pProp, PropertyLength, rSCLY);

        if (NextProperty > 0)
            rSCLY.Seek(NextProperty, SEEK_SET);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP2(IInputStream& rSCLY)
{
    u32 ObjStart = rSCLY.Tell();
    u32 ObjectID = rSCLY.ReadLong();
    u16 ObjectSize = rSCLY.ReadShort();
    u32 ObjEnd = rSCLY.Tell() + ObjectSize;

    CScriptTemplate *pTemplate = mpMaster->TemplateByID(ObjectID);

    if (!pTemplate)
    {
        Log::FileError(rSCLY.GetSourceString(), ObjStart, "Unknown object ID encountered: " + CFourCC(ObjectID).ToString());
        rSCLY.Seek(ObjEnd, SEEK_SET);
        return nullptr;
    }

    u32 InstanceID = rSCLY.ReadLong();
    if (InstanceID == -1) InstanceID = mpArea->FindUnusedInstanceID(mpLayer);
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemplate);

    // Load connections
    u32 NumConnections = rSCLY.ReadShort();
    mpObj->mOutLinks.reserve(NumConnections);

    for (u32 iCon = 0; iCon < NumConnections; iCon++)
    {
        u32 State = rSCLY.ReadLong();
        u32 Message = rSCLY.ReadLong();
        u32 ReceiverID = rSCLY.ReadLong();

        CLink *pLink = new CLink(mpArea, State, Message, mpObj->mInstanceID, ReceiverID);
        mpObj->mOutLinks.push_back(pLink);
    }

    // Load object
    rSCLY.Seek(0x6, SEEK_CUR); // Skip base struct ID + size
    LoadStructMP2(rSCLY, mpObj->mpProperties, mpObj->mpTemplate->BaseStruct());

    // Cleanup and return
    rSCLY.Seek(ObjEnd, SEEK_SET);
    mpObj->EvaluateProperties();
    return mpObj;
}

CScriptLayer* CScriptLoader::LoadLayerMP2(IInputStream& rSCLY)
{
    rSCLY.Seek(0x1, SEEK_CUR); // Skipping version. todo: verify this?
    u32 NumObjects = rSCLY.ReadLong();

    mpLayer = new CScriptLayer(mpArea);
    mpLayer->Reserve(NumObjects);

    for (u32 iObj = 0; iObj < NumObjects; iObj++)
    {
        CScriptObject *pObj = LoadObjectMP2(rSCLY);
        if (pObj)
            mpLayer->AddInstance(pObj);
    }

    return mpLayer;
}

// ************ STATIC ************
CScriptLayer* CScriptLoader::LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version)
{
    if (!rSCLY.IsValid()) return nullptr;

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
        return Loader.LoadLayerMP1(rSCLY);
    else
        return Loader.LoadLayerMP2(rSCLY);
}

CScriptObject* CScriptLoader::LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat)
{
    if (!rSCLY.IsValid()) return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = (ForceReturnsFormat ? eReturns : Version);
    Loader.mpMaster = CMasterTemplate::GetMasterForGame(Version);
    Loader.mpArea = pArea;
    Loader.mpLayer = pLayer;

    if (!Loader.mpMaster)
    {
        Log::Write("This game doesn't have a master template; couldn't load script instance");
        return nullptr;
    }

    if (!Loader.mpMaster->IsLoadedSuccessfully())
        CTemplateLoader::LoadGameTemplates(Version);

    if (Loader.mVersion <= ePrime)
        return Loader.LoadObjectMP1(rSCLY);
    else
        return Loader.LoadObjectMP2(rSCLY);
}
