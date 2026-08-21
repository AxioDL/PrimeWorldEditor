#include "Core/Resource/Cooker/CScriptCooker.h"

#include "Core/Resource/Script/CLink.h"
#include "Core/Resource/Script/CScriptLayer.h"
#include "Core/Resource/Script/CScriptObject.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include <Core/Resource/Script/Property/CArrayProperty.h>
#include <Core/Resource/Script/Property/CAssetProperty.h>
#include <Core/Resource/Script/Property/CEnumProperty.h>
#include <Core/Resource/Script/Property/CFlagsProperty.h>

#include <Common/FileIO/IOutputStream.h>

CScriptCooker::CScriptCooker(EGame Game, bool WriteGeneratedObjectsSeparately)
    : mGame(Game)
    , mWriteGeneratedSeparately(WriteGeneratedObjectsSeparately && mGame >= EGame::EchoesDemo)
{
}

CScriptCooker::~CScriptCooker() = default;

void CScriptCooker::WriteProperty(IOutputStream& rOut, IProperty* pProperty, void* pData, bool InAtomicStruct)
{
    uint32_t SizeOffset = 0;
    uint32_t PropStart = 0;

    if (mGame >= EGame::EchoesDemo && !InAtomicStruct)
    {
        rOut.WriteU32(pProperty->ID());
        SizeOffset = rOut.Tell();
        rOut.WriteU16(0x0);
        PropStart = rOut.Tell();
    }

    switch (pProperty->Type())
    {
    case EPropertyType::Bool:
    {
        auto* pBool = TPropCast<CBoolProperty>(pProperty);
        rOut.WriteBool(pBool->Value(pData));
        break;
    }

    case EPropertyType::Byte:
    {
        auto* pByte = TPropCast<CByteProperty>(pProperty);
        rOut.WriteS8(pByte->Value(pData));
        break;
    }

    case EPropertyType::Short:
    {
        auto* pShort = TPropCast<CShortProperty>(pProperty);
        rOut.WriteS16(pShort->Value(pData));
        break;
    }

    case EPropertyType::Int:
    {
        auto* pInt = TPropCast<CIntProperty>(pProperty);
        rOut.WriteS32(pInt->Value(pData));
        break;
    }

    case EPropertyType::Float:
    {
        auto* pFloat = TPropCast<CFloatProperty>(pProperty);
        rOut.WriteF32(pFloat->Value(pData));
        break;
    }

    case EPropertyType::Choice:
    {
        auto* pChoice = TPropCast<CChoiceProperty>(pProperty);
        rOut.WriteS32(pChoice->Value(pData));
        break;
    }

    case EPropertyType::Enum:
    {
        auto* pEnum = TPropCast<CEnumProperty>(pProperty);
        rOut.WriteS32(pEnum->Value(pData));
        break;
    }

    case EPropertyType::Flags:
    {
        auto* pFlags = TPropCast<CFlagsProperty>(pProperty);
        rOut.WriteU32(pFlags->Value(pData));
        break;
    }

    case EPropertyType::String:
    {
        auto* pString = TPropCast<CStringProperty>(pProperty);
        rOut.WriteString(pString->ValueRef(pData));
        break;
    }

    case EPropertyType::Vector:
    {
        auto* pVector = TPropCast<CVectorProperty>(pProperty);
        pVector->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Color:
    {
        auto* pColor = TPropCast<CColorProperty>(pProperty);
        pColor->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Asset:
    {
        auto* pAsset = TPropCast<CAssetProperty>(pProperty);
        pAsset->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Sound:
    {
        auto* pSound = TPropCast<CSoundProperty>(pProperty);
        rOut.WriteS32(pSound->Value(pData));
        break;
    }

    case EPropertyType::Animation:
    {
        auto* pAnim = TPropCast<CAnimationProperty>(pProperty);
        rOut.WriteU32(pAnim->Value(pData));
        break;
    }

    case EPropertyType::AnimationSet:
    {
        auto* pAnimSet = TPropCast<CAnimationSetProperty>(pProperty);
        pAnimSet->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Sequence:
    {
        // TODO
        break;
    }

    case EPropertyType::Spline:
    {
        const auto* pSpline = TPropCast<CSplineProperty>(pProperty);
        const std::vector<char>& rBuffer = pSpline->ValueRef(pData);

        if (!rBuffer.empty())
        {
            rOut.WriteBytes(rBuffer.data(), rBuffer.size());
        }
        else
        {
            if (mGame < EGame::DKCReturns)
            {
                rOut.WriteU16(0);
                rOut.WriteU32(0);
                rOut.WriteU8(1);
                rOut.WriteF32(0.0f);
                rOut.WriteF32(1.0f);
            }
            else
            {
                rOut.WriteU32(0);
                rOut.WriteF32(0.0f);
                rOut.WriteF32(1.0f);
                rOut.WriteU16(0);
                rOut.WriteU8(1);
            }
        }
        break;
    }

    case EPropertyType::Guid:
    {
        auto* pGuid = TPropCast<CGuidProperty>(pProperty);
        std::vector<char>& rBuffer = pGuid->ValueRef(pData);

        if (rBuffer.empty())
            rBuffer.resize(16, 0);

        rOut.WriteBytes( rBuffer.data(), rBuffer.size() );
        break;
    }

    case EPropertyType::Struct:
    {
        auto* pStruct = TPropCast<CStructProperty>(pProperty);
        std::vector<IProperty*> PropertiesToWrite;

        for (size_t ChildIdx = 0; ChildIdx < pStruct->NumChildren(); ChildIdx++)
        {
            IProperty *pChild = pStruct->ChildByIndex(ChildIdx);

            if (pStruct->IsAtomic() || pChild->ShouldCook(pData))
                PropertiesToWrite.push_back(pChild);
        }

        if (!pStruct->IsAtomic())
        {
            if (mGame <= EGame::Prime)
                rOut.WriteU32(static_cast<uint32_t>(PropertiesToWrite.size()));
            else
                rOut.WriteU16(static_cast<uint16_t>(PropertiesToWrite.size()));
        }

        for (auto* property : PropertiesToWrite)
            WriteProperty(rOut, property, pData, pStruct->IsAtomic());

        break;
    }

    case EPropertyType::Array:
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
        const auto Count = pArray->ArrayCount(pData);
        rOut.WriteU32(Count);

        for (uint32_t ElementIdx = 0; ElementIdx < pArray->ArrayCount(pData); ElementIdx++)
        {
            WriteProperty(rOut, pArray->ItemArchetype(), pArray->ItemPointer(pData, ElementIdx), true);
        }

        break;
    }
    default:
        break;
    }

    if (SizeOffset != 0)
    {
        const auto PropEnd = rOut.Tell();
        rOut.Seek(SizeOffset, SEEK_SET);
        rOut.WriteU16(static_cast<uint16_t>(PropEnd - PropStart));
        rOut.Seek(PropEnd, SEEK_SET);
    }
}

void CScriptCooker::WriteInstance(IOutputStream& rOut, CScriptObject *pInstance)
{
    ASSERT(pInstance->Area()->Game() == mGame);

    // Note the format is pretty much the same between games; the main difference is a
    // number of fields changed size between MP1 and 2, but they're still the same fields
    const bool IsPrime1 = mGame <= EGame::Prime;

    const auto ObjectType = pInstance->ObjectTypeID();
    IsPrime1 ? rOut.WriteU8(static_cast<uint8_t>(ObjectType)) : rOut.WriteU32(ObjectType);

    const auto SizeOffset = rOut.Tell();
    IsPrime1 ? rOut.WriteU32(0) : rOut.WriteU16(0);

    const auto InstanceStart = rOut.Tell();
    const auto InstanceID = (pInstance->Layer()->AreaIndex() << 26) | pInstance->InstanceID().Value();
    rOut.WriteU32(InstanceID);

    const auto Links = pInstance->Links(ELinkType::Outgoing);
    IsPrime1 ? rOut.WriteS32(static_cast<int32_t>(Links.size())) : rOut.WriteU16(static_cast<uint16_t>(Links.size()));

    for (const auto* link : Links)
    {
        rOut.WriteU32(link->State());
        rOut.WriteU32(link->Message());
        rOut.WriteU32(link->ReceiverID().Value());
    }

    WriteProperty(rOut, pInstance->Template()->Properties(), pInstance->PropertyData(), false);
    const auto InstanceEnd = rOut.Tell();

    rOut.Seek(SizeOffset, SEEK_SET);
    const auto Size = InstanceEnd - InstanceStart;
    IsPrime1 ? rOut.WriteU32(Size) : rOut.WriteU16(static_cast<uint16_t>(Size));
    rOut.Seek(InstanceEnd, SEEK_SET);
}

void CScriptCooker::WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer)
{
    ASSERT(pLayer->Area()->Game() == mGame);

    rOut.WriteU8(mGame <= EGame::Prime ? 0 : 1); // Version

    const auto InstanceCountOffset = rOut.Tell();
    uint32_t NumWrittenInstances = 0;
    rOut.WriteU32(0);

    for (auto* instance : pLayer->Instances())
    {
        // Is this a generated instance?
        bool ShouldWrite = true;

        if (mWriteGeneratedSeparately)
        {
            // GenericCreature instances in DKCR always write to both SCLY and SCGN
            if (mGame == EGame::DKCReturns && instance->ObjectTypeID() == CFourCC("GCTR").ToU32())
            {
                mGeneratedObjects.push_back(instance);
            }
            // Instances receiving a Generate/Activate message (MP2) or a
            // Generate/Attach message (MP3+) should be written to SCGN, not SCLY
            else
            {
                for (const auto* link : instance->Links(ELinkType::Incoming))
                {
                    const CFourCC Message(link->Message());
                    const CFourCC State(link->State());

                    if (mGame <= EGame::Echoes)
                    {
                        if (State == CFourCC("GRNT") && Message == CFourCC("ACTV"))
                        {
                            ShouldWrite = false;
                            break;
                        }
                    }
                    else
                    {
                        if (Message == CFourCC("ATCH"))
                        {
                            if (State == CFourCC("GRNT") || State == CFourCC("GRN0") || State == CFourCC("GRN1"))
                            {
                                ShouldWrite = false;
                                break;
                            }
                        }
                    }
                }

                if (!ShouldWrite)
                    mGeneratedObjects.push_back(instance);
            }
        }

        if (ShouldWrite)
        {
            WriteInstance(rOut, instance);
            NumWrittenInstances++;
        }
    }

    const auto LayerEnd = rOut.Tell();
    rOut.GoTo(InstanceCountOffset);
    rOut.WriteU32(NumWrittenInstances);
    rOut.GoTo(LayerEnd);
}

void CScriptCooker::WriteGeneratedLayer(IOutputStream& rOut)
{
    rOut.WriteU8(1); // Version
    rOut.WriteU32(static_cast<uint32_t>(mGeneratedObjects.size()));

    for (auto* object : mGeneratedObjects)
        WriteInstance(rOut, object);
}
