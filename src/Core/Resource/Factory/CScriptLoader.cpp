#include "CScriptLoader.h"
#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/NGameList.h"
#include "Core/Resource/Script/Property/CArrayProperty.h"
#include "Core/Resource/Script/Property/CAssetProperty.h"
#include "Core/Resource/Script/Property/CEnumProperty.h"
#include "Core/Resource/Script/Property/CFlagsProperty.h"
#include <Common/Log.h>
#include <iostream>
#include <sstream>

// Whether to ensure the values of enum/flag properties are valid
#define VALIDATE_PROPERTY_VALUES 1

CScriptLoader::CScriptLoader()
    : mpObj(nullptr)
    , mpArrayItemData(nullptr)
{
}

void CScriptLoader::ReadProperty(IProperty *pProp, u32 Size, IInputStream& rSCLY)
{
    void* pData = (mpArrayItemData ? mpArrayItemData : mpObj->mPropertyData.data());

    switch (pProp->Type())
    {

    case EPropertyType::Bool:
    {
        CBoolProperty* pBool = TPropCast<CBoolProperty>(pProp);
        pBool->ValueRef(pData) = rSCLY.ReadBool();
        break;
    }

    case EPropertyType::Byte:
    {
        CByteProperty* pByte = TPropCast<CByteProperty>(pProp);
        pByte->ValueRef(pData) = rSCLY.ReadByte();
        break;
    }

    case EPropertyType::Short:
    {
        CShortProperty* pShort = TPropCast<CShortProperty>(pProp);
        pShort->ValueRef(pData) = rSCLY.ReadShort();
        break;
    }

    case EPropertyType::Int:
    {
        CIntProperty* pInt = TPropCast<CIntProperty>(pProp);
        pInt->ValueRef(pData) = rSCLY.ReadLong();
        break;
    }

    case EPropertyType::Float:
    {
        CFloatProperty* pFloat = TPropCast<CFloatProperty>(pProp);
        pFloat->ValueRef(pData) = rSCLY.ReadFloat();
        break;
    }

    case EPropertyType::Choice:
    {
        CChoiceProperty* pChoice = TPropCast<CChoiceProperty>(pProp);
        pChoice->ValueRef(pData) = rSCLY.ReadLong();

#if VALIDATE_PROPERTY_VALUES
        if (!pChoice->HasValidValue(pData))
        {
            u32 Value = pChoice->ValueRef(pData);
            Log::FileError(rSCLY.GetSourceString(), rSCLY.Tell() - 4, "Choice property \"" + pChoice->Name() + "\" (" + pChoice->IDString(true) + ") has unrecognized value: " + TString::HexString(Value));
        }
#endif
        break;
    }

    case EPropertyType::Enum:
    {
        CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);
        pEnum->ValueRef(pData) = rSCLY.ReadLong();

#if VALIDATE_PROPERTY_VALUES
        if (!pEnum->HasValidValue(pData))
        {
            u32 Value = pEnum->ValueRef(pData);
            Log::FileError(rSCLY.GetSourceString(), rSCLY.Tell() - 4, "Enum property \"" + pEnum->Name() + "\" (" + pEnum->IDString(true) + ") has unrecognized value: " + TString::HexString(Value));
        }
#endif
        break;
    }

    case EPropertyType::Flags:
    {
        CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);
        pFlags->ValueRef(pData) = rSCLY.ReadLong();

#if VALIDATE_PROPERTY_VALUES
        u32 InvalidBits = pFlags->HasValidValue(pData);

        if (InvalidBits)
        {
            Log::FileWarning(rSCLY.GetSourceString(), rSCLY.Tell() - 4, "Flags property \"" + pFlags->Name() + "\" + (" + pFlags->IDString(true) + ") has unrecognized flags set: " + TString::HexString(InvalidBits));
        }
#endif
        break;
    }

    case EPropertyType::String:
    {
        CStringProperty* pString = TPropCast<CStringProperty>(pProp);
        pString->ValueRef(pData) = rSCLY.ReadString();
        break;
    }

    case EPropertyType::Vector:
    {
        CVectorProperty* pVector = TPropCast<CVectorProperty>(pProp);
        pVector->ValueRef(pData) = CVector3f(rSCLY);
        break;
    }

    case EPropertyType::Color:
    {
        CColorProperty* pColor = TPropCast<CColorProperty>(pProp);
        pColor->ValueRef(pData) = CColor(rSCLY);
        break;
    }

    case EPropertyType::Asset:
    {
        CAssetProperty* pAsset = TPropCast<CAssetProperty>(pProp);
        pAsset->ValueRef(pData) = CAssetID(rSCLY, mpGameTemplate->Game());

#if VALIDATE_PROPERTY_VALUES
        CAssetID ID = pAsset->ValueRef(pData);

        if (ID.IsValid())
        {
            CResourceEntry *pEntry = gpResourceStore->FindEntry(ID);

            if (pEntry)
            {
                const CResTypeFilter& rkFilter = pAsset->GetTypeFilter();
                bool Valid = rkFilter.Accepts(pEntry->ResourceType());

                if (!Valid)
                    Log::FileWarning(rSCLY.GetSourceString(), rSCLY.Tell() - ID.Length(), "Asset property \"" + pAsset->Name() + "\" (" + pAsset->IDString(true) + ") has a reference to an illegal asset type: " + pEntry->CookedExtension());
            }
        }
#endif
        break;
    }

    case EPropertyType::Sound:
    {
        CSoundProperty* pSound = TPropCast<CSoundProperty>(pProp);
        pSound->ValueRef(pData) = rSCLY.ReadLong();
        break;
    }

    case EPropertyType::Animation:
    {
        CAnimationProperty* pAnim = TPropCast<CAnimationProperty>(pProp);
        pAnim->ValueRef(pData) = rSCLY.ReadLong();
        break;
    }

    case EPropertyType::AnimationSet:
    {
        CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProp);
        pAnimSet->ValueRef(pData) = CAnimationParameters(rSCLY, mpGameTemplate->Game());
        break;
    }

    case EPropertyType::Sequence:
    {
        // TODO
        break;
    }

    case EPropertyType::Spline:
    {
        CSplineProperty* pSpline = TPropCast<CSplineProperty>(pProp);
        std::vector<char>& Buffer = pSpline->ValueRef(pData);
        Buffer.resize(Size);
        rSCLY.ReadBytes(Buffer.data(), Buffer.size());
        break;
    }

    case EPropertyType::Guid:
    {
        ASSERT(Size == 16);
        CGuidProperty* pGuid = TPropCast<CGuidProperty>(pProp);
        pGuid->ValueRef(pData).resize(16);
        rSCLY.ReadBytes(pGuid->ValueRef(pData).data(), 16);
        break;
    }

    case EPropertyType::Struct:
    {
        CStructProperty* pStruct = TPropCast<CStructProperty>(pProp);

        if (mVersion < EGame::EchoesDemo)
            LoadStructMP1(rSCLY, pStruct);
        else
            LoadStructMP2(rSCLY, pStruct);
        break;
    }

    case EPropertyType::Array:
    {
        CArrayProperty *pArray = TPropCast<CArrayProperty>(pProp);
        int Count = rSCLY.ReadLong();

        pArray->Resize(pData, Count);
        void* pOldArrayItemData = mpArrayItemData;

        // Make sure the array archetype is atomic... non-atomic array archetypes is not supported
        // because arrays can only have one possible archetype so having property IDs here wouldn't make sense
        ASSERT(pArray->ItemArchetype()->IsAtomic());

        for (int ElementIdx = 0; ElementIdx < Count; ElementIdx++)
        {
            /**
             * so this is kind of annoying, there isn't really any good way to cleanly integrate arrays into the property system
             * because calculating the pointer to an item requires knowing the array index, which the property itself can't store
             * because the same property object is used for every array element; and we can't dynamically add children to the array
             * based on its size either, because the same array property is shared between multiple script instances. so, instead,
             * we determine the item pointer ourselves and the array archetype property will respect it.
             *
             * arrays are an edge case anyway - they only really appear in Prime 1 and there are only a couple array properties in
             * the game. the only situation where an array property appears in other games is SequenceTimer, and that's going to be
             * migrated to Sequence properties eventually, so there isn't really any good reason to spend a lot of effort refactoring
             * things to make this cleaner
             */
            mpArrayItemData = pArray->ItemPointer(pData, ElementIdx);
            ReadProperty(pArray->ItemArchetype(), 0, rSCLY);
        }

        mpArrayItemData = pOldArrayItemData;
        break;
    }

    }
}

void CScriptLoader::LoadStructMP1(IInputStream& rSCLY, CStructProperty* pStruct)
{
    u32 StructStart = rSCLY.Tell();

    // Verify property count
    u32 PropertyCount = pStruct->NumChildren();
    u32 Version = 0;

    if (!pStruct->IsAtomic())
    {
        u32 FilePropCount = rSCLY.ReadLong();
        //@todo version checking
    }

    // Parse properties
    for (u32 ChildIndex = 0; ChildIndex < PropertyCount; ChildIndex++)
    {
        IProperty *pProperty = pStruct->ChildByIndex(ChildIndex);

        //@todo version check
        if (pProperty->CookPreference() != ECookPreference::Never)
            ReadProperty(pProperty, 0, rSCLY);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP1(IInputStream& rSCLY)
{
    u32 StartOffset = rSCLY.Tell();
    u8 Type = rSCLY.ReadByte();
    u32 Size = rSCLY.ReadLong();
    u32 End = rSCLY.Tell() + Size;

    CScriptTemplate *pTemplate = mpGameTemplate->TemplateByID((u32) Type);
    if (!pTemplate)
    {
        // No valid template for this object; can't load
        Log::FileError(rSCLY.GetSourceString(), StartOffset, "Unknown object ID encountered: " + TString::HexString(Type, 2));
        rSCLY.Seek(End, SEEK_SET);
        return nullptr;
    }

    u32 InstanceID = rSCLY.ReadLong() & 0x03FFFFFF;
    if (InstanceID == 0x03FFFFFF) InstanceID = mpArea->FindUnusedInstanceID();
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemplate);

    // Load connections
    u32 NumLinks = rSCLY.ReadLong();
    mpObj->mOutLinks.reserve(NumLinks);

    for (u32 iLink = 0; iLink < NumLinks; iLink++)
    {
        u32 State = rSCLY.ReadLong();
        u32 Message = rSCLY.ReadLong();
        u32 ReceiverID = rSCLY.ReadLong() & 0x03FFFFFF;

        CLink *pLink = new CLink(mpArea, State, Message, mpObj->mInstanceID, ReceiverID);
        mpObj->mOutLinks.push_back(pLink);
    }

    // Load object...
    CStructProperty* pProperties = pTemplate->Properties();
    LoadStructMP1(rSCLY, pProperties);

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

    for (u32 ObjectIndex = 0; ObjectIndex < NumObjects; ObjectIndex++)
    {
        CScriptObject *pObject = LoadObjectMP1(rSCLY);
        if (pObject)
            mpLayer->AddInstance(pObject);
    }

    // Layer sizes are always a multiple of 32 - skip end padding before returning
    u32 Remaining = 32 - ((rSCLY.Tell() - LayerStart) & 0x1F);
    rSCLY.Seek(Remaining, SEEK_CUR);
    return mpLayer;
}

void CScriptLoader::LoadStructMP2(IInputStream& rSCLY, CStructProperty* pStruct)
{
    // Verify property count
    u32 ChildCount = pStruct->NumChildren();

    if (!pStruct->IsAtomic())
        ChildCount = rSCLY.ReadShort();

    // Parse properties
    for (u32 ChildIdx = 0; ChildIdx < ChildCount; ChildIdx++)
    {
        IProperty* pProperty = nullptr;
        u32 PropertyStart = rSCLY.Tell();
        u32 PropertyID = -1;
        u16 PropertySize = 0;
        u32 NextProperty = 0;

        if (pStruct->IsAtomic())
        {
            pProperty = pStruct->ChildByIndex(ChildIdx);
        }
        else
        {
            PropertyID = rSCLY.ReadLong();
            PropertySize = rSCLY.ReadShort();
            NextProperty = rSCLY.Tell() + PropertySize;
            pProperty = pStruct->ChildByID(PropertyID);
        }

        if (!pProperty)
            Log::FileError(rSCLY.GetSourceString(), PropertyStart, "Can't find template for property " + TString::HexString(PropertyID) + " - skipping");
        else
            ReadProperty(pProperty, PropertySize, rSCLY);

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

    CScriptTemplate* pTemplate = mpGameTemplate->TemplateByID(ObjectID);

    if (!pTemplate)
    {
        Log::FileError(rSCLY.GetSourceString(), ObjStart, "Unknown object ID encountered: " + CFourCC(ObjectID).ToString());
        rSCLY.Seek(ObjEnd, SEEK_SET);
        return nullptr;
    }

    u32 InstanceID = rSCLY.ReadLong() & 0x03FFFFFF;
    if (InstanceID == 0x03FFFFFF) InstanceID = mpArea->FindUnusedInstanceID();
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemplate);

    // Load connections
    u32 NumConnections = rSCLY.ReadShort();
    mpObj->mOutLinks.reserve(NumConnections);

    for (u32 LinkIdx = 0; LinkIdx < NumConnections; LinkIdx++)
    {
        u32 State = rSCLY.ReadLong();
        u32 Message = rSCLY.ReadLong();
        u32 ReceiverID = rSCLY.ReadLong() & 0x03FFFFFF;

        CLink* pLink = new CLink(mpArea, State, Message, mpObj->mInstanceID, ReceiverID);
        mpObj->mOutLinks.push_back(pLink);
    }

    // Load object
    rSCLY.Seek(0x6, SEEK_CUR); // Skip base struct ID + size
    LoadStructMP2(rSCLY, pTemplate->Properties());

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

    for (u32 ObjectIdx = 0; ObjectIdx < NumObjects; ObjectIdx++)
    {
        CScriptObject* pObject = LoadObjectMP2(rSCLY);
        if (pObject)
            mpLayer->AddInstance(pObject);
    }

    return mpLayer;
}

// ************ STATIC ************
CScriptLayer* CScriptLoader::LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version)
{
    if (!rSCLY.IsValid()) return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = Version;
    Loader.mpGameTemplate = NGameList::GetGameTemplate(Version);
    Loader.mpArea = pArea;

    if (!Loader.mpGameTemplate)
    {
        Log::Write("This game doesn't have a game template; couldn't load script layer");
        return nullptr;
    }

    if (Version <= EGame::Prime)
        return Loader.LoadLayerMP1(rSCLY);
    else
        return Loader.LoadLayerMP2(rSCLY);
}

CScriptObject* CScriptLoader::LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat)
{
    if (!rSCLY.IsValid()) return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = (ForceReturnsFormat ? EGame::DKCReturns : Version);
    Loader.mpGameTemplate = NGameList::GetGameTemplate(Version);
    Loader.mpArea = pArea;
    Loader.mpLayer = pLayer;

    if (!Loader.mpGameTemplate)
    {
        Log::Write("This game doesn't have a game template; couldn't load script instance");
        return nullptr;
    }

    if (Loader.mVersion <= EGame::Prime)
        return Loader.LoadObjectMP1(rSCLY);
    else
        return Loader.LoadObjectMP2(rSCLY);
}
