#include "CScriptCooker.h"

void CScriptCooker::WriteProperty(IProperty *pProp, bool InSingleStruct)
{
    u32 SizeOffset = 0, PropStart = 0;

    if (mVersion >= eEchoesDemo && !InSingleStruct)
    {
        mpSCLY->WriteLong(pProp->ID());
        SizeOffset = mpSCLY->Tell();
        mpSCLY->WriteShort(0x0);
        PropStart = mpSCLY->Tell();
    }

    switch (pProp->Type())
    {

    case eBoolProperty:
    {
        TBoolProperty *pBoolCast = static_cast<TBoolProperty*>(pProp);
        mpSCLY->WriteByte(pBoolCast->Get() ? 1 : 0);
        break;
    }

    case eByteProperty:
    {
        TByteProperty *pByteCast = static_cast<TByteProperty*>(pProp);
        mpSCLY->WriteByte(pByteCast->Get());
        break;
    }

    case eShortProperty:
    {
        TShortProperty *pShortCast = static_cast<TShortProperty*>(pProp);
        mpSCLY->WriteShort(pShortCast->Get());
        break;
    }

    case eLongProperty:
    {
        TLongProperty *pLongCast = static_cast<TLongProperty*>(pProp);
        mpSCLY->WriteLong(pLongCast->Get());
        break;
    }

    case eEnumProperty:
    {
        TEnumProperty *pEnumCast = static_cast<TEnumProperty*>(pProp);
        mpSCLY->WriteLong(pEnumCast->Get());
        break;
    }

    case eBitfieldProperty:
    {
        TBitfieldProperty *pBitfieldCast = static_cast<TBitfieldProperty*>(pProp);
        mpSCLY->WriteLong(pBitfieldCast->Get());
        break;
    }

    case eFloatProperty:
    {
        TFloatProperty *pFloatCast = static_cast<TFloatProperty*>(pProp);
        mpSCLY->WriteFloat(pFloatCast->Get());
        break;
    }

    case eStringProperty:
    {
        TStringProperty *pStringCast = static_cast<TStringProperty*>(pProp);
        mpSCLY->WriteString(pStringCast->Get().ToStdString());
        break;
    }

    case eVector3Property:
    {
        TVector3Property *pVectorCast = static_cast<TVector3Property*>(pProp);
        pVectorCast->Get().Write(*mpSCLY);
        break;
    }

    case eColorProperty:
    {
        TColorProperty *pColorCast = static_cast<TColorProperty*>(pProp);
        pColorCast->Get().Write(*mpSCLY, false);
        break;
    }

    case eFileProperty:
    {
        TFileProperty *pFileCast = static_cast<TFileProperty*>(pProp);
        if (mVersion <= eEchoes)
            mpSCLY->WriteLong(pFileCast->Get().ID().ToLong());
        else
            mpSCLY->WriteLongLong(pFileCast->Get().ID().ToLongLong());
        break;
    }

    case eCharacterProperty:
    {
        TCharacterProperty *pCharCast = static_cast<TCharacterProperty*>(pProp);
        pCharCast->Get().Write(*mpSCLY);
        break;
    }

    case eMayaSplineProperty:
    {
        TMayaSplineProperty *pSplineCast = static_cast<TMayaSplineProperty*>(pProp);
        std::vector<u8> Buffer = pSplineCast->Get();
        if (!Buffer.empty()) mpSCLY->WriteBytes(Buffer.data(), Buffer.size());

        else
        {
            if (mVersion < eReturns)
            {
                mpSCLY->WriteShort(0);
                mpSCLY->WriteLong(0);
                mpSCLY->WriteByte(1);
                mpSCLY->WriteFloat(0);
                mpSCLY->WriteFloat(1);
            }
            else
            {
                mpSCLY->WriteLong(0);
                mpSCLY->WriteFloat(0);
                mpSCLY->WriteFloat(1);
                mpSCLY->WriteShort(0);
                mpSCLY->WriteByte(1);
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
            IProperty *pSubProp = pStruct->PropertyByIndex(iProp);
            ECookPreference Pref = pSubProp->Template()->CookPreference();
            if (Pref == eNeverCook) continue;

            if (mVersion < eReturns || pTemp->IsSingleProperty() || Pref == eAlwaysCook || !pSubProp->MatchesDefault())
                PropertiesToWrite.push_back(pSubProp);
        }

        if (!pTemp->IsSingleProperty())
        {
            if (mVersion <= ePrime)
                mpSCLY->WriteLong(PropertiesToWrite.size());
            else
                mpSCLY->WriteShort((u16) PropertiesToWrite.size());
        }

        for (u32 iProp = 0; iProp < PropertiesToWrite.size(); iProp++)
            WriteProperty(PropertiesToWrite[iProp], pTemp->IsSingleProperty());

        break;
    }

    case eArrayProperty:
    {
        CArrayProperty *pArray = static_cast<CArrayProperty*>(pProp);
        mpSCLY->WriteLong(pArray->Count());

        for (u32 iProp = 0; iProp < pArray->Count(); iProp++)
            WriteProperty(pArray->PropertyByIndex(iProp), true);

        break;
    }

    }

    if (SizeOffset != 0)
    {
        u32 PropEnd = mpSCLY->Tell();
        mpSCLY->Seek(SizeOffset, SEEK_SET);
        mpSCLY->WriteShort((u16) (PropEnd - PropStart));
        mpSCLY->Seek(PropEnd, SEEK_SET);
    }
}

void CScriptCooker::WriteLayerMP1(CScriptLayer *pLayer)
{
    u32 LayerStart = mpSCLY->Tell();
    mpSCLY->WriteByte(0); // Unknown value
    mpSCLY->WriteLong(pLayer->NumInstances());

    for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptObject *pInstance = pLayer->InstanceByIndex(iInst);
        WriteInstanceMP1(pInstance);
    }

    u32 LayerSize = mpSCLY->Tell() - LayerStart;
    u32 NumPadBytes = 32 - (LayerSize % 32);
    if (NumPadBytes == 32) NumPadBytes = 0;

    for (u32 iPad = 0; iPad < NumPadBytes; iPad++)
        mpSCLY->WriteByte(0);
}

void CScriptCooker::WriteInstanceMP1(CScriptObject *pInstance)
{
    mpSCLY->WriteByte((u8) pInstance->ObjectTypeID());

    u32 SizeOffset = mpSCLY->Tell();
    mpSCLY->WriteLong(0);
    u32 InstanceStart = mpSCLY->Tell();

    mpSCLY->WriteLong(pInstance->InstanceID());
    mpSCLY->WriteLong(pInstance->NumOutLinks());

    for (u32 iLink = 0; iLink < pInstance->NumOutLinks(); iLink++)
    {
        const SLink& rkLink = pInstance->OutLink(iLink);
        mpSCLY->WriteLong(rkLink.State);
        mpSCLY->WriteLong(rkLink.Message);
        mpSCLY->WriteLong(rkLink.ObjectID);
    }

    WriteProperty(pInstance->Properties(), false);
    u32 InstanceEnd = mpSCLY->Tell();

    mpSCLY->Seek(SizeOffset, SEEK_SET);
    mpSCLY->WriteLong(InstanceEnd - InstanceStart);
    mpSCLY->Seek(InstanceEnd, SEEK_SET);
}

void CScriptCooker::WriteLayerMP2(CScriptLayer *pLayer)
{
    u32 LayerStart = mpSCLY->Tell();
    mpSCLY->WriteByte(0x1);
    mpSCLY->WriteLong(pLayer->NumInstances());

    for (u32 iInst = 0; iInst < pLayer->NumInstances(); iInst++)
    {
        CScriptObject *pInstance = pLayer->InstanceByIndex(iInst);
        WriteInstanceMP2(pInstance);
    }

    if (mVersion == eEchoesDemo)
    {
        u32 LayerSize = mpSCLY->Tell() - LayerStart;
        u32 NumPadBytes = 32 - (LayerSize % 32);
        if (NumPadBytes == 32) NumPadBytes = 0;

        for (u32 iPad = 0; iPad < NumPadBytes; iPad++)
            mpSCLY->WriteByte(0);
    }
}

void CScriptCooker::WriteInstanceMP2(CScriptObject *pInstance)
{
    mpSCLY->WriteLong(pInstance->ObjectTypeID());

    u32 SizeOffset = mpSCLY->Tell();
    mpSCLY->WriteShort(0);
    u32 InstanceStart = mpSCLY->Tell();

    mpSCLY->WriteLong(pInstance->InstanceID());
    mpSCLY->WriteShort((u16) pInstance->NumOutLinks());

    for (u32 iLink = 0; iLink < pInstance->NumOutLinks(); iLink++)
    {
        const SLink& rkLink = pInstance->OutLink(iLink);
        mpSCLY->WriteLong(rkLink.State);
        mpSCLY->WriteLong(rkLink.Message);
        mpSCLY->WriteLong(rkLink.ObjectID);
    }

    WriteProperty(pInstance->Properties(), false);
    u32 InstanceEnd = mpSCLY->Tell();

    mpSCLY->Seek(SizeOffset, SEEK_SET);
    mpSCLY->WriteShort((u16) (InstanceEnd - InstanceStart));
    mpSCLY->Seek(InstanceEnd, SEEK_SET);
}

// ************ STATIC ************
void CScriptCooker::WriteLayer(EGame Game, CScriptLayer *pLayer, IOutputStream& rOut)
{
    CScriptCooker Cooker;
    Cooker.mpSCLY = &rOut;
    Cooker.mVersion = Game;

    if (Game <= ePrime)
        Cooker.WriteLayerMP1(pLayer);
    else
        Cooker.WriteLayerMP2(pLayer);
}
