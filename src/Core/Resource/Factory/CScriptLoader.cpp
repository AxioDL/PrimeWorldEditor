#include "Core/Resource/Factory/CScriptLoader.h"

#include "Core/GameProject/CResourceStore.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/NGameList.h"
#include "Core/Resource/Script/Property/CArrayProperty.h"
#include "Core/Resource/Script/Property/CAssetProperty.h"
#include "Core/Resource/Script/Property/CEnumProperty.h"
#include "Core/Resource/Script/Property/CFlagsProperty.h"

#include <Common/Log.h>

// Whether to ensure the values of enum/flag properties are valid
#define VALIDATE_PROPERTY_VALUES 1

CScriptLoader::CScriptLoader() = default;

void CScriptLoader::ReadProperty(IProperty *pProp, uint32_t Size, IInputStream& rSCLY)
{
    void* pData = (mpCurrentData ? mpCurrentData : mpObj->mPropertyData.data());

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
        pByte->ValueRef(pData) = rSCLY.ReadS8();
        break;
    }

    case EPropertyType::Short:
    {
        CShortProperty* pShort = TPropCast<CShortProperty>(pProp);
        pShort->ValueRef(pData) = rSCLY.ReadS16();
        break;
    }

    case EPropertyType::Int:
    {
        CIntProperty* pInt = TPropCast<CIntProperty>(pProp);
        pInt->ValueRef(pData) = rSCLY.ReadS32();
        break;
    }

    case EPropertyType::Float:
    {
        CFloatProperty* pFloat = TPropCast<CFloatProperty>(pProp);
        pFloat->ValueRef(pData) = rSCLY.ReadF32();
        break;
    }

    case EPropertyType::Choice:
    {
        CChoiceProperty* pChoice = TPropCast<CChoiceProperty>(pProp);
        pChoice->ValueRef(pData) = rSCLY.ReadS32();

#if VALIDATE_PROPERTY_VALUES
        if (!pChoice->HasValidValue(pData))
        {
            const auto Value = pChoice->ValueRef(pData);
            NLog::Error("{} [0x{:X}]: Choice property \"{}\" ({}) has unrecognized value: 0x{:08X}",
                        rSCLY.GetSourceString(),
                        rSCLY.Tell() - 4,
                        pChoice->Name(),
                        pChoice->IDString(true),
                        Value);
        }
#endif
        break;
    }

    case EPropertyType::Enum:
    {
        CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProp);
        pEnum->ValueRef(pData) = rSCLY.ReadS32();

#if VALIDATE_PROPERTY_VALUES
        if (!pEnum->HasValidValue(pData))
        {
            const auto Value = pEnum->ValueRef(pData);
            NLog::Error("{} [0x{:X}]: Enum property \"{}\" ({}) has unrecognized value: 0x{:08X}",
                        rSCLY.GetSourceString(),
                        rSCLY.Tell() - 4,
                        pEnum->Name(),
                        pEnum->IDString(true),
                        Value);
        }
#endif
        break;
    }

    case EPropertyType::Flags:
    {
        CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProp);
        pFlags->ValueRef(pData) = rSCLY.ReadU32();

#if VALIDATE_PROPERTY_VALUES
        const auto InvalidBits = pFlags->HasValidValue(pData);

        if (InvalidBits)
        {
            NLog::Warn("{} [0x{:X}]: Flags property \"{}\" ({}) has unrecognized flags set: 0x{:08X}",
                       rSCLY.GetSourceString(),
                       rSCLY.Tell() - 4,
                       pFlags->Name(),
                       pFlags->IDString(true),
                       InvalidBits);
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
        const CAssetID ID(pAsset->ValueRef(pData));

        if (ID.IsValid())
        {
            CResourceEntry *pEntry = mResourceStore->FindEntry(ID);

            if (pEntry)
            {
                const CResTypeFilter& rkFilter = pAsset->GetTypeFilter();
                bool Valid = rkFilter.Accepts(pEntry->ResourceType());

                if (!Valid)
                {
                    NLog::Warn("{} [0x{:X}]: Asset property \"{}\" ({}) has a reference to an illegal asset type: {}",
                               rSCLY.GetSourceString(),
                               rSCLY.Tell() - static_cast<uint32_t>(ID.Length()),
                               pAsset->Name(),
                               pAsset->IDString(true),
                               pEntry->CookedExtension().ToString());
                }
            }
        }
#endif
        break;
    }

    case EPropertyType::Sound:
    {
        CSoundProperty* pSound = TPropCast<CSoundProperty>(pProp);
        pSound->ValueRef(pData) = rSCLY.ReadS32();
        break;
    }

    case EPropertyType::Animation:
    {
        CAnimationProperty* pAnim = TPropCast<CAnimationProperty>(pProp);
        pAnim->ValueRef(pData) = rSCLY.ReadU32();
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
        ASSERT(Size == 16 || pProp->IsAtomic());
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
        const auto Count = rSCLY.ReadS32();

        pArray->Resize(pData, Count);
        void* pOldData = mpCurrentData;

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
            mpCurrentData = pArray->ItemPointer(pData, ElementIdx);
            ReadProperty(pArray->ItemArchetype(), 0, rSCLY);
        }

        mpCurrentData = pOldData;
        break;
    }
    default: break;
    }
}

void CScriptLoader::LoadStructMP1(IInputStream& rSCLY, CStructProperty* pStruct)
{
    [[maybe_unused]] const auto StructStart = rSCLY.Tell();

    // Verify property count
    const size_t PropertyCount = pStruct->NumChildren();
    [[maybe_unused]] uint32_t Version = 0;

    if (!pStruct->IsAtomic())
    {
        [[maybe_unused]] const auto FilePropCount = rSCLY.ReadU32();
        //@todo version checking
    }

    // Parse properties
    for (size_t ChildIndex = 0; ChildIndex < PropertyCount; ChildIndex++)
    {
        IProperty *pProperty = pStruct->ChildByIndex(ChildIndex);

        //@todo version check
        if (pProperty->CookPreference() != ECookPreference::Never)
            ReadProperty(pProperty, 0, rSCLY);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP1(IInputStream& rSCLY)
{
    const auto StartOffset = rSCLY.Tell();
    const auto Type = rSCLY.ReadU8();
    const auto Size = rSCLY.ReadU32();
    const auto End = rSCLY.Tell() + Size;

    CScriptTemplate *pTemplate = mpGameTemplate->TemplateByID(static_cast<uint32_t>(Type));
    if (!pTemplate)
    {
        // No valid template for this object; can't load
        NLog::Error("{} [0x{:X}]: Unknown object ID encountered: 0x{:02X}", rSCLY.GetSourceString(), StartOffset, Type);
        rSCLY.Seek(End, SEEK_SET);
        return nullptr;
    }

    auto InstanceID = CInstanceID(rSCLY.ReadU32() & 0x03FFFFFF);
    if (InstanceID.Value() == 0x03FFFFFFU)
        InstanceID = mpArea->FindUnusedInstanceID();
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemplate);

    // Load connections
    const auto NumLinks = rSCLY.ReadU32();
    mpObj->mOutLinks.reserve(NumLinks);

    for (uint32_t iLink = 0; iLink < NumLinks; iLink++)
    {
        const auto State = rSCLY.ReadU32();
        const auto Message = rSCLY.ReadU32();
        const auto ReceiverID = CInstanceID(rSCLY.ReadU32() & 0x03FFFFFF);

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

std::unique_ptr<CScriptLayer> CScriptLoader::LoadLayerMP1(IInputStream& rSCLY)
{
    const auto LayerStart = rSCLY.Tell();

    rSCLY.Seek(0x1, SEEK_CUR); // One unknown byte at the start of each layer
    const auto NumObjects = rSCLY.ReadU32();

    auto layer = std::make_unique<CScriptLayer>(mpArea);

    mpLayer = layer.get();
    mpLayer->Reserve(NumObjects);

    for (uint32_t ObjectIndex = 0; ObjectIndex < NumObjects; ObjectIndex++)
    {
        CScriptObject *pObject = LoadObjectMP1(rSCLY);
        if (pObject)
            mpLayer->AddInstance(pObject);
    }

    // Layer sizes are always a multiple of 32 - skip end padding before returning
    const uint32_t Remaining = 32 - ((rSCLY.Tell() - LayerStart) & 0x1F);
    rSCLY.Seek(Remaining, SEEK_CUR);

    return layer;
}

void CScriptLoader::LoadStructMP2(IInputStream& rSCLY, CStructProperty* pStruct)
{
    // Verify property count
    size_t ChildCount = pStruct->NumChildren();

    if (!pStruct->IsAtomic())
        ChildCount = rSCLY.ReadU16();

    // Parse properties
    for (size_t ChildIdx = 0; ChildIdx < ChildCount; ChildIdx++)
    {
        IProperty* pProperty = nullptr;
        const uint32_t PropertyStart = rSCLY.Tell();
        uint32_t PropertyID = UINT32_MAX;
        uint16_t PropertySize = 0;
        uint32_t NextProperty = 0;

        if (pStruct->IsAtomic())
        {
            pProperty = pStruct->ChildByIndex(ChildIdx);
        }
        else
        {
            PropertyID = rSCLY.ReadU32();
            PropertySize = rSCLY.ReadU16();
            NextProperty = rSCLY.Tell() + PropertySize;
            pProperty = pStruct->ChildByID(PropertyID);
        }

        if (pProperty)
            ReadProperty(pProperty, PropertySize, rSCLY);
        else
            NLog::Error("{} [0x{:X}]: Can't find template for property 0x{:08X} - skipping", rSCLY.GetSourceString(), PropertyStart, PropertyID);

        if (NextProperty > 0)
            rSCLY.Seek(NextProperty, SEEK_SET);
    }
}

CScriptObject* CScriptLoader::LoadObjectMP2(IInputStream& rSCLY)
{
    const auto ObjStart = rSCLY.Tell();
    const auto ObjectID = rSCLY.ReadU32();
    const auto ObjectSize = rSCLY.ReadU16();
    const auto ObjEnd = rSCLY.Tell() + ObjectSize;

    CScriptTemplate* pTemplate = mpGameTemplate->TemplateByID(ObjectID);

    if (!pTemplate)
    {
        NLog::Error("{} [0x{:X}]: Unknown object ID encountered: {}", rSCLY.GetSourceString(), ObjStart, CFourCC(ObjectID).ToString());
        rSCLY.Seek(ObjEnd, SEEK_SET);
        return nullptr;
    }

    auto InstanceID = CInstanceID(rSCLY.ReadU32() & 0x03FFFFFF);
    if (InstanceID.Value() == 0x03FFFFFFU)
        InstanceID = mpArea->FindUnusedInstanceID();
    mpObj = new CScriptObject(InstanceID, mpArea, mpLayer, pTemplate);

    // Load connections
    const uint32_t NumConnections = rSCLY.ReadU16();
    mpObj->mOutLinks.reserve(NumConnections);

    for (uint32_t LinkIdx = 0; LinkIdx < NumConnections; LinkIdx++)
    {
        const auto State = rSCLY.ReadU32();
        const auto Message = rSCLY.ReadU32();
        const auto ReceiverID = CInstanceID(rSCLY.ReadU32() & 0x03FFFFFF);

        auto* pLink = new CLink(mpArea, State, Message, mpObj->mInstanceID, ReceiverID);
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

std::unique_ptr<CScriptLayer> CScriptLoader::LoadLayerMP2(IInputStream& rSCLY)
{
    rSCLY.Seek(0x1, SEEK_CUR); // Skipping version. todo: verify this?
    const auto NumObjects = rSCLY.ReadU32();

    auto layer = std::make_unique<CScriptLayer>(mpArea);

    mpLayer = layer.get();
    mpLayer->Reserve(NumObjects);

    for (uint32_t ObjectIdx = 0; ObjectIdx < NumObjects; ObjectIdx++)
    {
        CScriptObject* pObject = LoadObjectMP2(rSCLY);
        if (pObject)
            mpLayer->AddInstance(pObject);
    }

    return layer;
}

// ************ STATIC ************
std::unique_ptr<CScriptLayer> CScriptLoader::LoadLayer(IInputStream& rSCLY, CGameArea *pArea, EGame Version)
{
    if (!rSCLY.IsValid())
        return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = Version;
    Loader.mpGameTemplate = NGameList::GetGameTemplate(Version);
    Loader.mpArea = pArea;
    Loader.mResourceStore = pArea->Entry()->ResourceStore();

    if (!Loader.mpGameTemplate)
    {
        NLog::Debug("This game doesn't have a game template; couldn't load script layer");
        return nullptr;
    }

    if (Version <= EGame::Prime)
        return Loader.LoadLayerMP1(rSCLY);
    else
        return Loader.LoadLayerMP2(rSCLY);
}

CScriptObject* CScriptLoader::LoadInstance(IInputStream& rSCLY, CGameArea *pArea, CScriptLayer *pLayer, EGame Version, bool ForceReturnsFormat)
{
    if (!rSCLY.IsValid())
        return nullptr;

    CScriptLoader Loader;
    Loader.mVersion = (ForceReturnsFormat ? EGame::DKCReturns : Version);
    Loader.mpGameTemplate = NGameList::GetGameTemplate(Version);
    Loader.mpArea = pArea;
    Loader.mpLayer = pLayer;
    Loader.mResourceStore = pArea->Entry()->ResourceStore();

    if (!Loader.mpGameTemplate)
    {
        NLog::Debug("This game doesn't have a game template; couldn't load script instance");
        return nullptr;
    }

    if (Loader.mVersion <= EGame::Prime)
        return Loader.LoadObjectMP1(rSCLY);
    else
        return Loader.LoadObjectMP2(rSCLY);
}

void CScriptLoader::LoadStructData(IInputStream& rInput, CStructRef InStruct, CResourceStore* resourceStore)
{
    if (!rInput.IsValid())
        return;

    CScriptLoader Loader;
    Loader.mVersion = InStruct.Property()->Game();
    Loader.mpGameTemplate = NGameList::GetGameTemplate(Loader.mVersion);
    Loader.mpArea = nullptr;
    Loader.mpLayer = nullptr;
    Loader.mpCurrentData = InStruct.DataPointer();
    Loader.mResourceStore = resourceStore;

    if (Loader.mVersion <= EGame::Prime)
        Loader.LoadStructMP1(rInput, InStruct.Property());
    else
        Loader.LoadStructMP2(rInput, InStruct.Property());
}
