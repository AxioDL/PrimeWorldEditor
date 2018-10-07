#include "CScriptCooker.h"
#include "Core/Resource/Script/CLink.h"
#include <Core/Resource/Script/Property/CArrayProperty.h>
#include <Core/Resource/Script/Property/CAssetProperty.h>
#include <Core/Resource/Script/Property/CEnumProperty.h>
#include <Core/Resource/Script/Property/CFlagsProperty.h>

void CScriptCooker::WriteProperty(IOutputStream& rOut, IProperty* pProperty, bool InAtomicStruct)
{
    u32 SizeOffset = 0, PropStart = 0;
    void* pData = (mpArrayItemData ? mpArrayItemData : mpObject->PropertyData());

    if (mGame >= EGame::EchoesDemo && !InAtomicStruct)
    {
        rOut.WriteLong(pProperty->ID());
        SizeOffset = rOut.Tell();
        rOut.WriteShort(0x0);
        PropStart = rOut.Tell();
    }

    switch (pProperty->Type())
    {

    case EPropertyType::Bool:
    {
        CBoolProperty* pBool = TPropCast<CBoolProperty>(pProperty);
        rOut.WriteBool( pBool->Value(pData) );
        break;
    }

    case EPropertyType::Byte:
    {
        CByteProperty* pByte = TPropCast<CByteProperty>(pProperty);
        rOut.WriteByte( pByte->Value(pData) );
        break;
    }

    case EPropertyType::Short:
    {
        CShortProperty* pShort = TPropCast<CShortProperty>(pProperty);
        rOut.WriteShort( pShort->Value(pData) );
        break;
    }

    case EPropertyType::Int:
    {
        CIntProperty* pInt = TPropCast<CIntProperty>(pProperty);
        rOut.WriteLong( pInt->Value(pData) );
        break;
    }

    case EPropertyType::Float:
    {
        CFloatProperty* pFloat = TPropCast<CFloatProperty>(pProperty);
        rOut.WriteFloat( pFloat->Value(pData) );
        break;
    }

    case EPropertyType::Choice:
    {
        CChoiceProperty* pChoice = TPropCast<CChoiceProperty>(pProperty);
        rOut.WriteLong( pChoice->Value(pData) );
        break;
    }

    case EPropertyType::Enum:
    {
        CEnumProperty* pEnum = TPropCast<CEnumProperty>(pProperty);
        rOut.WriteLong( pEnum->Value(pData) );
        break;
    }

    case EPropertyType::Flags:
    {
        CFlagsProperty* pFlags = TPropCast<CFlagsProperty>(pProperty);
        rOut.WriteLong( pFlags->Value(pData) );
        break;
    }

    case EPropertyType::String:
    {
        CStringProperty* pString = TPropCast<CStringProperty>(pProperty);
        rOut.WriteString( pString->Value(pData) );
        break;
    }

    case EPropertyType::Vector:
    {
        CVectorProperty* pVector = TPropCast<CVectorProperty>(pProperty);
        pVector->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Color:
    {
        CColorProperty* pColor = TPropCast<CColorProperty>(pProperty);
        pColor->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Asset:
    {
        CAssetProperty* pAsset = TPropCast<CAssetProperty>(pProperty);
        pAsset->ValueRef(pData).Write(rOut);
        break;
    }

    case EPropertyType::Sound:
    {
        CSoundProperty* pSound = TPropCast<CSoundProperty>(pProperty);
        rOut.WriteLong( pSound->Value(pData) );
        break;
    }

    case EPropertyType::Animation:
    {
        CAnimationProperty* pAnim = TPropCast<CAnimationProperty>(pProperty);
        rOut.WriteLong( pAnim->Value(pData) );
        break;
    }

    case EPropertyType::AnimationSet:
    {
        CAnimationSetProperty* pAnimSet = TPropCast<CAnimationSetProperty>(pProperty);
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
        CSplineProperty* pSpline = TPropCast<CSplineProperty>(pProperty);
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
        CGuidProperty* pGuid = TPropCast<CGuidProperty>(pProperty);
        std::vector<char>& rBuffer = pGuid->ValueRef(pData);

        if (rBuffer.empty())
            rBuffer.resize(16, 0);

        rOut.WriteBytes( rBuffer.data(), rBuffer.size() );
        break;
    }

    case EPropertyType::Struct:
    {
        CStructProperty* pStruct = TPropCast<CStructProperty>(pProperty);
        std::vector<IProperty*> PropertiesToWrite;

        for (u32 ChildIdx = 0; ChildIdx < pStruct->NumChildren(); ChildIdx++)
        {
            IProperty *pChild = pStruct->ChildByIndex(ChildIdx);

            if (pStruct->IsAtomic() || pChild->ShouldCook(pData))
                PropertiesToWrite.push_back(pChild);
        }

        if (!pStruct->IsAtomic())
        {
            if (mGame <= EGame::Prime)
                rOut.WriteLong(PropertiesToWrite.size());
            else
                rOut.WriteShort((u16) PropertiesToWrite.size());
        }

        for (u32 PropertyIdx = 0; PropertyIdx < PropertiesToWrite.size(); PropertyIdx++)
            WriteProperty(rOut, PropertiesToWrite[PropertyIdx], pStruct->IsAtomic());

        break;
    }

    case EPropertyType::Array:
    {
        CArrayProperty* pArray = TPropCast<CArrayProperty>(pProperty);
        u32 Count = pArray->ArrayCount(pData);
        rOut.WriteLong(Count);

        void* pOldItemData = mpArrayItemData;

        for (u32 ElementIdx = 0; ElementIdx < pArray->ArrayCount(pData); ElementIdx++)
        {
            mpArrayItemData = pArray->ItemPointer(pData, ElementIdx);
            WriteProperty(rOut, pArray->ItemArchetype(), true);
        }

        mpArrayItemData = pOldItemData;
        break;
    }

    }

    if (SizeOffset != 0)
    {
        u32 PropEnd = rOut.Tell();
        rOut.Seek(SizeOffset, SEEK_SET);
        rOut.WriteShort((u16) (PropEnd - PropStart));
        rOut.Seek(PropEnd, SEEK_SET);
    }
}

// ************ PUBLIC ************
void CScriptCooker::WriteInstance(IOutputStream& rOut, CScriptObject *pInstance)
{
    ASSERT(pInstance->Area()->Game() == mGame);

    // Note the format is pretty much the same between games; the main difference is a
    // number of fields changed size between MP1 and 2, but they're still the same fields
    bool IsPrime1 = (mGame <= EGame::Prime);

    u32 ObjectType = pInstance->ObjectTypeID();
    IsPrime1 ? rOut.WriteByte((u8) ObjectType) : rOut.WriteLong(ObjectType);

    u32 SizeOffset = rOut.Tell();
    IsPrime1 ? rOut.WriteLong(0) : rOut.WriteShort(0);

    u32 InstanceStart = rOut.Tell();
    u32 InstanceID = (pInstance->Layer()->AreaIndex() << 26) | pInstance->InstanceID();
    rOut.WriteLong(InstanceID);

    u32 NumLinks = pInstance->NumLinks(eOutgoing);
    IsPrime1 ? rOut.WriteLong(NumLinks) : rOut.WriteShort((u16) NumLinks);

    for (u32 LinkIdx = 0; LinkIdx < NumLinks; LinkIdx++)
    {
        CLink *pLink = pInstance->Link(eOutgoing, LinkIdx);
        rOut.WriteLong(pLink->State());
        rOut.WriteLong(pLink->Message());
        rOut.WriteLong(pLink->ReceiverID());
    }

    mpObject = pInstance;
    WriteProperty(rOut, pInstance->Template()->Properties(), false);
    u32 InstanceEnd = rOut.Tell();

    rOut.Seek(SizeOffset, SEEK_SET);
    u32 Size = InstanceEnd - InstanceStart;
    IsPrime1 ? rOut.WriteLong(Size) : rOut.WriteShort((u16) Size);
    rOut.Seek(InstanceEnd, SEEK_SET);
}

void CScriptCooker::WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer)
{
    ASSERT(pLayer->Area()->Game() == mGame);

    rOut.WriteByte( mGame <= EGame::Prime ? 0 : 1 ); // Version

    u32 InstanceCountOffset = rOut.Tell();
    u32 NumWrittenInstances = 0;
    rOut.WriteLong(0);

    for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptObject *pInstance = pLayer->InstanceByIndex(iInst);

        // Is this a generated instance?
        bool ShouldWrite = true;

        if (mWriteGeneratedSeparately)
        {
            // GenericCreature instances in DKCR always write to both SCLY and SCGN
            if (mGame == EGame::DKCReturns && pInstance->ObjectTypeID() == FOURCC('GCTR'))
                mGeneratedObjects.push_back(pInstance);

            // Instances receiving a Generate/Activate message (MP2) or a
            // Generate/Attach message (MP3+) should be written to SCGN, not SCLY
            else
            {
                for (u32 LinkIdx = 0; LinkIdx < pInstance->NumLinks(eIncoming); LinkIdx++)
                {
                    CLink *pLink = pInstance->Link(eIncoming, LinkIdx);

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

    u32 LayerEnd = rOut.Tell();
    rOut.GoTo(InstanceCountOffset);
    rOut.WriteLong(NumWrittenInstances);
    rOut.GoTo(LayerEnd);
}

void CScriptCooker::WriteGeneratedLayer(IOutputStream& rOut)
{
    rOut.WriteByte(1); // Version
    rOut.WriteLong(mGeneratedObjects.size());

    for (u32 ObjectIdx = 0; ObjectIdx < mGeneratedObjects.size(); ObjectIdx++)
        WriteInstance(rOut, mGeneratedObjects[ObjectIdx]);
}
