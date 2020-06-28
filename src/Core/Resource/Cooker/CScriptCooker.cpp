#include "CScriptCooker.h"
#include "Core/Resource/Script/CLink.h"
#include <Core/Resource/Script/Property/CArrayProperty.h>
#include <Core/Resource/Script/Property/CAssetProperty.h>
#include <Core/Resource/Script/Property/CEnumProperty.h>
#include <Core/Resource/Script/Property/CFlagsProperty.h>

void CScriptCooker::WriteProperty(IOutputStream& rOut, IProperty* pProperty, void* pData, bool InAtomicStruct)
{
    uint32 SizeOffset = 0;
    uint32 PropStart = 0;

    if (mGame >= EGame::EchoesDemo && !InAtomicStruct)
    {
        rOut.WriteULong(pProperty->ID());
        SizeOffset = rOut.Tell();
        rOut.WriteUShort(0x0);
        PropStart = rOut.Tell();
    }

    switch (pProperty->Type())
    {

    case EPropertyType::Bool:
    {
        auto* pBool = TPropCast<CBoolProperty>(pProperty);
        rOut.WriteBool( pBool->Value(pData) );
        break;
    }

    case EPropertyType::Byte:
    {
        auto* pByte = TPropCast<CByteProperty>(pProperty);
        rOut.WriteByte(pByte->Value(pData));
        break;
    }

    case EPropertyType::Short:
    {
        auto* pShort = TPropCast<CShortProperty>(pProperty);
        rOut.WriteShort(pShort->Value(pData));
        break;
    }

    case EPropertyType::Int:
    {
        auto* pInt = TPropCast<CIntProperty>(pProperty);
        rOut.WriteLong(pInt->Value(pData));
        break;
    }

    case EPropertyType::Float:
    {
        auto* pFloat = TPropCast<CFloatProperty>(pProperty);
        rOut.WriteFloat(pFloat->Value(pData));
        break;
    }

    case EPropertyType::Choice:
    {
        auto* pChoice = TPropCast<CChoiceProperty>(pProperty);
        rOut.WriteLong(pChoice->Value(pData));
        break;
    }

    case EPropertyType::Enum:
    {
        auto* pEnum = TPropCast<CEnumProperty>(pProperty);
        rOut.WriteLong(pEnum->Value(pData));
        break;
    }

    case EPropertyType::Flags:
    {
        auto* pFlags = TPropCast<CFlagsProperty>(pProperty);
        rOut.WriteLong(pFlags->Value(pData));
        break;
    }

    case EPropertyType::String:
    {
        auto* pString = TPropCast<CStringProperty>(pProperty);
        rOut.WriteString(pString->Value(pData));
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
        rOut.WriteLong( pSound->Value(pData) );
        break;
    }

    case EPropertyType::Animation:
    {
        auto* pAnim = TPropCast<CAnimationProperty>(pProperty);
        rOut.WriteLong( pAnim->Value(pData) );
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
        auto* pSpline = TPropCast<CSplineProperty>(pProperty);
        std::vector<char>& rBuffer = pSpline->ValueRef(pData);

        if (!rBuffer.empty())
        {
            rOut.WriteBytes( rBuffer.data(), rBuffer.size() );
        }
        else
        {
            if (mGame < EGame::DKCReturns)
            {
                rOut.WriteShort(0);
                rOut.WriteLong(0);
                rOut.WriteByte(1);
                rOut.WriteFloat(0);
                rOut.WriteFloat(1);
            }
            else
            {
                rOut.WriteLong(0);
                rOut.WriteFloat(0);
                rOut.WriteFloat(1);
                rOut.WriteShort(0);
                rOut.WriteByte(1);
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
                rOut.WriteULong(static_cast<uint32>(PropertiesToWrite.size()));
            else
                rOut.WriteUShort(static_cast<uint16>(PropertiesToWrite.size()));
        }

        for (auto* property : PropertiesToWrite)
            WriteProperty(rOut, property, pData, pStruct->IsAtomic());

        break;
    }

    case EPropertyType::Array:
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
        const uint32 Count = pArray->ArrayCount(pData);
        rOut.WriteULong(Count);

        for (uint32 ElementIdx = 0; ElementIdx < pArray->ArrayCount(pData); ElementIdx++)
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
        const uint32 PropEnd = rOut.Tell();
        rOut.Seek(SizeOffset, SEEK_SET);
        rOut.WriteUShort(static_cast<uint16>(PropEnd - PropStart));
        rOut.Seek(PropEnd, SEEK_SET);
    }
}

void CScriptCooker::WriteInstance(IOutputStream& rOut, CScriptObject *pInstance)
{
    ASSERT(pInstance->Area()->Game() == mGame);

    // Note the format is pretty much the same between games; the main difference is a
    // number of fields changed size between MP1 and 2, but they're still the same fields
    const bool IsPrime1 = mGame <= EGame::Prime;

    const uint32 ObjectType = pInstance->ObjectTypeID();
    IsPrime1 ? rOut.WriteUByte(static_cast<uint8>(ObjectType)) : rOut.WriteULong(ObjectType);

    const uint32 SizeOffset = rOut.Tell();
    IsPrime1 ? rOut.WriteLong(0) : rOut.WriteShort(0);

    const uint32 InstanceStart = rOut.Tell();
    const uint32 InstanceID = (pInstance->Layer()->AreaIndex() << 26) | pInstance->InstanceID();
    rOut.WriteULong(InstanceID);

    const size_t NumLinks = pInstance->NumLinks(ELinkType::Outgoing);
    IsPrime1 ? rOut.WriteLong(static_cast<int32>(NumLinks)) : rOut.WriteUShort(static_cast<uint16>(NumLinks));

    for (size_t LinkIdx = 0; LinkIdx < NumLinks; LinkIdx++)
    {
        const CLink *pLink = pInstance->Link(ELinkType::Outgoing, LinkIdx);
        rOut.WriteULong(pLink->State());
        rOut.WriteULong(pLink->Message());
        rOut.WriteULong(pLink->ReceiverID());
    }

    WriteProperty(rOut, pInstance->Template()->Properties(), pInstance->PropertyData(), false);
    const uint32 InstanceEnd = rOut.Tell();

    rOut.Seek(SizeOffset, SEEK_SET);
    const uint32 Size = InstanceEnd - InstanceStart;
    IsPrime1 ? rOut.WriteULong(Size) : rOut.WriteUShort(static_cast<uint16>(Size));
    rOut.Seek(InstanceEnd, SEEK_SET);
}

void CScriptCooker::WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer)
{
    ASSERT(pLayer->Area()->Game() == mGame);

    rOut.WriteByte(mGame <= EGame::Prime ? 0 : 1); // Version

    const uint32 InstanceCountOffset = rOut.Tell();
    uint32 NumWrittenInstances = 0;
    rOut.WriteLong(0);

    for (size_t iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptObject *pInstance = pLayer->InstanceByIndex(iInst);

        // Is this a generated instance?
        bool ShouldWrite = true;

        if (mWriteGeneratedSeparately)
        {
            // GenericCreature instances in DKCR always write to both SCLY and SCGN
            if (mGame == EGame::DKCReturns && pInstance->ObjectTypeID() == FOURCC('GCTR'))
            {
                mGeneratedObjects.push_back(pInstance);
            }
            // Instances receiving a Generate/Activate message (MP2) or a
            // Generate/Attach message (MP3+) should be written to SCGN, not SCLY
            else
            {
                for (size_t LinkIdx = 0; LinkIdx < pInstance->NumLinks(ELinkType::Incoming); LinkIdx++)
                {
                    const CLink *pLink = pInstance->Link(ELinkType::Incoming, LinkIdx);

                    if (mGame <= EGame::Echoes)
                    {
                        if (pLink->State() == FOURCC('GRNT') && pLink->Message() == FOURCC('ACTV'))
                        {
                            ShouldWrite = false;
                            break;
                        }
                    }
                    else
                    {
                        if (pLink->Message() == FOURCC('ATCH'))
                        {
                            if (pLink->State() == FOURCC('GRNT') || pLink->State() == FOURCC('GRN0') || pLink->State() == FOURCC('GRN1'))
                            {
                                ShouldWrite = false;
                                break;
                            }
                        }
                    }
                }

                if (!ShouldWrite)
                    mGeneratedObjects.push_back(pInstance);
            }
        }

        if (ShouldWrite)
        {
            WriteInstance(rOut, pInstance);
            NumWrittenInstances++;
        }
    }

    const uint32 LayerEnd = rOut.Tell();
    rOut.GoTo(InstanceCountOffset);
    rOut.WriteULong(NumWrittenInstances);
    rOut.GoTo(LayerEnd);
}

void CScriptCooker::WriteGeneratedLayer(IOutputStream& rOut)
{
    rOut.WriteByte(1); // Version
    rOut.WriteULong(static_cast<uint32>(mGeneratedObjects.size()));

    for (auto* object : mGeneratedObjects)
        WriteInstance(rOut, object);
}
