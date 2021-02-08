#include "CUnsupportedParticleLoader.h"
#include <Core/GameProject/CGameProject.h>

// ************ PARAMETER LOADING ************
bool CUnsupportedParticleLoader::ParseParticleParameter(IInputStream& rPART)
{
    uint32 ParamOffset = rPART.Tell();
    CFourCC Param = rPART.ReadLong();
    if (Param == FOURCC('_END'))
        return false;

    switch (Param.ToLong())
    {
    // Bool Constant
    case FOURCC('AAPH'):
    case FOURCC('CIND'):
    case FOURCC('DBPS'):
    case FOURCC('DDIT'):
    case FOURCC('DFOG'):
    case FOURCC('EMTU'):
    case FOURCC('FXBI'):
    case FOURCC('FXBT'):
    case FOURCC('FXLL'):
    case FOURCC('INDM'):
    case FOURCC('INDP'):
    case FOURCC('ITPE'):
    case FOURCC('LINE'):
    case FOURCC('LIT_'):
    case FOURCC('MBLR'):
    case FOURCC('NTIK'):
    case FOURCC('ONET'):
    case FOURCC('OPTS'):
    case FOURCC('ORNT'):
    case FOURCC('ORTC'):
    case FOURCC('PMAB'):
    case FOURCC('PMLT'):
    case FOURCC('PMOO'):
    case FOURCC('PMTF'):
    case FOURCC('PMUS'):
    case FOURCC('RDOP'):
    case FOURCC('RSOP'):
    case FOURCC('SCNV'):
    case FOURCC('SORT'):
    case FOURCC('STOP'):
    case FOURCC('VGD1'):
    case FOURCC('VGD2'):
    case FOURCC('VGD3'):
    case FOURCC('VGD4'):
    case FOURCC('VMD1'):
    case FOURCC('VMD2'):
    case FOURCC('VMD3'):
    case FOURCC('VMD4'):
    case FOURCC('VMPC'):
    case FOURCC('ZBUF'):
        ParseBool(rPART);
        break;

    // Bitfield
    case FOURCC('DFLG'):
        ParseBitfieldFunction(rPART);
        break;

    // Int
    case FOURCC('ALSC'):
    case FOURCC('AMSC'):
    case FOURCC('CSSD'):
    case FOURCC('HJAK'):
    case FOURCC('IAV0'):
    case FOURCC('IAV1'):
    case FOURCC('IAV2'):
    case FOURCC('IAV3'):
    case FOURCC('LFOT'):
    case FOURCC('LTME'):
    case FOURCC('LTYP'):
    case FOURCC('MAXP'):
    case FOURCC('MBDM'):
    case FOURCC('MBSP'):
    case FOURCC('MSLT'):
    case FOURCC('NCSY'):
    case FOURCC('NDSY'):
    case FOURCC('PBDM'):
    case FOURCC('PISY'):
    case FOURCC('PSLT'):
    case FOURCC('PSWT'):
    case FOURCC('PWGT'):
    case FOURCC('SCTR'):
    case FOURCC('SEED'):
    case FOURCC('SESD'):
    case FOURCC('SISY'):
    case FOURCC('SSSD'):
    case FOURCC('SSWT'):
    case FOURCC('SVI0'):
    case FOURCC('SVI1'):
    case FOURCC('VSPC'):
    case FOURCC('XJAK'):
    case FOURCC('XTAD'):
        ParseIntFunction(rPART);
        break;

    // Float
    case FOURCC('ADV1'):
    case FOURCC('ADV2'):
    case FOURCC('ADV3'):
    case FOURCC('ADV4'):
    case FOURCC('ADV5'):
    case FOURCC('ADV6'):
    case FOURCC('ADV7'):
    case FOURCC('ADV8'):
    case FOURCC('ADV9'):
    case FOURCC('DBIS'):
    case FOURCC('EADV'):
    case FOURCC('FXBR'):
    case FOURCC('GRTE'):
    case FOURCC('ISVF'):
    case FOURCC('LENG'):
    case FOURCC('LFOR'):
    case FOURCC('LINT'):
    case FOURCC('LSLA'):
    case FOURCC('PSTS'):
    case FOURCC('ROTA'):
    case FOURCC('SIZE'):
    case FOURCC('SVEO'):
    case FOURCC('SVR0'):
    case FOURCC('SVR1'):
    case FOURCC('SVR2'):
    case FOURCC('WIDT'):
        ParseFloatFunction(rPART);
        break;

    // Vector
    case FOURCC('FXBM'):
    case FOURCC('FXBO'):
    case FOURCC('ILOC'):
    case FOURCC('IVEC'):
    case FOURCC('LDIR'):
    case FOURCC('LOFF'):
    case FOURCC('PMOP'):
    case FOURCC('PMOV'):
    case FOURCC('PMPV'):
    case FOURCC('PMRT'):
    case FOURCC('PMSC'):
    case FOURCC('POFS'):
    case FOURCC('PSIV'):
    case FOURCC('PSOV'):
    case FOURCC('SAXS'):
    case FOURCC('SEPO'):
    case FOURCC('SSPO'):
    case FOURCC('SVV0'):
    case FOURCC('SVV1'):
    case FOURCC('VAV1'):
    case FOURCC('VAV2'):
    case FOURCC('VAV3'):
    case FOURCC('VAV4'):
        ParseVectorFunction(rPART);
        break;

    // Mod Vector
    case FOURCC('PSVM'):
    case FOURCC('VEL1'):
    case FOURCC('VEL2'):
    case FOURCC('VEL3'):
    case FOURCC('VEL4'):
        ParseModVectorFunction(rPART);
        break;

    // Color
    case FOURCC('CAV0'):
    case FOURCC('CAV1'):
    case FOURCC('CAV2'):
    case FOURCC('CAV3'):
    case FOURCC('COLR'):
    case FOURCC('LCLR'):
    case FOURCC('PMCL'):
    case FOURCC('SVC0'):
    case FOURCC('SVC1'):
        ParseColorFunction(rPART);
        break;

    // UV
    case FOURCC('TEXR'):
    case FOURCC('TIND'):
        ParseUVFunction(rPART);
        break;

    // Emitter
    case FOURCC('EMTR'):
        ParseEmitterFunction(rPART);
        break;

    // Spawn System Keyframe Data
    case FOURCC('KSSM'):
        ParseSpawnSystemKeyframeData(rPART);
        break;

    // Asset
    case FOURCC('ICTS'):
    case FOURCC('IDTS'):
    case FOURCC('IITS'):
    case FOURCC('MDL1'):
    case FOURCC('MDL2'):
    case FOURCC('MDL3'):
    case FOURCC('MDL4'):
    case FOURCC('MDL5'):
    case FOURCC('MDL6'):
    case FOURCC('MDL7'):
    case FOURCC('MDL8'):
    case FOURCC('MDL9'):
    case FOURCC('PMDL'):
    case FOURCC('SELC'):
    case FOURCC('SSWH'):
        ParseAssetFunction(rPART);
        break;

    default:
        errorf("%s [0x%X]: Unknown PART parameter: %s", *rPART.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseElectricParameter(IInputStream& rELSC)
{
    uint32 ParamOffset = rELSC.Tell();
    CFourCC Param = rELSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('ZERY'):
        ParseBool(rELSC);
        break;

    case FOURCC('DFLG'):
        ParseBitfieldFunction(rELSC);
        break;

    case FOURCC('BLDM'):
    case FOURCC('LIFE'):
    case FOURCC('SCNT'):
    case FOURCC('SLIF'):
    case FOURCC('SSEG'):
        ParseIntFunction(rELSC);
        break;

    case FOURCC('AMPD'):
    case FOURCC('AMPL'):
    case FOURCC('GRAT'):
    case FOURCC('LWD1'):
    case FOURCC('LWD2'):
    case FOURCC('LWD3'):
        ParseFloatFunction(rELSC);
        break;

    case FOURCC('COLR'):
    case FOURCC('LCL1'):
    case FOURCC('LCL2'):
    case FOURCC('LCL3'):
        ParseColorFunction(rELSC);
        break;

    case FOURCC('TEXR'):
        ParseUVFunction(rELSC);
        break;

    case FOURCC('FEMT'):
    case FOURCC('IEMT'):
        ParseEmitterFunction(rELSC);
        break;

    case FOURCC('EPSM'):
    case FOURCC('GPSM'):
    case FOURCC('SSWH'):
        ParseAssetFunction(rELSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown ELSC parameter: %s", *rELSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseSortedParameter(IInputStream& rSRSC)
{
    uint32 ParamOffset = rSRSC.Tell();
    CFourCC Param = rSRSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    if (Param == FOURCC('SPWN'))
        ParseSpawnSystemKeyframeData(rSRSC);

    else
    {
        errorf("%s [0x%X]: Unknown SRSC parameter: %s", *rSRSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseSpawnParameter(IInputStream& rSPSC)
{
    uint32 ParamOffset = rSPSC.Tell();
    CFourCC Param = rSPSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('DEOL'):
    case FOURCC('FRCO'):
    case FOURCC('IGGT'):
    case FOURCC('IGLT'):
    case FOURCC('VMD1'):
    case FOURCC('VMD2'):
        ParseBool(rSPSC);
        break;

    case FOURCC('GIVL'):
    case FOURCC('PSLT'):
        ParseIntFunction(rSPSC);
        break;

    case FOURCC('VBLN'):
        ParseFloatFunction(rSPSC);
        break;

    case FOURCC('FROV'):
    case FOURCC('GORN'):
    case FOURCC('GTRN'):
    case FOURCC('IVEC'):
    case FOURCC('LSCL'):
    case FOURCC('ORNT'):
    case FOURCC('SCLE'):
    case FOURCC('TRNL'):
        ParseVectorFunction(rSPSC);
        break;

    case FOURCC('VLM1'):
    case FOURCC('VLM2'):
        ParseModVectorFunction(rSPSC);
        break;

    case FOURCC('PCOL'):
        ParseColorFunction(rSPSC);
        break;

    case FOURCC('SPWN'):
        ParseSpawnSystemKeyframeData(rSPSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown SPSC parameter: %s", *rSPSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseSwooshParameter(IInputStream& rSWHC)
{
    uint32 ParamOffset = rSWHC.Tell();
    CFourCC Param = rSWHC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('AALP'):
    case FOURCC('CLTX'):
    case FOURCC('CRND'):
    case FOURCC('CROS'):
    case FOURCC('LLRD'):
    case FOURCC('ORNT'):
    case FOURCC('PTMG'):
    case FOURCC('SCNV'):
    case FOURCC('SROT'):
    case FOURCC('TEXW'):
    case FOURCC('VLS1'):
    case FOURCC('VLS2'):
    case FOURCC('WIRE'):
    case FOURCC('ZBUF'):
        ParseBool(rSWHC);
        break;

    case FOURCC('DFLG'):
        ParseBitfieldFunction(rSWHC);
        break;

    case FOURCC('ALSC'):
    case FOURCC('LENG'):
    case FOURCC('PSLT'):
    case FOURCC('SBDM'):
    case FOURCC('SIDE'):
    case FOURCC('SPLN'):
    case FOURCC('TSPN'):
        ParseIntFunction(rSWHC);
        break;

    case FOURCC('IROT'):
    case FOURCC('LRAD'):
    case FOURCC('ROTM'):
    case FOURCC('RRAD'):
    case FOURCC('TIME'):
    case FOURCC('XROT'):
    case FOURCC('YROT'):
    case FOURCC('ZROT'):
        ParseFloatFunction(rSWHC);
        break;

    case FOURCC('IVEL'):
    case FOURCC('NPOS'):
    case FOURCC('POFS'):
        ParseVectorFunction(rSWHC);
        break;

    case FOURCC('VELM'):
    case FOURCC('VLM2'):
        ParseModVectorFunction(rSWHC);
        break;

    case FOURCC('COLR'):
        ParseColorFunction(rSWHC);
        break;

    case FOURCC('TEXR'):
        ParseUVFunction(rSWHC);
        break;

    default:
        errorf("%s [0x%X]: Unknown SWHC parameter: %s", *rSWHC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseDecalParameter(IInputStream& rDPSC)
{
    uint32 ParamOffset = rDPSC.Tell();
    CFourCC Param = rDPSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('1ADD'):
    case FOURCC('2ADD'):
    case FOURCC('DMAB'):
    case FOURCC('DMOO'):
        ParseBool(rDPSC);
        break;

    case FOURCC('1LFT'):
    case FOURCC('2LFT'):
    case FOURCC('DLFT'):
        ParseIntFunction(rDPSC);
        break;

    case FOURCC('1ROT'):
    case FOURCC('2ROT'):
    case FOURCC('1SZE'):
    case FOURCC('2SZE'):
        ParseFloatFunction(rDPSC);
        break;

    case FOURCC('1OFF'):
    case FOURCC('2OFF'):
    case FOURCC('DMOP'):
    case FOURCC('DMRT'):
    case FOURCC('DMSC'):
        ParseVectorFunction(rDPSC);
        break;

    case FOURCC('1CLR'):
    case FOURCC('2CLR'):
    case FOURCC('DMCL'):
        ParseColorFunction(rDPSC);
        break;

    case FOURCC('1TEX'):
    case FOURCC('2TEX'):
        ParseUVFunction(rDPSC);
        break;

    case FOURCC('DMDL'):
        ParseAssetFunction(rDPSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown DPSC parameter: %s", *rDPSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseWeaponParameter(IInputStream& rWPSC)
{
    uint32 ParamOffset = rWPSC.Tell();
    CFourCC Param = rWPSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('APSO'):
    case FOURCC('AP11'):
    case FOURCC('AP21'):
    case FOURCC('AS11'):
    case FOURCC('AS12'):
    case FOURCC('AS13'):
    case FOURCC('BEAM'):
    case FOURCC('BHBT'):
    case FOURCC('DP1C'):
    case FOURCC('DP2C'):
    case FOURCC('EELT'):
    case FOURCC('EWTR'):
    case FOURCC('F60H'):
    case FOURCC('FC60'):
    case FOURCC('HOMG'):
    case FOURCC('LWTR'):
    case FOURCC('NDTT'):
    case FOURCC('RB1A'):
    case FOURCC('RB2A'):
    case FOURCC('RTLA'):
    case FOURCC('RWPE'):
    case FOURCC('SPS1'):
    case FOURCC('SPS2'):
    case FOURCC('SREL'):
    case FOURCC('SVBD'):
    case FOURCC('SWTR'):
    case FOURCC('TCND'):
    case FOURCC('VMD2'):
        ParseBool(rWPSC);
        break;
        
    case FOURCC('PSLT'):
        ParseIntFunction(rWPSC);
        break;

    case FOURCC('B1RT'):
    case FOURCC('B1SE'):
    case FOURCC('B2RT'):
    case FOURCC('B2SE'):
    case FOURCC('FADD'):
    case FOURCC('FADS'):
    case FOURCC('FOFF'):
    case FOURCC('RNGE'):
    case FOURCC('TLEN'):
    case FOURCC('TRAT'):
    case FOURCC('TSZE'):
        ParseFloatFunction(rWPSC);
        break;

    case FOURCC('B1PO'):
    case FOURCC('B2PO'):
    case FOURCC('IORN'):
    case FOURCC('IVEC'):
    case FOURCC('OFST'):
    case FOURCC('POFS'):
    case FOURCC('PSCL'):
    case FOURCC('PSOV'):
    case FOURCC('TLPO'):
        ParseVectorFunction(rWPSC);
        break;

    case FOURCC('PSVM'):
        ParseModVectorFunction(rWPSC);
        break;

    case FOURCC('B1CL'):
    case FOURCC('B2CL'):
    case FOURCC('PCOL'):
    case FOURCC('TECL'):
    case FOURCC('TSCL'):
        ParseColorFunction(rWPSC);
        break;

    case FOURCC('B1TX'):
    case FOURCC('B2TX'):
    case FOURCC('TTEX'):
        ParseUVFunction(rWPSC);
        break;
        
    case FOURCC('PJFX'):
        ParseSoundFunction(rWPSC);
        break;

    case FOURCC('APSM'):
    case FOURCC('APS1'):
    case FOURCC('APS2'):
    case FOURCC('ASW1'):
    case FOURCC('ASW2'):
    case FOURCC('ASW3'):
    case FOURCC('COLR'):
    case FOURCC('OHEF'):
    case FOURCC('PSFX'):
        ParseAssetFunction(rWPSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown WPSC parameter: %s", *rWPSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseCollisionResponseParameter(IInputStream& rCRSC)
{
    uint32 ParamOffset = rCRSC.Tell();
    CFourCC Param = rCRSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    // CRSC has way too many useless extra parameters that are never used to bother typing out, so just skip past them
    uint32 FuncPeek = rCRSC.PeekLong();

    if (FuncPeek == FOURCC('NONE'))
    {
        rCRSC.Seek(0x4, SEEK_CUR);
        return true;
    }

    switch (Param.ToLong())
    {
    case FOURCC('1ATA'):
    case FOURCC('2ATA'):
    case FOURCC('3ATA'):
    case FOURCC('4ATA'):
    case FOURCC('5ATA'):
    case FOURCC('6ATA'):
    case FOURCC('1ATB'):
    case FOURCC('2ATB'):
    case FOURCC('3ATB'):
    case FOURCC('4ATB'):
    case FOURCC('5ATB'):
    case FOURCC('6ATB'):
    case FOURCC('1BSE'):
    case FOURCC('2BSE'):
    case FOURCC('1DRN'):
    case FOURCC('2DRN'):
    case FOURCC('3DRN'):
    case FOURCC('4DRN'):
    case FOURCC('5DRN'):
    case FOURCC('6DRN'):
    case FOURCC('6GRN'):
    case FOURCC('2MUD'):
    case FOURCC('2SAN'):
    case FOURCC('DCSH'):
    case FOURCC('DSHX'):
    case FOURCC('PBHX'):
    case FOURCC('PBOS'):
    case FOURCC('PBSX'):
    case FOURCC('TASP'):
        ParseIntFunction(rCRSC);
        break;

    case FOURCC('FOFF'):
    case FOURCC('RNGE'):
        ParseFloatFunction(rCRSC);
        break;
        
    case FOURCC('BHFX'):
    case FOURCC('CHFX'):
    case FOURCC('CSFX'):
    case FOURCC('CZFX'):
    case FOURCC('DSFX'):
    case FOURCC('GOFX'):
    case FOURCC('GRFX'):
    case FOURCC('HBFX'):
    case FOURCC('ICFX'):
    case FOURCC('MSFX'):
    case FOURCC('SHFX'):
    case FOURCC('TAFX'):
    case FOURCC('WSFX'):
    case FOURCC('WTFX'):
        ParseSoundFunction(rCRSC);
        break;
        
    case FOURCC('1LAV'):
    case FOURCC('3LAV'):
    case FOURCC('1MUD'):
    case FOURCC('3MUD'):
    case FOURCC('1PRJ'):
    case FOURCC('3PRJ'):
    case FOURCC('1SAN'):
    case FOURCC('3SAN'):
    case FOURCC('1WOD'):
    case FOURCC('3WOD'):
    case FOURCC('CHDL'):
    case FOURCC('CODL'):
    case FOURCC('CRTS'):
    case FOURCC('CSSD'):
    case FOURCC('DCHR'):
    case FOURCC('DCSD'):
    case FOURCC('DDCL'):
    case FOURCC('DEFS'):
    case FOURCC('DENM'):
    case FOURCC('DESD'):
    case FOURCC('DESH'):
    case FOURCC('DSND'):
    case FOURCC('DSSD'):
    case FOURCC('ENDL'):
    case FOURCC('GLAS'):
    case FOURCC('GLDL'):
    case FOURCC('GODL'):
    case FOURCC('GOOO'):
    case FOURCC('GRAS'):
    case FOURCC('GRDL'):
    case FOURCC('ICDL'):
    case FOURCC('ICEE'):
    case FOURCC('MEDL'):
    case FOURCC('MTLS'):
    case FOURCC('P3SD'):
    case FOURCC('PLAY'):
    case FOURCC('PLSD'):
    case FOURCC('PLY3'):
    case FOURCC('SNDL'):
    case FOURCC('SNEE'):
    case FOURCC('TALP'):
    case FOURCC('WATR'):
    case FOURCC('WODL'):
    case FOURCC('WODS'):
    case FOURCC('WTDL'):
        ParseAssetFunction(rCRSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown CRSC parameter: %s", *rCRSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseBurstFireParameter(IInputStream& rBFRC)
{
    uint32 ParamOffset = rBFRC.Tell();
    CFourCC Param = rBFRC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('BTMI'):
        ParseIntFunction(rBFRC);
        break;

    case FOURCC('B0CH'):
    case FOURCC('B1CH'):
    case FOURCC('B2CH'):
    case FOURCC('B3CH'):
    case FOURCC('B4CH'):
    case FOURCC('B5CH'):
    case FOURCC('B6CH'):
    case FOURCC('B7CH'):
    case FOURCC('B0VL'):
    case FOURCC('B1VL'):
    case FOURCC('B2VL'):
    case FOURCC('B3VL'):
    case FOURCC('B4VL'):
    case FOURCC('B5VL'):
    case FOURCC('B6VL'):
    case FOURCC('B7VL'):
    case FOURCC('OVLH'):
    case FOURCC('OVLW'):
    case FOURCC('SHVR'):
        ParseFloatFunction(rBFRC);
        break;

    default:
        errorf("%s [0x%X]: Unknown BFRC parameter: %s", *rBFRC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseUserEvaluatorParameter(IInputStream& rUSRC)
{
    uint32 ParamOffset = rUSRC.Tell();
    CFourCC Param = rUSRC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('IPM0'):
    case FOURCC('IPM1'):
    case FOURCC('IPM2'):
    case FOURCC('IPM3'):
    case FOURCC('IPM4'):
        ParseIntFunction(rUSRC);
        break;

    case FOURCC('RPM0'):
    case FOURCC('RPM1'):
    case FOURCC('RPM2'):
    case FOURCC('RPM3'):
    case FOURCC('RPM4'):
    case FOURCC('RPM5'):
    case FOURCC('RPM6'):
    case FOURCC('RPM7'):
    case FOURCC('RPM8'):
    case FOURCC('RPM9'):
        ParseFloatFunction(rUSRC);
        break;

    case FOURCC('VEC0'):
    case FOURCC('VEC1'):
    case FOURCC('VEC2'):
        ParseVectorFunction(rUSRC);
        break;

    case FOURCC('MVC0'):
    case FOURCC('MVC1'):
        ParseModVectorFunction(rUSRC);
        break;

    case FOURCC('CLR1'):
        ParseColorFunction(rUSRC);
        break;

    case FOURCC('TEX0'):
    case FOURCC('TEX1'):
        ParseUVFunction(rUSRC);
        break;

    case FOURCC('EMT1'):
        ParseEmitterFunction(rUSRC);
        break;

    default:
        errorf("%s [0x%X]: Unknown USRC parameter: %s", *rUSRC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

bool CUnsupportedParticleLoader::ParseTransformParameter(IInputStream& rXFSC)
{
    uint32 ParamOffset = rXFSC.Tell();
    CFourCC Param = rXFSC.ReadLong();
    if (Param == FOURCC('_END')) return false;

    switch (Param.ToLong())
    {
    case FOURCC('IAV0'):
    case FOURCC('IAV1'):
    case FOURCC('IAV2'):
    case FOURCC('IAV3'):
        ParseIntFunction(rXFSC);
        break;

    case FOURCC('ADV1'):
    case FOURCC('ADV2'):
    case FOURCC('ADV3'):
    case FOURCC('ADV4'):
    case FOURCC('ADV5'):
    case FOURCC('ADV6'):
    case FOURCC('ADV7'):
    case FOURCC('ADV8'):
    case FOURCC('ADV9'):
    case FOURCC('EFSC'):
        ParseFloatFunction(rXFSC);
        break;

    case FOURCC('EOFS'):
    case FOURCC('SOFS'):
    case FOURCC('VAV1'):
    case FOURCC('VAV2'):
    case FOURCC('VAV3'):
    case FOURCC('VAV4'):
        ParseVectorFunction(rXFSC);
        break;

    case FOURCC('EROT'):
    case FOURCC('ROT0'):
    case FOURCC('SROT'):
        ParseRotationFunction(rXFSC);
        break;

    default:
        errorf("%s [0x%X]: Unknown XFSC parameter: %s", *rXFSC.GetSourceString(), ParamOffset, *Param.ToString());
        return false;
    }

    return true;
}

// ************ FUNCTION LOADING ************
void CUnsupportedParticleLoader::ParseBool(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('CNST'):
        rFile.Seek(0x1, SEEK_CUR);
        break;

    case FOURCC('NONE'):
        break;

    default:
        errorf("%s [0x%X]: Unknown bool constant function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseBoolFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('CNST'):
        rFile.Seek(0x1, SEEK_CUR);
        break;

    case FOURCC('MIRR'):
    case FOURCC('P50H'):
        break;

    case FOURCC('EQUL'):
    case FOURCC('LTHN'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('NORM'):
        ParseVectorFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown bool function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseBitfieldFunction(IInputStream& rFile)
{
    // todo: probably not the correct way to do this...
    rFile.Skip(0x10);
}

void CUnsupportedParticleLoader::ParseIntFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
    case FOURCC('GAPC'):
    case FOURCC('GEMT'):
    case FOURCC('GTCP'):
    case FOURCC('PAP0'):
    case FOURCC('PAP1'):
    case FOURCC('PAP2'):
    case FOURCC('PAP3'):
    case FOURCC('PCRT'):
    case FOURCC('PDET'):
        break;

    case FOURCC('ILPT'):
    case FOURCC('IMPL'):
    case FOURCC('KPIN'):
        ParseIntFunction(rFile);
        break;

    case FOURCC('ADD_'):
    case FOURCC('DETH'):
    case FOURCC('DIVD'):
    case FOURCC('IRND'):
    case FOURCC('ISWT'):
    case FOURCC('MODU'):
    case FOURCC('MULT'):
    case FOURCC('RAND'):
    case FOURCC('SUB_'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CHAN'):
    case FOURCC('CLMP'):
    case FOURCC('SPAH'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('PULS'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CNST'):
    {
        [[maybe_unused]] const uint32 Value = rFile.ReadULong();
        ASSERT(gpResourceStore->FindEntry(CAssetID(Value)) == nullptr);
        break;
    }

    case FOURCC('KEYE'):
    case FOURCC('KEYF'):
    case FOURCC('KEYP'):
        ParseKeyframeEmitterData(rFile, Func, 0x4);
        break;

    case FOURCC('TSCL'):
        ParseFloatFunction(rFile);
        break;

    case FOURCC('RTOI'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('BNID'):
        rFile.Seek(0x4, SEEK_CUR);
        rFile.ReadString();
        ParseBool(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown int function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseFloatFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
    case FOURCC('GTCP'):
    case FOURCC('PAP1'):
    case FOURCC('PAP2'):
    case FOURCC('PAP3'):
    case FOURCC('PAP4'):
    case FOURCC('PAP5'):
    case FOURCC('PAP6'):
    case FOURCC('PAP7'):
    case FOURCC('PAP8'):
    case FOURCC('PAP9'):
    case FOURCC('PEA0'):
    case FOURCC('PRLW'):
    case FOURCC('PSA0'):
    case FOURCC('PSA1'):
    case FOURCC('PSA2'):
    case FOURCC('PSC0'):
    case FOURCC('PSC1'):
    case FOURCC('PSI0'):
    case FOURCC('PSI1'):
    case FOURCC('PSV0'):
    case FOURCC('PSV1'):
    case FOURCC('PSLL'):
        break;

    case FOURCC('KPIN'):
    case FOURCC('PRN1'):
    case FOURCC('RLPT'):
    case FOURCC('SCAL'):
        ParseFloatFunction(rFile);
        break;

    case FOURCC('ADD_'):
    case FOURCC('IRND'):
    case FOURCC('ISWT'):
    case FOURCC('LFTW'):
    case FOURCC('MULT'):
    case FOURCC('PRN2'):
    case FOURCC('RAND'):
    case FOURCC('SUB_'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CEQL'):
    case FOURCC('CLTN'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('OCSP'):
        ParseIntFunction(rFile);
        break;

    case FOURCC('MULV'):
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CHAN'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CLMP'):
    case FOURCC('SINE'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CNST'):
        rFile.Seek(0x4, SEEK_CUR);
        break;

    case FOURCC('CRNG'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('DOTP'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('GTCR'):
    case FOURCC('GTCG'):
    case FOURCC('GTCB'):
    case FOURCC('GTCA'):
        ParseColorFunction(rFile);
        break;

    case FOURCC('ITRL'):
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('KEYE'):
    case FOURCC('KEYF'):
    case FOURCC('KEYP'):
        ParseKeyframeEmitterData(rFile, Func, 0x4);
        break;

    case FOURCC('PULS'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('PRN3'):
    case FOURCC('VMAG'):
    case FOURCC('VXTR'):
    case FOURCC('VYTR'):
    case FOURCC('VZTR'):
        ParseVectorFunction(rFile);
        break;

    case FOURCC('PNO1'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('PNO2'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('PNO3'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('PNO4'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('PRN4'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('TOCS'):
        ParseBool(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CREL'):
        ParseBoolFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CEXT'):
        ParseIntFunction(rFile);
        if (mpGroup->Game() >= EGame::DKCReturns) ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown float function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseVectorFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('ADD_'):
    case FOURCC('ISWT'):
    case FOURCC('MULT'):
    case FOURCC('SUB_'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('ANGC'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CCLU'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CHAN'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CIRC'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CNST'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CONE'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CTVC'):
        ParseColorFunction(rFile);
        break;

    case FOURCC('KEYE'):
    case FOURCC('KEYF'):
    case FOURCC('KEYP'):
        ParseKeyframeEmitterData(rFile, Func, 0xC);
        break;

    case FOURCC('KPIN'):
    case FOURCC('NORM'):
        ParseVectorFunction(rFile);
        break;

    case FOURCC('CMPS'):
    case FOURCC('NONE'):
    case FOURCC('PAP1'):
    case FOURCC('PAP2'):
    case FOURCC('PAP3'):
    case FOURCC('PAP4'):
    case FOURCC('PENV'):
    case FOURCC('PETR'):
    case FOURCC('PEVL'):
    case FOURCC('PINV'):
    case FOURCC('PITR'):
    case FOURCC('PIVL'):
    case FOURCC('PLCO'):
    case FOURCC('PLOC'):
    case FOURCC('PNCV'):
    case FOURCC('PSOF'):
    case FOURCC('PSOR'):
    case FOURCC('PSOU'):
    case FOURCC('PSTR'):
    case FOURCC('PVEL'):
    case FOURCC('SLOC'):
        break;

    case FOURCC('PULS'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('RNDV'):
    case FOURCC('RTOV'):
        ParseFloatFunction(rFile);
        break;

    case FOURCC('BNPS'):
    case FOURCC('BNSC'):
        ParseIntFunction(rFile);
        break;

    case FOURCC('CVEC'):
        ParseBoolFunction(rFile);
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('IRTV'):
    case FOURCC('ROTV'):
        ParseRotationFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('VEXT'):
        ParseIntFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown vector function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseModVectorFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
        break;

    case FOURCC('BNCE'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseBool(rFile);
        break;

    case FOURCC('BOXV'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseModVectorFunction(rFile);
        break;

    case FOURCC('CHAN'):
        ParseModVectorFunction(rFile);
        ParseModVectorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CNST'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('EMPL'):
    case FOURCC('IMPL'):
    case FOURCC('LMPL'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseBool(rFile);
        break;

    case FOURCC('EXPL'):
    case FOURCC('SWLC'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('GRAV'):
    case FOURCC('SPOS'):
        ParseVectorFunction(rFile);
        break;

    case FOURCC('PULS'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseModVectorFunction(rFile);
        ParseModVectorFunction(rFile);
        break;

    case FOURCC('SPHV'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseModVectorFunction(rFile);
        break;

    case FOURCC('SWRL'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('WIND'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown mod vector function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseColorFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('CFDE'):
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CHAN'):
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseIntFunction(rFile);
        break;

    case FOURCC('CNST'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('FADE'):
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('CFDL'):
    case FOURCC('ISWT'):
    case FOURCC('MULT'):
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        break;

    case FOURCC('KEYE'):
    case FOURCC('KEYF'):
    case FOURCC('KEYP'):
        ParseKeyframeEmitterData(rFile, Func, 0x10);
        break;

    case FOURCC('KPIN'):
        ParseColorFunction(rFile);
        break;

    case FOURCC('MDAO'):
        ParseColorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('NONE'):
    case FOURCC('PAP0'):
    case FOURCC('PAP1'):
    case FOURCC('PAP2'):
    case FOURCC('PAP3'):
    case FOURCC('PCOL'):
        break;

    case FOURCC('PULS'):
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseColorFunction(rFile);
        ParseColorFunction(rFile);
        break;

    case FOURCC('VRTC'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown color function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseRotationFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
    case FOURCC('RLCL'):
    case FOURCC('RSYS'):
        break;

    case FOURCC('CNST'):
        rFile.Seek(0x10, SEEK_CUR);
        break;

    case FOURCC('BNRT'):
        ParseIntFunction(rFile);
        break;

    case FOURCC('CROT'):
        ParseBoolFunction(rFile);
        ParseRotationFunction(rFile);
        ParseRotationFunction(rFile);
        break;

    case FOURCC('ISWT'):
    case FOURCC('RADD'):
    case FOURCC('RADN'):
    case FOURCC('RSBN'):
    case FOURCC('RSUB'):
        ParseRotationFunction(rFile);
        ParseRotationFunction(rFile);
        break;

    case FOURCC('KPIN'):
        ParseRotationFunction(rFile);
        break;

    case FOURCC('RAXZ'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseRotationFunction(rFile);
        break;

    case FOURCC('REUL'):
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown rotation function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseUVFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
        break;

    case FOURCC('CNST'):
        ParseAssetFunction(rFile);
        break;

    case FOURCC('ATEX'):
        ParseAssetFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseBool(rFile);
        break;

    case FOURCC('TEXP'):
        ParseAssetFunction(rFile);
        ParseIntFunction(rFile);
        ParseIntFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown UV function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseEmitterFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
        break;

    case FOURCC('ASPH'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('ELPS'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseBool(rFile);
        break;

    case FOURCC('PLNE'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    case FOURCC('SEMR'):
        ParseVectorFunction(rFile);
        ParseVectorFunction(rFile);
        break;

    case FOURCC('SETR'):
        ParseParticleParameter(rFile);
        ParseParticleParameter(rFile);
        break;

    case FOURCC('SPHE'):
        ParseVectorFunction(rFile);
        ParseFloatFunction(rFile);
        ParseFloatFunction(rFile);
        break;

    default:
        errorf("%s [0x%X]: Unknown emitter function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseSoundFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();
    
    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
        break;
        
    case FOURCC('CNST'):
    {
        uint32 SoundID = rFile.ReadLong() & 0xFFFF;

        if (SoundID != 0xFFFF)
        {
            SSoundInfo SoundInfo = mpGroup->Entry()->Project()->AudioManager()->GetSoundInfo(SoundID);
            mpGroup->AddDependency(SoundInfo.pAudioGroup);
        }
        
        break;
    }
        
    default:
        errorf("%s [0x%X]: Unknown sound function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseAssetFunction(IInputStream& rFile)
{
    uint32 FuncOffset = rFile.Tell();
    CFourCC Func = rFile.ReadLong();

    switch (Func.ToLong())
    {
    case FOURCC('NONE'):
        break;

    case FOURCC('CNST'):
        mpGroup->AddDependency( CAssetID(rFile, mpGroup->Game()) );
        break;

    default:
        errorf("%s [0x%X]: Unknown asset function: %s", *rFile.GetSourceString(), FuncOffset, *Func.ToString());
        break;
    }
}

void CUnsupportedParticleLoader::ParseSpawnSystemKeyframeData(IInputStream& rFile)
{
    CFourCC Func = rFile.ReadLong();
    if (Func == "NONE") return;
    ASSERT(Func == "CNST");

    rFile.Seek(0x10, SEEK_CUR); // Skip unneeded values
    uint32 Count = rFile.ReadLong();

    for (uint32 iKey = 0; iKey < Count; iKey++)
    {
        rFile.Seek(0x4, SEEK_CUR); // Skip frame number
        uint32 InfoCount = rFile.ReadLong();

        for (uint32 iInfo = 0; iInfo < InfoCount; iInfo++)
        {
            mpGroup->AddDependency( CAssetID(rFile, mpGroup->Game()) );
            rFile.Seek(0xC, SEEK_CUR); // Skip unknown/unneeded values
        }
    }
}

void CUnsupportedParticleLoader::ParseKeyframeEmitterData(IInputStream& rFile, const CFourCC& rkFunc, uint32 ElemSize)
{
    // Skip unneeded values
    if (rkFunc == "KEYE" || rkFunc == "KEYP")
        rFile.Seek(0x12, SEEK_CUR);
    else if (rkFunc == "KEYF")
        rFile.Seek(0x1A, SEEK_CUR);

    uint32 KeyCount = rFile.ReadLong();
    rFile.Seek(KeyCount * ElemSize, SEEK_CUR);

    if (rkFunc == "KEYF")
        ParseFloatFunction(rFile);
}

// ************ STATIC ************
std::unique_ptr<CDependencyGroup> CUnsupportedParticleLoader::LoadParticle(IInputStream& rFile, CResourceEntry *pEntry)
{
    CUnsupportedParticleLoader Loader;
    Loader.mpGroup = std::make_unique<CDependencyGroup>(pEntry);

    // Validate DKCR asset header
    if (pEntry->Game() == EGame::DKCReturns)
    {
        uint32 AssetHeader = rFile.ReadLong();

        if (AssetHeader != 0x6E190001)
        {
            errorf("Invalid DKCR particle header: %08X", AssetHeader);
            return std::move(Loader.mpGroup);
        }
    }

    CFourCC Magic = rFile.ReadLong();

    // Loop through particle functions
    while (true)
    {
        bool ShouldContinue = false;

        switch (Magic.ToLong())
        {
        case FOURCC('GPSM'): ShouldContinue = Loader.ParseParticleParameter(rFile);          break;
        case FOURCC('ELSM'): ShouldContinue = Loader.ParseElectricParameter(rFile);          break;
        case FOURCC('SRSM'): ShouldContinue = Loader.ParseSortedParameter(rFile);            break;
        case FOURCC('SPSM'): ShouldContinue = Loader.ParseSpawnParameter(rFile);             break;
        case FOURCC('SWSH'): ShouldContinue = Loader.ParseSwooshParameter(rFile);            break;
        case FOURCC('DPSM'): ShouldContinue = Loader.ParseDecalParameter(rFile);             break;
        case FOURCC('WPSM'): ShouldContinue = Loader.ParseWeaponParameter(rFile);            break;
        case FOURCC('CRSM'): ShouldContinue = Loader.ParseCollisionResponseParameter(rFile); break;
        case FOURCC('BFRE'): ShouldContinue = Loader.ParseBurstFireParameter(rFile);         break;
        case FOURCC('USER'): ShouldContinue = Loader.ParseUserEvaluatorParameter(rFile);     break;
        case FOURCC('XFSM'): ShouldContinue = Loader.ParseTransformParameter(rFile);         break;

        default:
            errorf("Unrecognized particle system magic: %s", *Magic.ToString());
            ShouldContinue = false;
            break;
        }

        if (!ShouldContinue) break;
    }

    return std::move(Loader.mpGroup);
}
