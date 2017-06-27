#include "CScriptCooker.h"
#include "Core/Resource/Script/CLink.h"

void CScriptCooker::WriteProperty(IOutputStream& rOut,IProperty *pProp, bool InSingleStruct)
{
    u32 SizeOffset = 0, PropStart = 0;

    if (mGame >= eEchoesDemo && !InSingleStruct)
    {
        rOut.WriteLong(pProp->ID());
        SizeOffset = rOut.Tell();
        rOut.WriteShort(0x0);
        PropStart = rOut.Tell();
    }

    switch (pProp->Type())
    {

    case eBoolProperty:
    {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(pProp);
        rOut.WriteBool(pBoolCast->Get());
        break;
    }

    case eByteProperty:
    {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(pProp);
        rOut.WriteByte(pByteCast->Get());
        break;
    }

    case eShortProperty:
    {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(pProp);
        rOut.WriteShort(pShortCast->Get());
        break;
    }

    case eLongProperty:
    {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(pProp);
        rOut.WriteLong(pLongCast->Get());
        break;
    }

    case eEnumProperty:
    {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(pProp);
        rOut.WriteLong(pEnumCast->Get());
        break;
    }

    case eBitfieldProperty:
    {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(pProp);
        rOut.WriteLong(pBitfieldCast->Get());
        break;
    }

    case eFloatProperty:
    {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(pProp);
        rOut.WriteFloat(pFloatCast->Get());
        break;
    }

    case eStringProperty:
    {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(pProp);
        rOut.WriteString(pStringCast->Get());
        break;
    }

    case eVector3Property:
    {
        TVector3Property *pVectorCast = static_cast<TVector3Property*>(pProp);
        pVectorCast->Get().Write(rOut);
        break;
    }

    case eColorProperty:
    {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(pProp);
        pColorCast->Get().Write(rOut, false);
        break;
    }

    case eSoundProperty:
    {
        TSoundProperty *pSoundCast = static_cast<TSoundProperty*>(pProp);
        rOut.WriteLong(pSoundCast->Get());
        break;
    }

    case eAssetProperty:
    {
        TAssetProperty *pAssetCast = static_cast<TAssetProperty*>(pProp);
        pAssetCast->Get().Write(rOut);
        break;
    }

    case eCharacterProperty:
    {
        TCharacterProperty *pCharCast = static_cast<TCharacterProperty*>(pProp);
        pCharCast->Get().Write(rOut);
        break;
    }

    case eMayaSplineProperty:
    {
        TMayaSplineProperty *pSplineCast = static_cast<TMayaSplineProperty*>(pProp);
        std::vector<u8> Buffer = pSplineCast->Get();
        if (!Buffer.empty()) rOut.WriteBytes(Buffer.data(), Buffer.size());

        else
        {
            if (mGame < eReturns)
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

    case eStructProperty:
    {
        CPropertyStruct *pStruct = static_cast<CPropertyStruct*>(pProp);
        CStructTemplate *pTemp = static_cast<CStructTemplate*>(pStruct->Template());

        std::vector<IProperty*> PropertiesToWrite;

        for (u32 iProp = 0; iProp < pStruct->Count(); iProp++)
        {
            IProperty *pSubProp = pStruct->PropertyByIndex(iProp);\

            if (pTemp->IsSingleProperty() || pSubProp->ShouldCook())
                PropertiesToWrite.push_back(pSubProp);
        }

        if (!pTemp->IsSingleProperty())
        {
            if (mGame <= ePrime)
                rOut.WriteLong(PropertiesToWrite.size());
            else
                rOut.WriteShort((u16) PropertiesToWrite.size());
        }

        for (u32 iProp = 0; iProp < PropertiesToWrite.size(); iProp++)
            WriteProperty(rOut, PropertiesToWrite[iProp], pTemp->IsSingleProperty());

        break;
    }

    case eArrayProperty:
    {
        CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
        rOut.WriteLong(pArray->Count());

        for (u32 iProp = 0; iProp < pArray->Count(); iProp++)
            WriteProperty(rOut, pArray->PropertyByIndex(iProp), true);

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
    bool IsPrime1 = (mGame <= ePrime);

    u32 ObjectType = pInstance->ObjectTypeID();
    IsPrime1 ? rOut.WriteByte((u8) ObjectType) : rOut.WriteLong(ObjectType);

    u32 SizeOffset = rOut.Tell();
    IsPrime1 ? rOut.WriteLong(0) : rOut.WriteShort(0);

    u32 InstanceStart = rOut.Tell();
    u32 InstanceID = (pInstance->Layer()->AreaIndex() << 26) | pInstance->InstanceID();
    rOut.WriteLong(InstanceID);

    u32 NumLinks = pInstance->NumLinks(eOutgoing);
    IsPrime1 ? rOut.WriteLong(NumLinks) : rOut.WriteShort((u16) NumLinks);

    for (u32 iLink = 0; iLink < pInstance->NumLinks(eOutgoing); iLink++)
    {
        CLink *pLink = pInstance->Link(eOutgoing, iLink);
        rOut.WriteLong(pLink->State());
        rOut.WriteLong(pLink->Message());
        rOut.WriteLong(pLink->ReceiverID());
    }

    WriteProperty(rOut, pInstance->Properties(), false);
    u32 InstanceEnd = rOut.Tell();

    rOut.Seek(SizeOffset, SEEK_SET);
    u32 Size = InstanceEnd - InstanceStart;
    IsPrime1 ? rOut.WriteLong(Size) : rOut.WriteShort((u16) Size);
    rOut.Seek(InstanceEnd, SEEK_SET);
}

void CScriptCooker::WriteLayer(IOutputStream& rOut, CScriptLayer *pLayer)
{
    ASSERT(pLayer->Area()->Game() == mGame);

    rOut.WriteByte( mGame <= ePrime ? 0 : 1 ); // Version

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
            if (mGame == eReturns && pInstance->ObjectTypeID() == FOURCC('GCTR'))
                mGeneratedObjects.push_back(pInstance);

            // Instances receiving a Generate/Activate message (MP2) or a
            // Generate/Attach message (MP3+) should be written to SCGN, not SCLY
            else
            {
                for (u32 LinkIdx = 0; LinkIdx < pInstance->NumLinks(eIncoming); LinkIdx++)
                {
                    CLink *pLink = pInstance->Link(eIncoming, LinkIdx);

                    if (mGame <= eEchoes)
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

    for (u32 InstIdx = 0; InstIdx < mGeneratedObjects.size(); InstIdx++)
        WriteInstance(rOut, mGeneratedObjects[InstIdx]);
}
