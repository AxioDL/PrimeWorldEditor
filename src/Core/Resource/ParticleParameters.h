#ifndef PARTICLEPARAMETERS
#define PARTICLEPARAMETERS

#include <Common/CFourCC.h>

// ************ PARTICLE PARAMETERS ************
// This setup kinda sucks but it was the best way I found that wouldn't take a ton of work, works well with the
// temp particle parser, and compiles. This can probably be done better with actual particle support, by just
// defining classes for each parameter/function that can dynamically look up their parameters, and then that could
// be used by the loader instead of needing constant expressions and switch statements.
#define PARAMS_BEGIN enum {
#define PARAMS_END kParticleParamsMax };
#define PARAM(ParticleType, ParamName, CharA, CharB, CharC, CharD) k##ParticleType##ParamName = FOURCC_CONSTEXPR(CharA, CharB, CharC, CharD),

PARAMS_BEGIN
PARAM(Param, END, '_', 'E', 'N', 'D')
// Parameters (PART)
PARAM(Gen, AAPH, 'A','A','P','H') // Bool
PARAM(Gen, ADV1, 'A','D','V','1') // Float
PARAM(Gen, ADV2, 'A','D','V','2') // Float
PARAM(Gen, ADV3, 'A','D','V','3') // Float
PARAM(Gen, ADV4, 'A','D','V','4') // Float
PARAM(Gen, ADV5, 'A','D','V','5') // Float
PARAM(Gen, ADV6, 'A','D','V','6') // Float
PARAM(Gen, ADV7, 'A','D','V','7') // Float
PARAM(Gen, ADV8, 'A','D','V','8') // Float
PARAM(Gen, CIND, 'C', 'I', 'N', 'D') // Bool
PARAM(Gen, COLR, 'C', 'O', 'L', 'R') // Color
PARAM(Gen, CSSD, 'C', 'S', 'S', 'D') // Int
PARAM(Gen, EMTR, 'E', 'M', 'T', 'R') // Emitter
PARAM(Gen, FXLL, 'F', 'X', 'L', 'L') // Bool
PARAM(Gen, GRTE, 'G', 'R', 'T', 'E') // Float
PARAM(Gen, ICTS, 'I', 'C', 'T', 'S') // Asset (PART)
PARAM(Gen, IDTS, 'I', 'D', 'T', 'S') // Asset (PART)
PARAM(Gen, IITS, 'I', 'I', 'T', 'S') // Asset (PART)
PARAM(Gen, ILOC, 'I', 'L', 'O', 'C') // Vector
PARAM(Gen, IVEC, 'I', 'V', 'E', 'C') // Vector
PARAM(Gen, KSSM, 'K', 'S', 'S', 'M') // SpawnSystemKeyframeData
PARAM(Gen, LCLR, 'L', 'C', 'L', 'R') // Color
PARAM(Gen, LDIR, 'L', 'D', 'I', 'R') // Vector
PARAM(Gen, LENG, 'L', 'E', 'N', 'G') // Float
PARAM(Gen, LFOT, 'L', 'F', 'O', 'T') // Int
PARAM(Gen, LFOR, 'L', 'F', 'O', 'R') // Float
PARAM(Gen, LINE, 'L', 'I', 'N', 'E') // Bool
PARAM(Gen, LINT, 'L', 'I', 'N', 'T') // Float
PARAM(Gen, LIT_, 'L', 'I', 'T', '_') // Bool
PARAM(Gen, LOFF, 'L', 'O', 'F', 'F') // Vector
PARAM(Gen, LSLA, 'L', 'S', 'L', 'A') // Float
PARAM(Gen, LTME, 'L', 'T', 'M', 'E') // Int
PARAM(Gen, LTYP, 'L', 'T', 'Y', 'P') // Int
PARAM(Gen, MAXP, 'M', 'A', 'X', 'P') // Int
PARAM(Gen, MBLR, 'M', 'B', 'L', 'R') // Bool
PARAM(Gen, MBSP, 'M', 'B', 'S', 'P') // Int
PARAM(Gen, NCSY, 'N', 'C', 'S', 'Y') // Int
PARAM(Gen, NDSY, 'N', 'D', 'S', 'Y') // Int
PARAM(Gen, OPTS, 'O', 'P', 'T', 'S') // Bool
PARAM(Gen, ORNT, 'O', 'R', 'N', 'T') // Bool
PARAM(Gen, PISY, 'P', 'I', 'S', 'Y') // Int
PARAM(Gen, PMAB, 'P', 'M', 'A', 'B') // Bool
PARAM(Gen, PMDL, 'P', 'M', 'D', 'L') // Asset (CMDL)
PARAM(Gen, PMOO, 'P', 'M', 'O', 'O') // Bool
PARAM(Gen, PMCL, 'P', 'M', 'C', 'L') // Color
PARAM(Gen, PMOP, 'P', 'M', 'O', 'P') // Vector
PARAM(Gen, PMRT, 'P', 'M', 'R', 'T') // Vector
PARAM(Gen, PMSC, 'P', 'M', 'S', 'C') // Vector
PARAM(Gen, PMUS, 'P', 'M', 'U', 'S') // Bool
PARAM(Gen, POFS, 'P', 'O', 'F', 'S') // Vector
PARAM(Gen, PSIV, 'P', 'S', 'I', 'V') // Int
PARAM(Gen, PSLT, 'P', 'S', 'L', 'T') // Int
PARAM(Gen, PSOV, 'P', 'S', 'O', 'V') // Vector
PARAM(Gen, PSTS, 'P', 'S', 'T', 'S') // Float
PARAM(Gen, PSWT, 'P', 'S', 'W', 'T') // Int
PARAM(Gen, PSVM, 'P', 'S', 'V', 'M') // Mod Vector
PARAM(Gen, ROTA, 'R', 'O', 'T', 'A') // Float
PARAM(Gen, RSOP, 'R', 'S', 'O', 'P') // Bool
PARAM(Gen, SEED, 'S', 'E', 'E', 'D') // Int
PARAM(Gen, SELC, 'S', 'E', 'L', 'C') // Asset (ELSC)
PARAM(Gen, SEPO, 'S', 'E', 'P', 'O') // Vector
PARAM(Gen, SESD, 'S', 'E', 'S', 'D') // Int
PARAM(Gen, SISY, 'S', 'I', 'S', 'Y') // Int
PARAM(Gen, SIZE, 'S', 'I', 'Z', 'E') // Float
PARAM(Gen, SORT, 'S', 'O', 'R', 'T') // Bool
PARAM(Gen, SSPO, 'S', 'S', 'P', 'O') // Vector
PARAM(Gen, SSSD, 'S', 'S', 'S', 'D') // Int
PARAM(Gen, SSWH, 'S', 'S', 'W', 'H') // Asset (SWHC)
PARAM(Gen, TEXR, 'T', 'E', 'X', 'R') // UV
PARAM(Gen, TIND, 'T', 'I', 'N', 'D') // UV
PARAM(Gen, VEL1, 'V', 'E', 'L', '1') // Mod Vector
PARAM(Gen, VEL2, 'V', 'E', 'L', '2') // Mod Vector
PARAM(Gen, VEL3, 'V', 'E', 'L', '3') // Mod Vector
PARAM(Gen, VEL4, 'V', 'E', 'L', '4') // Mod Vector
PARAM(Gen, VMD1, 'V', 'M', 'D', '1') // Bool
PARAM(Gen, VMD2, 'V', 'M', 'D', '2') // Bool
PARAM(Gen, VMD3, 'V', 'M', 'D', '3') // Bool
PARAM(Gen, VMD4, 'V', 'M', 'D', '4') // Bool
PARAM(Gen, WIDT, 'W', 'I', 'D', 'T') // Float
PARAM(Gen, ZBUF, 'Z', 'B', 'U', 'F') // Bool
// Parameters (ELSC)
PARAM(Elec, AMPD, 'A', 'M', 'P', 'D') // Float
PARAM(Elec, AMPL, 'A', 'M', 'P', 'L') // Bool
PARAM(Elec, COLR, 'C', 'O', 'L', 'R') // Color
PARAM(Elec, EPSM, 'E', 'P', 'S', 'M') // Asset (PART)
PARAM(Elec, FEMT, 'F', 'E', 'M', 'T') // Emitter
PARAM(Elec, GPSM, 'G', 'P', 'S', 'M') // Asset (PART)
PARAM(Elec, GRAT, 'G', 'R', 'A', 'T') // Float
PARAM(Elec, IEMT, 'I', 'E', 'M', 'T') // Emitter
PARAM(Elec, LCL1, 'L', 'C', 'L', '1') // Color
PARAM(Elec, LCL2, 'L', 'C', 'L', '2') // Color
PARAM(Elec, LCL3, 'L', 'C', 'L', '3') // Color
PARAM(Elec, LIFE, 'L', 'I', 'F', 'E') // Int
PARAM(Elec, LWD1, 'L', 'W', 'D', '1') // Float
PARAM(Elec, LWD2, 'L', 'W', 'D', '2') // Float
PARAM(Elec, LWD3, 'L', 'W', 'D', '3') // Float
PARAM(Elec, SCNT, 'S', 'C', 'N', 'T') // Int
PARAM(Elec, SLIF, 'S', 'L', 'I', 'F') // Int
PARAM(Elec, SSEG, 'S', 'S', 'E', 'G') // Int
PARAM(Elec, SSWH, 'S', 'S', 'W', 'H') // Asset (SWHC)
PARAM(Elec, ZERY, 'Z', 'E', 'R', 'Y') // Bool
// Parameters (SWHC)
PARAM(Swoosh, AALP, 'A', 'A', 'L', 'P') // Bool
PARAM(Swoosh, COLR, 'C', 'O', 'L', 'R') // Color
PARAM(Swoosh, CRND, 'C', 'R', 'N', 'D') // Bool
PARAM(Swoosh, CROS, 'C', 'R', 'O', 'S') // Bool
PARAM(Swoosh, IROT, 'I', 'R', 'O', 'T') // Float
PARAM(Swoosh, IVEL, 'I', 'V', 'E', 'L') // Vector
PARAM(Swoosh, LENG, 'L', 'E', 'N', 'G') // Int
PARAM(Swoosh, LLRD, 'L', 'L', 'R', 'D') // Bool
PARAM(Swoosh, LRAD, 'L', 'R', 'A', 'D') // Float
PARAM(Swoosh, NPOS, 'N', 'P', 'O', 'S') // Vector
PARAM(Swoosh, ORNT, 'O', 'R', 'N', 'T') // Bool
PARAM(Swoosh, POFS, 'P', 'O', 'F', 'S') // Vector
PARAM(Swoosh, PSLT, 'P', 'S', 'L', 'T') // Int
PARAM(Swoosh, ROTM, 'R', 'O', 'T', 'M') // Float
PARAM(Swoosh, RRAD, 'R', 'R', 'A', 'D') // Float
PARAM(Swoosh, SIDE, 'S', 'I', 'D', 'E') // Int
PARAM(Swoosh, SPLN, 'S', 'P', 'L', 'N') // Float
PARAM(Swoosh, SROT, 'S', 'R', 'O', 'T') // Bool
PARAM(Swoosh, TEXR, 'T', 'E', 'X', 'R') // UV
PARAM(Swoosh, TEXW, 'T', 'E', 'X', 'W') // Bool
PARAM(Swoosh, TIME, 'T', 'I', 'M', 'E') // Float
PARAM(Swoosh, TSPN, 'T', 'S', 'P', 'N') // Float
PARAM(Swoosh, VELM, 'V', 'E', 'L', 'M') // Vector
PARAM(Swoosh, VLM2, 'V', 'L', 'M', '2') // Vector
PARAM(Swoosh, VLS1, 'V', 'L', 'S', '1') // Bool
PARAM(Swoosh, VLS2, 'V', 'L', 'S', '2') // Bool
PARAM(Swoosh, WIRE, 'W', 'I', 'R', 'E') // Bool
PARAM(Swoosh, ZBUF, 'Z', 'B', 'U', 'F') // Bool
// Parameters (DPSC)
PARAM(Decal, 1ADD, '1', 'A', 'D', 'D') // Bool
PARAM(Decal, 2ADD, '2', 'A', 'D', 'D') // Bool
PARAM(Decal, 1TEX, '1', 'T', 'E', 'X') // Asset (TXTR)
PARAM(Decal, 2TEX, '2', 'T', 'E', 'X') // Asset (TXTR)
PARAM(Decal, 1CLR, '1', 'C', 'L', 'R') // Color
PARAM(Decal, 2CLR, '2', 'C', 'L', 'R') // Color
PARAM(Decal, 1OFF, '1', 'O', 'F', 'F') // Bool
PARAM(Decal, 2OFF, '2', 'O', 'F', 'F') // Bool
PARAM(Decal, 1ROT, '1', 'R', 'O', 'T') // Float
PARAM(Decal, 2ROT, '2', 'R', 'O', 'T') // Float
PARAM(Decal, 1SZE, '1', 'S', 'Z', 'E') // Float
PARAM(Decal, 2SZE, '2', 'S', 'Z', 'E') // Float
PARAM(Decal, 1LFT, '1', 'L', 'F', 'T') // Int
PARAM(Decal, 2LFT, '2', 'L', 'F', 'T') // Int
PARAM(Decal, DLFT, 'D', 'L', 'F', 'T') // Int
PARAM(Decal, DMAB, 'D', 'M', 'A', 'B') // Bool
PARAM(Decal, DMCL, 'D', 'M', 'C', 'L') // Bool
PARAM(Decal, DMDL, 'D', 'M', 'D', 'L') // Bool
PARAM(Decal, DMOO, 'D', 'M', 'O', 'O') // Bool
PARAM(Decal, DMOP, 'D', 'M', 'O', 'P') // Bool
PARAM(Decal, DMRT, 'D', 'M', 'R', 'T') // Bool
PARAM(Decal, DMSC, 'D', 'M', 'S', 'C') // Bool
// Parameters (WPSC)
PARAM(Weapon, APSM, 'A', 'P', 'S', 'M') // Asset (PART)
PARAM(Weapon, APSO, 'A', 'P', 'S', 'O') // Bool
PARAM(Weapon, APS1, 'A', 'P', 'S', '1') // Asset (PART)
PARAM(Weapon, AP11, 'A', 'P', '1', '1') // Bool
PARAM(Weapon, APS2, 'A', 'P', 'S', '2') // Asset (PART)
PARAM(Weapon, AP21, 'A', 'P', '2', '1') // Bool
PARAM(Weapon, ASW1, 'A', 'S', 'W', '1') // Asset (SWHC)
PARAM(Weapon, AS11, 'A', 'S', '1', '1') // Bool
PARAM(Weapon, ASW2, 'A', 'S', 'W', '2') // Asset (SWHC)
PARAM(Weapon, AS12, 'A', 'S', '1', '2') // Bool
PARAM(Weapon, ASW3, 'A', 'S', 'W', '3') // Asset (SWHC)
PARAM(Weapon, AS13, 'A', 'S', '1', '3') // Bool
PARAM(Weapon, COLR, 'C', 'O', 'L', 'R') // Asset (CRSC)
PARAM(Weapon, EWTR, 'E', 'W', 'T', 'R') // Bool
PARAM(Weapon, FC60, 'F', 'C', '6', '0') // Bool
PARAM(Weapon, HOMG, 'H', 'O', 'M', 'G') // Bool
PARAM(Weapon, IORN, 'I', 'O', 'R', 'N') // Vector
PARAM(Weapon, IVEC, 'I', 'V', 'E', 'C') // Vector
PARAM(Weapon, LWTR, 'L', 'W', 'T', 'R') // Bool
PARAM(Weapon, OFST, 'O', 'F', 'S', 'T') // Vector
PARAM(Weapon, OHEF, 'O', 'H', 'E', 'F') // Asset (CMDL)
PARAM(Weapon, PCOL, 'P', 'C', 'O', 'L') // Color
PARAM(Weapon, PJFX, 'P', 'J', 'F', 'X') // Int
PARAM(Weapon, POFS, 'P', 'O', 'F', 'S') // Vector
PARAM(Weapon, PSCL, 'P', 'S', 'C', 'L') // Vector
PARAM(Weapon, PSLT, 'P', 'S', 'L', 'T') // Int
PARAM(Weapon, PSOV, 'P', 'S', 'O', 'V') // Float
PARAM(Weapon, PSVM, 'P', 'S', 'V', 'M') // Vector
PARAM(Weapon, RNGE, 'R', 'N', 'G', 'E') // Float
PARAM(Weapon, SPS1, 'S', 'P', 'S', '1') // Bool
PARAM(Weapon, SPS2, 'S', 'P', 'S', '2') // Bool
PARAM(Weapon, SWTR, 'S', 'W', 'T', 'R') // Bool
PARAM(Weapon, TRAT, 'T', 'R', 'A', 'T') // Float
PARAM(Weapon, VMD2, 'V', 'M', 'D', '2') // Bool
// Parameters (CRSC)
PARAM(Coli, 1ATA, '1', 'A', 'T', 'A') // Int
PARAM(Coli, 2ATA, '2', 'A', 'T', 'A') // Int
PARAM(Coli, 3ATA, '3', 'A', 'T', 'A') // Int
PARAM(Coli, 4ATA, '4', 'A', 'T', 'A') // Int
PARAM(Coli, 5ATA, '5', 'A', 'T', 'A') // Int
PARAM(Coli, 6ATA, '6', 'A', 'T', 'A') // Int
PARAM(Coli, 1ATB, '1', 'A', 'T', 'B') // Int
PARAM(Coli, 2ATB, '2', 'A', 'T', 'B') // Int
PARAM(Coli, 3ATB, '3', 'A', 'T', 'B') // Int
PARAM(Coli, 4ATB, '4', 'A', 'T', 'B') // Int
PARAM(Coli, 5ATB, '5', 'A', 'T', 'B') // Int
PARAM(Coli, 6ATB, '6', 'A', 'T', 'B') // Int
PARAM(Coli, 1BSE, '1', 'B', 'S', 'E') // Int
PARAM(Coli, 2BSE, '2', 'B', 'S', 'E') // Int
PARAM(Coli, 1DRN, '1', 'D', 'R', 'N') // Int
PARAM(Coli, 2DRN, '2', 'D', 'R', 'N') // Int
PARAM(Coli, 3DRN, '3', 'D', 'R', 'N') // Int
PARAM(Coli, 4DRN, '4', 'D', 'R', 'N') // Int
PARAM(Coli, 5DRN, '5', 'D', 'R', 'N') // Int
PARAM(Coli, 6DRN, '6', 'D', 'R', 'N') // Int
PARAM(Coli, 2MUD, '2', 'M', 'U', 'D') // Int
PARAM(Coli, 2SAN, '2', 'S', 'A', 'N') // Int
PARAM(Coli, BHFX, 'B', 'H', 'F', 'X') // Int
PARAM(Coli, CHDL, 'C', 'H', 'D', 'L') // Asset (DPSC)
PARAM(Coli, CHFX, 'C', 'H', 'F', 'X') // Int
PARAM(Coli, CODL, 'C', 'O', 'D', 'L') // Asset (DPSC)
PARAM(Coli, CSFX, 'C', 'S', 'F', 'X') // Int
PARAM(Coli, CZFX, 'C', 'Z', 'F', 'X') // Int
PARAM(Coli, DCHR, 'D', 'C', 'H', 'R') // Asset (PART)
PARAM(Coli, DCSH, 'D', 'C', 'S', 'H') // Int
PARAM(Coli, DDCL, 'D', 'D', 'C', 'L') // Asset (DPSC)
PARAM(Coli, DEFS, 'D', 'E', 'F', 'S') // Asset (PART)
PARAM(Coli, DENM, 'D', 'E', 'N', 'M') // Asset (PART)
PARAM(Coli, DESH, 'D', 'E', 'S', 'H') // Asset (PART)
PARAM(Coli, DSFX, 'D', 'S', 'F', 'X') // Int
PARAM(Coli, DSHX, 'D', 'S', 'H', 'X') // Int
PARAM(Coli, ENDL, 'E', 'N', 'D', 'L') // Asset (DPSC)
PARAM(Coli, FOFF, 'F', 'O', 'F', 'F') // Float
PARAM(Coli, GODL, 'G', 'O', 'D', 'L') // Asset (DPSC)
PARAM(Coli, GOFX, 'G', 'O', 'F', 'X') // Int
PARAM(Coli, GOOO, 'G', 'O', 'O', 'O') // Int
PARAM(Coli, GRAS, 'G', 'R', 'A', 'S') // Int
PARAM(Coli, GRDL, 'G', 'R', 'D', 'L') // Asset (DPSC)
PARAM(Coli, GRFX, 'G', 'R', 'F', 'X') // Int
PARAM(Coli, HBFX, 'H', 'B', 'F', 'X') // Int
PARAM(Coli, ICDL, 'I', 'C', 'D', 'L') // Asset (DPSC)
PARAM(Coli, ICEE, 'I', 'C', 'E', 'E') // Int
PARAM(Coli, ICFX, 'I', 'C', 'F', 'X') // Int
PARAM(Coli, MEDL, 'M', 'E', 'D', 'L') // Asset (DPSC)
PARAM(Coli, MSFX, 'M', 'S', 'F', 'X') // Int
PARAM(Coli, PBOS, 'P', 'B', 'O', 'S') // Int
PARAM(Coli, PBHX, 'P', 'B', 'H', 'X') // Int
PARAM(Coli, PBSX, 'P', 'B', 'S', 'X') // Int
PARAM(Coli, RNGE, 'R', 'N', 'G', 'E') // Float
PARAM(Coli, SHFX, 'S', 'H', 'F', 'X') // Int
PARAM(Coli, TAFX, 'T', 'A', 'F', 'X') // Int
PARAM(Coli, TALP, 'T', 'A', 'L', 'P') // Asset (PART)
PARAM(Coli, TASP, 'T', 'A', 'S', 'P') // Int
PARAM(Coli, WODL, 'W', 'O', 'D', 'L') // Asset (DPSC)
PARAM(Coli, WSFX, 'W', 'S', 'F', 'X') // Int
PARAM(Coli, WTDL, 'W', 'T', 'D', 'L') // Asset (DPSC)
PARAM(Coli, WTFX, 'W', 'T', 'F', 'X') // Int
PARAMS_END

#undef PARAMS_BEGIN
#undef PARAMS_END
#undef PARAM

// ************ PARTICLE FUNCTIONS ************
#define FUNCS_BEGIN enum {
#define FUNCS_END kParticleFunctionsMax };
#define FUNC(Type, ParamName, CharA, CharB, CharC, CharD) k##Type##ParamName = FOURCC_CONSTEXPR(##CharA, ##CharB, ##CharC, ##CharD),

FUNCS_BEGIN
FUNC(Func, NONE, 'N', 'O', 'N', 'E')
// Bool Functions
FUNC(Bool, CNST, 'C','N','S','T') // Bool
// Int Functions
FUNC(Int, ADD_, 'A', 'D', 'D', '_') // Int, Int
FUNC(Int, CHAN, 'C', 'H', 'A', 'N') // Int, Int, Int
FUNC(Int, CLMP, 'C', 'L', 'M', 'P') // Int, Int, Int
FUNC(Int, CNST, 'C', 'N', 'S', 'T') // Int
FUNC(Int, DETH, 'D', 'E', 'T', 'H') // Int, Int
FUNC(Int, GAPC, 'G', 'A', 'P', 'C') // N/A
FUNC(Int, GEMT, 'G', 'E', 'M', 'T') // N/A
FUNC(Int, GTCP, 'G', 'T', 'C', 'P') // N/A
FUNC(Int, ILPT, 'I', 'L', 'P', 'T') // Int
FUNC(Int, IMPL, 'I', 'M', 'P', 'L') // Int
FUNC(Int, IRND, 'I', 'R', 'N', 'D') // Int, Int
FUNC(Int, KEYE, 'K', 'E', 'Y', 'E') // KeyframeEmitterData
FUNC(Int, KEYP, 'K', 'E', 'Y', 'P') // KeyframeEmitterData
FUNC(Int, MODU, 'M', 'O', 'D', 'U') // Int, Int
FUNC(Int, MULT, 'M', 'U', 'L', 'T') // Int, Int
FUNC(Int, PULS, 'P', 'U', 'L', 'S') // Int, Int, Int, Int
FUNC(Int, RAND, 'R', 'A', 'N', 'D') // Int, Int
FUNC(Int, SPAH, 'S', 'P', 'A', 'H') // Int, Int, Int
FUNC(Int, SUB_, 'S', 'U', 'B', '_') // Int, Int
FUNC(Int, TSCL, 'T', 'S', 'C', 'L') // Float
// Float Functions
FUNC(Float, ADD_, 'A', 'D', 'D', '_') // Float, Float
FUNC(Float, CEQL, 'C', 'E', 'Q', 'L') // Float, Float, Float, Float
FUNC(Float, CEXT, 'C', 'E', 'X', 'T') // Int
FUNC(Float, CHAN, 'C', 'H', 'A', 'N') // Float, Float, Int
FUNC(Float, CLMP, 'C', 'L', 'M', 'P') // Float, Float, Float
FUNC(Float, CLTN, 'C', 'L', 'T', 'N') // Float, Float, Float, Float
FUNC(Float, CNST, 'C', 'N', 'S', 'T') // Float
FUNC(Float, CRNG, 'C', 'R', 'N', 'G') // Float, Float, Float, Float, Float
FUNC(Float, DOTP, 'D', 'O', 'T', 'P') // Vector, Vector
FUNC(Float, GTCR, 'G', 'T', 'C', 'R') // Color
FUNC(Float, GTCG, 'G', 'T', 'C', 'G') // Color
FUNC(Float, GTCB, 'G', 'T', 'C', 'B') // Color
FUNC(Float, GTCA, 'G', 'T', 'C', 'A') // Color
FUNC(Float, IRND, 'I', 'R', 'N', 'D') // Float, Float
FUNC(Float, ISWT, 'I', 'S', 'W', 'T') // Float, Float
FUNC(Float, ITRL, 'I', 'T', 'R', 'L') // Int, Float
FUNC(Float, KEYE, 'K', 'E', 'Y', 'E') // KeyframeEmitterData
FUNC(Float, KEYP, 'K', 'E', 'Y', 'P') // KeyframeEmitterData
FUNC(Float, LFTW, 'L', 'F', 'T', 'W') // Float, Float
FUNC(Float, MULT, 'M', 'U', 'L', 'T') // Float, Float
FUNC(Float, PAP1, 'P', 'A', 'P', '1') // N/A
FUNC(Float, PAP2, 'P', 'A', 'P', '2') // N/A
FUNC(Float, PAP3, 'P', 'A', 'P', '3') // N/A
FUNC(Float, PAP4, 'P', 'A', 'P', '4') // N/A
FUNC(Float, PAP5, 'P', 'A', 'P', '5') // N/A
FUNC(Float, PAP6, 'P', 'A', 'P', '6') // N/A
FUNC(Float, PAP7, 'P', 'A', 'P', '7') // N/A
FUNC(Float, PAP8, 'P', 'A', 'P', '8') // N/A
FUNC(Float, PRLW, 'P', 'R', 'L', 'W') // N/A
FUNC(Float, PSLL, 'P', 'S', 'L', 'L') // N/A
FUNC(Float, PULS, 'P', 'U', 'L', 'S') // Int, Int, Float, Float
FUNC(Float, RAND, 'R', 'A', 'N', 'D') // Float, Float
FUNC(Float, RLPT, 'R', 'L', 'P', 'T') // Float
FUNC(Float, SCAL, 'S', 'C', 'A', 'L') // Float
FUNC(Float, SINE, 'S', 'I', 'N', 'E') // Float, Float, Float
FUNC(Float, SUB_, 'S', 'U', 'B', '_') // Float, Float
FUNC(Float, VMAG, 'V', 'M', 'A', 'G') // Vector
FUNC(Float, VXTR, 'V', 'X', 'T', 'R') // Vector
FUNC(Float, VYTR, 'V', 'Y', 'T', 'R') // Vector
FUNC(Float, VZTR, 'V', 'Z', 'T', 'R') // Vector
// Vector Functions
FUNC(Vector, ADD_, 'A', 'D', 'D', '_') // Vector, Vector
FUNC(Vector, ANGC, 'A', 'N', 'G', 'C') // Float, Float, Float, Float, Float
FUNC(Vector, CCLU, 'C', 'C', 'L', 'U') // Vector, Vector, Int, Float
FUNC(Vector, CHAN, 'C', 'H', 'A', 'N') // Vector, Vector, Int
FUNC(Vector, CIRC, 'C', 'I', 'R', 'C') // Vector, Vector, Float, Float, Float
FUNC(Vector, CNST, 'C', 'N', 'S', 'T') // Float, Float, Float
FUNC(Vector, CONE, 'C', 'O', 'N', 'E') // Vector, Float
FUNC(Vector, CTVC, 'C', 'T', 'V', 'C') // Color
FUNC(Vector, KEYE, 'K', 'E', 'Y', 'E') // KeyframeEmitterData
FUNC(Vector, KEYP, 'K', 'E', 'Y', 'P') // KeyframeEmitterData
FUNC(Vector, MULT, 'M', 'U', 'L', 'T') // Vector, Vector
FUNC(Vector, PLCO, 'P', 'L', 'C', 'O') // N/A
FUNC(Vector, PLOC, 'P', 'L', 'O', 'C') // N/A
FUNC(Vector, PSOF, 'P', 'S', 'O', 'F') // N/A
FUNC(Vector, PSOU, 'P', 'S', 'O', 'U') // N/A
FUNC(Vector, PSOR, 'P', 'S', 'O', 'R') // N/A
FUNC(Vector, PSTR, 'P', 'S', 'T', 'R') // N/A
FUNC(Vector, PULS, 'P', 'U', 'L', 'S') // Int, Int, Vector, Vector
FUNC(Vector, PVEL, 'P', 'V', 'E', 'L') // N/A
FUNC(Vector, RTOV, 'R', 'T', 'O', 'V') // Float
FUNC(Vector, SUB_, 'S', 'U', 'B', '_') // Vector, Vector
// Mod Vector Functions
FUNC(ModVector, BNCE, 'B', 'N', 'C', 'E') // Vector, Vector, Float, Float, Bool
FUNC(ModVector, CHAN, 'C', 'H', 'A', 'N') // ModVector, ModVector, Int
FUNC(ModVector, CNST, 'C', 'N', 'S', 'T') // Float, Float, Float
FUNC(ModVector, EMPL, 'E', 'M', 'P', 'L') // Vector, Float, Float, Float, Bool
FUNC(ModVector, EXPL, 'E', 'X', 'P', 'L') // Float, Float
FUNC(ModVector, GRAV, 'G', 'R', 'A', 'V') // Vector
FUNC(ModVector, IMPL, 'I', 'M', 'P', 'L') // Vector, Float, Float, Float, Bool
FUNC(ModVector, LMPL, 'L', 'M', 'P', 'L') // Vector, Float, Float, Float, Bool
FUNC(ModVector, PULS, 'P', 'U', 'L', 'S') // Int, Int, ModVector, ModVector
FUNC(ModVector, SPOS, 'S', 'P', 'O', 'S') // Vector
FUNC(ModVector, SWRL, 'S', 'W', 'R', 'L') // Vector, Vector, Float, Float
FUNC(ModVector, WIND, 'W', 'I', 'N', 'D') // Vector, Float
// Color Functions
FUNC(Color, CFDE, 'C', 'F', 'D', 'E') // Color, Color, Float, Float
FUNC(Color, CHAN, 'C', 'H', 'A', 'N') // Color, Color, Int
FUNC(Color, CNST, 'C', 'N', 'S', 'T') // Float, Float, Float, Float
FUNC(Color, FADE, 'F', 'A', 'D', 'E') // Color, Color, Float
FUNC(Color, KEYE, 'K', 'E', 'Y', 'E') // KeyframeEmitterData
FUNC(Color, KEYP, 'K', 'E', 'Y', 'P') // KeyframeEmitterData
FUNC(Color, PCOL, 'P', 'C', 'O', 'L') // N/A
FUNC(Color, PULS, 'P', 'U', 'L', 'S') // Int, Int, Color, Color
// UV Functions
FUNC(UV, CNST, 'C', 'N', 'S', 'T') // Asset (TXTR)
FUNC(UV, ATEX, 'A', 'T', 'E', 'X') // Asset (TXTR), Int, Int, Int, Int, Int, Bool
// Asset Functions
FUNC(Asset, CNST, 'C', 'N', 'S', 'T') // Asset ID
// Emitter Functions
FUNC(Emitter, SETR, 'S', 'E', 'T', 'R') // Vector, Vector
FUNC(Emitter, SEMR, 'S', 'E', 'M', 'R') // Vector, Vector
FUNC(Emitter, SPHE, 'S', 'P', 'H', 'E') // Vector, Float, Float
FUNC(Emitter, ASPH, 'A', 'S', 'P', 'H') // Vector, Float, Float, Float, Float, Float, Float
FUNCS_END

#undef FUNCS_BEGIN
#undef FUNCS_END
#undef FUNC

#endif // PARTICLEPARAMETERS

