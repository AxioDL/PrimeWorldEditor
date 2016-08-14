#include "CUnsupportedParticleLoader.h"

// ************ PARAMETER LOADING ************
bool CUnsupportedParticleLoader::ParseParticleParameter(IInputStream& rPART)
{
    u32 ParamOffset = rPART.Tell();
    CFourCC Param = rPART.ReadLong();
    if (Param == kParamEND)
        return false;

    switch (Param.ToLong())
    {
    // Bool
    case kGenAAPH:
    case kGenCIND:
    case kGenFXLL:
    case kGenLINE:
    case kGenLIT_:
    case kGenMBLR:
    case kGenOPTS:
    case kGenORNT:
    case kGenPMAB:
    case kGenPMOO:
    case kGenPMUS:
    case kGenRSOP:
    case kGenSORT:
    case kGenVMD1:
    case kGenVMD2:
    case kGenVMD3:
    case kGenVMD4:
    case kGenZBUF:
        ParseBoolFunction(rPART);
        break;

    // Int
    case kGenCSSD:
    case kGenLFOT:
    case kGenLTME:
    case kGenLTYP:
    case kGenMAXP:
    case kGenMBSP:
    case kGenNCSY:
    case kGenNDSY:
    case kGenPISY:
    case kGenPSIV:
    case kGenPSLT:
    case kGenPSWT:
    case kGenSEED:
    case kGenSESD:
    case kGenSISY:
    case kGenSSSD:
        ParseIntFunction(rPART);
        break;

    // Float
    case kGenADV1:
    case kGenADV2:
    case kGenADV3:
    case kGenADV4:
    case kGenADV5:
    case kGenADV6:
    case kGenADV7:
    case kGenADV8:
    case kGenGRTE:
    case kGenLENG:
    case kGenLFOR:
    case kGenLINT:
    case kGenLSLA:
    case kGenPSTS:
    case kGenROTA:
    case kGenSIZE:
    case kGenWIDT:
        ParseFloatFunction(rPART);
        break;

    // Vector
    case kGenILOC:
    case kGenIVEC:
    case kGenLDIR:
    case kGenLOFF:
    case kGenPMOP:
    case kGenPMRT:
    case kGenPMSC:
    case kGenPOFS:
    case kGenPSOV:
    case kGenSEPO:
    case kGenSSPO:
        ParseVectorFunction(rPART);
        break;

    // Mod Vector
    case kGenPSVM:
    case kGenVEL1:
    case kGenVEL2:
    case kGenVEL3:
    case kGenVEL4:
        ParseModVectorFunction(rPART);
        break;

    // Color
    case kGenCOLR:
    case kGenLCLR:
    case kGenPMCL:
        ParseColorFunction(rPART);
        break;

    // UV
    case kGenTEXR:
    case kGenTIND:
        ParseUVFunction(rPART);
        break;

    // Emitter
    case kGenEMTR:
        ParseEmitterFunction(rPART);
        break;

    // Spawn System Keyframe Data
    case kGenKSSM:
        ParseSpawnSystemKeyframeData(rPART);
        break;

    // Asset
    case kGenICTS:
    case kGenIDTS:
    case kGenIITS:
    case kGenPMDL:
    case kGenSELC:
    case kGenSSWH:
        ParseAssetFunction(rPART);
        break;

    default:
        Log::FileError(rPART.GetSourceString(), ParamOffset, "Unknown PART parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseElectricParameter(IInputStream& rELSC)
{
    u32 ParamOffset = rELSC.Tell();
    CFourCC Param = rELSC.ReadLong();
    if (Param == kParamEND) return false;

    switch (Param.ToLong())
    {
    case kElecZERY:
        ParseBoolFunction(rELSC);
        break;

    case kElecLIFE:
    case kElecSCNT:
    case kElecSLIF:
    case kElecSSEG:
        ParseIntFunction(rELSC);
        break;

    case kElecAMPD:
    case kElecAMPL:
    case kElecGRAT:
    case kElecLWD1:
    case kElecLWD2:
    case kElecLWD3:
        ParseFloatFunction(rELSC);
        break;

    case kElecCOLR:
    case kElecLCL1:
    case kElecLCL2:
    case kElecLCL3:
        ParseColorFunction(rELSC);
        break;

    case kElecFEMT:
    case kElecIEMT:
        ParseEmitterFunction(rELSC);
        break;

    case kElecEPSM:
    case kElecGPSM:
    case kElecSSWH:
        ParseAssetFunction(rELSC);
        break;

    default:
        Log::FileError(rELSC.GetSourceString(), ParamOffset, "Unknown ELSC parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseSwooshParameter(IInputStream& rSWHC)
{
    u32 ParamOffset = rSWHC.Tell();
    CFourCC Param = rSWHC.ReadLong();
    if (Param == kParamEND) return false;

    switch (Param.ToLong())
    {
    case kSwooshAALP:
    case kSwooshCRND:
    case kSwooshCROS:
    case kSwooshLLRD:
    case kSwooshORNT:
    case kSwooshSROT:
    case kSwooshTEXW:
    case kSwooshVLS1:
    case kSwooshVLS2:
    case kSwooshWIRE:
    case kSwooshZBUF:
        ParseBoolFunction(rSWHC);
        break;

    case kSwooshLENG:
    case kSwooshPSLT:
    case kSwooshSIDE:
        ParseIntFunction(rSWHC);
        break;

    case kSwooshIROT:
    case kSwooshLRAD:
    case kSwooshROTM:
    case kSwooshRRAD:
    case kSwooshSPLN:
    case kSwooshTIME:
    case kSwooshTSPN:
        ParseFloatFunction(rSWHC);
        break;

    case kSwooshIVEL:
    case kSwooshNPOS:
    case kSwooshPOFS:
        ParseVectorFunction(rSWHC);
        break;

    case kSwooshVELM:
    case kSwooshVLM2:
        ParseModVectorFunction(rSWHC);
        break;

    case kSwooshCOLR:
        ParseColorFunction(rSWHC);
        break;

    case kSwooshTEXR:
        ParseUVFunction(rSWHC);
        break;

    default:
        Log::FileError(rSWHC.GetSourceString(), ParamOffset, "Unknown SWHC parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseDecalParameter(IInputStream& rDPSC)
{
    u32 ParamOffset = rDPSC.Tell();
    CFourCC Param = rDPSC.ReadLong();
    if (Param == kParamEND) return false;

    switch (Param.ToLong())
    {
    case kDecal1ADD:
    case kDecal2ADD:
    case kDecal1OFF:
    case kDecal2OFF:
    case kDecalDMAB:
    case kDecalDMCL:
    case kDecalDMDL:
    case kDecalDMOO:
    case kDecalDMOP:
    case kDecalDMRT:
    case kDecalDMSC:
        ParseBoolFunction(rDPSC);
        break;

    case kDecal1LFT:
    case kDecal2LFT:
    case kDecalDLFT:
        ParseIntFunction(rDPSC);
        break;

    case kDecal1ROT:
    case kDecal2ROT:
    case kDecal1SZE:
    case kDecal2SZE:
        ParseFloatFunction(rDPSC);
        break;

    case kDecal1CLR:
    case kDecal2CLR:
        ParseColorFunction(rDPSC);
        break;

    case kDecal1TEX:
    case kDecal2TEX:
        if (rDPSC.ReadLong() == kFuncNONE) break;
        ParseAssetFunction(rDPSC);
        break;

    default:
        Log::FileError(rDPSC.GetSourceString(), ParamOffset, "Unknown DPSC parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseWeaponParameter(IInputStream& rWPSC)
{
    u32 ParamOffset = rWPSC.Tell();
    CFourCC Param = rWPSC.ReadLong();
    if (Param == kParamEND) return false;

    switch (Param.ToLong())
    {
    case kWeaponAPSO:
    case kWeaponAP11:
    case kWeaponAP21:
    case kWeaponAS11:
    case kWeaponAS12:
    case kWeaponAS13:
    case kWeaponEWTR:
    case kWeaponFC60:
    case kWeaponHOMG:
    case kWeaponLWTR:
    case kWeaponSPS1:
    case kWeaponSPS2:
    case kWeaponSWTR:
    case kWeaponVMD2:
        ParseBoolFunction(rWPSC);
        break;

    case kWeaponPJFX:
    case kWeaponPSLT:
        ParseIntFunction(rWPSC);
        break;

    case kWeaponRNGE:
    case kWeaponTRAT:
        ParseFloatFunction(rWPSC);
        break;

    case kWeaponIORN:
    case kWeaponIVEC:
    case kWeaponOFST:
    case kWeaponPOFS:
    case kWeaponPSCL:
    case kWeaponPSOV:
        ParseVectorFunction(rWPSC);
        break;

    case kWeaponPSVM:
        ParseModVectorFunction(rWPSC);
        break;

    case kWeaponPCOL:
        ParseColorFunction(rWPSC);
        break;

    case kWeaponAPSM:
    case kWeaponAPS1:
    case kWeaponAPS2:
    case kWeaponASW1:
    case kWeaponASW2:
    case kWeaponASW3:
    case kWeaponCOLR:
    case kWeaponOHEF:
        ParseAssetFunction(rWPSC);
        break;

    default:
        Log::FileError(rWPSC.GetSourceString(), ParamOffset, "Unknown WPSC parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseCollisionResponseParameter(IInputStream& rCRSC)
{
    u32 ParamOffset = rCRSC.Tell();
    CFourCC Param = rCRSC.ReadLong();
    if (Param == kParamEND) return false;

    // CRSC has way too many useless extra parameters that are never used to bother typing out, so just skip past them
    u32 FuncPeek = rCRSC.PeekLong();

    if (FuncPeek == kFuncNONE)
    {
        rCRSC.Seek(0x4, SEEK_CUR);
        return true;
    }

    switch (Param.ToLong())
    {
    case kColi1ATA:
    case kColi2ATA:
    case kColi3ATA:
    case kColi4ATA:
    case kColi5ATA:
    case kColi6ATA:
    case kColi1ATB:
    case kColi2ATB:
    case kColi3ATB:
    case kColi4ATB:
    case kColi5ATB:
    case kColi6ATB:
    case kColi1BSE:
    case kColi2BSE:
    case kColi1DRN:
    case kColi2DRN:
    case kColi3DRN:
    case kColi4DRN:
    case kColi5DRN:
    case kColi6DRN:
    case kColi2MUD:
    case kColi2SAN:
    case kColiBHFX:
    case kColiCHFX:
    case kColiCSFX:
    case kColiCZFX:
    case kColiDCSH:
    case kColiDSFX:
    case kColiDSHX:
    case kColiGOFX:
    case kColiGOOO:
    case kColiGRAS:
    case kColiGRFX:
    case kColiHBFX:
    case kColiICEE:
    case kColiICFX:
    case kColiMSFX:
    case kColiPBHX:
    case kColiPBOS:
    case kColiPBSX:
    case kColiSHFX:
    case kColiTAFX:
    case kColiTASP:
    case kColiWSFX:
    case kColiWTFX:
        ParseIntFunction(rCRSC);
        break;

    case kColiFOFF:
    case kColiRNGE:
        ParseFloatFunction(rCRSC);
        break;

    case kColiCHDL:
    case kColiCODL:
    case kColiDCHR:
    case kColiDDCL:
    case kColiDEFS:
    case kColiDENM:
    case kColiDESH:
    case kColiENDL:
    case kColiGODL:
    case kColiGRDL:
    case kColiICDL:
    case kColiMEDL:
    case kColiTALP:
    case kColiWODL:
    case kColiWTDL:
        ParseAssetFunction(rCRSC);
        break;

    default:
        Log::FileError(rCRSC.GetSourceString(), ParamOffset, "Unknown CRSC parameter: " + Param.ToString());
        DEBUG_BREAK;
        return false;
    }

    return true;
}

// ************ FUNCTION LOADING ************
void CUnsupportedParticleLoader::ParseBoolFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kBoolCNST:
        rFile.Seek(0x1, SEEK_CUR);
        break;

    case kFuncNONE:
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown bool function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseIntFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
    case kIntGAPC:
    case kIntGEMT:
    case kIntGTCP:
        break;

    case kIntILPT:
    case kIntIMPL:
        ParseIntFunction(rFile);
        break;

    case kIntADD_:
    case kIntDETH:
    case kIntIRND:
    case kIntMODU:
    case kIntMULT:
    case kIntRAND:
    case kIntSUB_:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kIntCHAN:
    case kIntCLMP:
    case kIntSPAH:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kIntPULS:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kIntCNST:
    {
        u32 Value = rFile.ReadLong();
        ASSERT(gpResourceStore->FindEntry(Value) == nullptr);
        break;
    }

    case kIntKEYE:
    case kIntKEYP:
        ParseKeyframeEmitterData(rFile, 0x4);
        break;

    case kIntTSCL:
        ParseFloatFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown int function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseFloatFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
    case kFloatPAP1:
    case kFloatPAP2:
    case kFloatPAP3:
    case kFloatPAP4:
    case kFloatPAP5:
    case kFloatPAP6:
    case kFloatPAP7:
    case kFloatPAP8:
    case kFloatPRLW:
    case kFloatPSLL:
        break;

    case kFloatRLPT:
    case kFloatSCAL:
        ParseFloatFunction(rFile);
        break;

    case kFloatADD_:
    case kFloatIRND:
    case kFloatISWT:
    case kFloatLFTW:
    case kFloatMULT:
    case kFloatRAND:
    case kFloatSUB_:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatCEQL:
    case kFloatCLTN:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatCEXT:
        ParseIntFunction(rFile);
        break;

    case kFloatCHAN:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kFloatCLMP:
    case kFloatSINE:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatCNST:
        rFile.Seek(0x4, SEEK_CUR);
        break;

    case kFloatCRNG:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatDOTP:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case kFloatGTCR:
    case kFloatGTCG:
    case kFloatGTCB:
    case kFloatGTCA:
        ParseColorFunction(rFile);
        break;

    case kFloatITRL:
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatKEYE:
    case kFloatKEYP:
        ParseKeyframeEmitterData(rFile, 0x4);
        break;

    case kFloatPULS:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kFloatVMAG:
    case kFloatVXTR:
    case kFloatVYTR:
    case kFloatVZTR:
        ParseVectorFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown float function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseVectorFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kVectorADD_:
    case kVectorMULT:
    case kVectorSUB_:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case kVectorANGC:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kVectorCCLU:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kVectorCHAN:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kVectorCIRC:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kVectorCNST:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kVectorCONE:
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kVectorCTVC:
        ParseColorFunction(rFile);
        break;

    case kVectorKEYE:
    case kVectorKEYP:
        ParseKeyframeEmitterData(rFile, 0xC);
        break;

    case kFuncNONE:
    case kVectorPLCO:
    case kVectorPLOC:
    case kVectorPSOF:
    case kVectorPSOU:
    case kVectorPSOR:
    case kVectorPSTR:
    case kVectorPVEL:
        break;

    case kVectorPULS:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case kVectorRTOV:
        ParseFloatFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown vector function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseModVectorFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
        break;

    case kModVectorBNCE:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseBoolFunction(rFile);
        break;

    case kModVectorCHAN:
        ParseModVectorFunction(rFile);
        ParseModVectorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kModVectorCNST:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kModVectorEMPL:
    case kModVectorIMPL:
    case kModVectorLMPL:
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseBoolFunction(rFile);
        break;

    case kModVectorEXPL:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kModVectorGRAV:
    case kModVectorSPOS:
        ParseVectorFunction(rFile);
        break;

    case kModVectorPULS:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kModVectorSWRL:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kModVectorWIND:
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown mod vector function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseColorFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kColorCFDE:
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kColorCHAN:
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case kColorCNST:
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kColorFADE:
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kColorKEYE:
    case kColorKEYP:
        ParseKeyframeEmitterData(rFile, 0x10);
        break;

    case kFuncNONE:
    case kColorPCOL:
        break;

    case kColorPULS:
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown color function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseUVFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
        break;

    case kUVCNST:
        ParseAssetFunction(rFile);
        break;

    case kUVATEX:
        ParseAssetFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseBoolFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown UV function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseEmitterFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
        break;

    case kEmitterSETR:
        ParseParticleParameter(rFile);
        ParseParticleParameter(rFile);
        break;

    case kEmitterSEMR:
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case kEmitterSPHE:
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case kEmitterASPH:
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown emitter function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseAssetFunction(IInputStream& rFile)
{
    u32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case kFuncNONE:
        break;

    case kAssetCNST:
        mpGroup->AddDependency( CAssetID(rFile, mpGroup->Game()) );
        break;

    default:
        Log::FileError(rFile.GetSourceString(), FuncOffset, "Unknown asset function: " + Func.ToString());
        DEBUG_BREAK;
        break;
    }
}

void CUnsupportedParticleLoader::ParseSpawnSystemKeyframeData(IInputStream& rFile)
{
    CFourCC Func = rFile.ReadLong();
    if (Func == "NONE") return;
    ASSERT(Func == "CNST");

    rFile.Seek(0x10, SEEK_CUR); // Skip unneeded values
    u32 Count = rFile.ReadLong();

    for (u32 iKey = 0; iKey < Count; iKey++)
    {
        rFile.Seek(0x4, SEEK_CUR); // Skip frame number
        u32 InfoCount = rFile.ReadLong();

        for (u32 iInfo = 0; iInfo < InfoCount; iInfo++)
        {
            mpGroup->AddDependency( CAssetID(rFile, mpGroup->Game()) );
            rFile.Seek(0xC, SEEK_CUR); // Skip unknown/unneeded values
        }
    }
}

void CUnsupportedParticleLoader::ParseKeyframeEmitterData(IInputStream& rFile, u32 ElemSize)
{
    rFile.Seek(0x12, SEEK_CUR); // Skip unneeded values
    u32 KeyCount = rFile.ReadLong();
    rFile.Seek(KeyCount * ElemSize, SEEK_CUR);
}

// ************ STATIC ************
enum {
    kGPSM = FOURCC_CONSTEXPR('G', 'P', 'S', 'M'),
    kELSM = FOURCC_CONSTEXPR('E', 'L', 'S', 'M'),
    kSWSH = FOURCC_CONSTEXPR('S', 'W', 'S', 'H'),
    kDPSM = FOURCC_CONSTEXPR('D', 'P', 'S', 'M'),
    kWPSM = FOURCC_CONSTEXPR('W', 'P', 'S', 'M'),
    kCRSM = FOURCC_CONSTEXPR('C', 'R', 'S', 'M')
};

CDependencyGroup* CUnsupportedParticleLoader::LoadParticle(IInputStream& rFile, CResourceEntry *pEntry)
{
    CFourCC Magic = rFile.ReadLong();

    // Loop through particle functions
    CUnsupportedParticleLoader Loader;
    Loader.mpGroup = new CDependencyGroup(pEntry);

    while (true)
    {
        bool ShouldContinue = false;

        switch (Magic.ToLong())
        {
        case kGPSM: ShouldContinue = Loader.ParseParticleParameter(rFile);          break;
        case kELSM: ShouldContinue = Loader.ParseElectricParameter(rFile);          break;
        case kSWSH: ShouldContinue = Loader.ParseSwooshParameter(rFile);            break;
        case kDPSM: ShouldContinue = Loader.ParseDecalParameter(rFile);             break;
        case kWPSM: ShouldContinue = Loader.ParseWeaponParameter(rFile);            break;
        case kCRSM: ShouldContinue = Loader.ParseCollisionResponseParameter(rFile); break;

        default:
            Log::Error("Unrecognized particle system magic: " + Magic.ToString());
            break;
        }

        if (!ShouldContinue) break;
    }

    return Loader.mpGroup;
}
