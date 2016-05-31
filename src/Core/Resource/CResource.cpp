#include "CResource.h"
#include <Common/AssertMacro.h>
#include <map>

std::map<u32, EResType> gExtensionTypeMap;
std::map<u32, TString> gTypeExtensionMap;

u32 GetGameTypeID(EGame Game, EResType ResType)
{
    return ((Game & 0xFFFF) << 16) | (ResType & 0xFFFF);
}

// ************ STATIC ************
EResType CResource::ResTypeForExtension(CFourCC Extension)
{
    auto Find = gExtensionTypeMap.find(Extension.ToLong());

    if (Find == gExtensionTypeMap.end())
    {
        Log::Error("Couldn't find resource type for requested cooked extension: " + Extension.ToString());
        return eInvalidResType;
    }

    return Find->second;
}

// Implementation of functions declared in EResType.h
TString GetRawExtension(EResType /*Type*/, EGame /*Game*/)
{
    return "";
}

TString GetCookedExtension(EResType Type, EGame Game)
{
    u32 GameTypeID = GetGameTypeID(Game, Type);
    auto Find = gTypeExtensionMap.find(GameTypeID);
    if (Find != gTypeExtensionMap.end()) return Find->second;
    else return "";
}

// ************ TYPE REGISTRATIONS ************
/* This macro registers a resource's cooked extension with an EResType enum, which allows for
 * a resource type to be looked up via its extension, and vice versa. Because certain EResType
 * enumerators are reused with different extensions between games, it's possible to set up a
 * registration to be valid for only a specific range of games, and then tie a different
 * extension to the same enumerator for a different game. This allows you to always look up the
 * correct extension for a given resource type with a combination of an EResType and an EGame.
 *
 * You shouldn't need to add any new resource types, as the currently registered ones cover every
 * resource type for every game from the MP1 demo up to DKCR. However, if you do, simply add an
 * extra REGISTER_RESOURCE_TYPE line to the list below.
 */
#define REGISTER_RESOURCE_TYPE(CookedExtension, TypeEnum, FirstGame, LastGame) \
    class CResourceTypeRegistrant__##CookedExtension \
    { \
    public: \
        CResourceTypeRegistrant__##CookedExtension() \
        { \
            ASSERT(FirstGame != eUnknownVersion); \
            \
            /* Register extension with resource type (should be consistent across all games) */ \
            u32 IntExt = CFourCC(#CookedExtension).ToLong(); \
            auto ExtFind = gExtensionTypeMap.find(IntExt); \
            if (ExtFind != gExtensionTypeMap.end()) \
                ASSERT(ExtFind->second == TypeEnum); \
            \
            gExtensionTypeMap[IntExt] = TypeEnum; \
            \
            /* Register resource type with extension for the specified game range */ \
            EGame Game = FirstGame; \
            \
            while (Game <= LastGame) \
            { \
                u32 GameTypeID = GetGameTypeID(Game, TypeEnum); \
                auto Find = gTypeExtensionMap.find(GameTypeID); \
                ASSERT(Find == gTypeExtensionMap.end()); \
                gTypeExtensionMap[GameTypeID] = #CookedExtension; \
                Game = (EGame) ((int) Game + 1); \
            } \
        } \
    }; \
    CResourceTypeRegistrant__##CookedExtension gResourceTypeRegistrant__##CookedExtension;

REGISTER_RESOURCE_TYPE(AFSM, eStateMachine, ePrimeDemo, eEchoes)
REGISTER_RESOURCE_TYPE(AGSC, eAudioGroupSet, ePrimeDemo, eCorruptionProto)
REGISTER_RESOURCE_TYPE(ANCS, eAnimSet, ePrimeDemo, eEchoes)
REGISTER_RESOURCE_TYPE(ANIM, eAnimation, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(ATBL, eAudioLookupTable, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(BFRC, eBurstFireData, eCorruptionProto, eCorruption)
REGISTER_RESOURCE_TYPE(CAAD, eUnknown_CAAD, eCorruption, eCorruption)
REGISTER_RESOURCE_TYPE(CAUD, eAudioMacro, eCorruptionProto, eReturns)
REGISTER_RESOURCE_TYPE(CHAR, eCharacter, eCorruptionProto, eReturns)
REGISTER_RESOURCE_TYPE(CINF, eSkeleton, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(CMDL, eModel, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(CRSC, eParticleCollisionResponse, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(CPRM, eAnimCollisionPrimData, eReturns, eReturns)
REGISTER_RESOURCE_TYPE(CSKR, eSkin, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(CSMP, eAudioSample, eCorruptionProto, eReturns)
REGISTER_RESOURCE_TYPE(CSNG, eMidi, ePrimeDemo, eEchoes)
REGISTER_RESOURCE_TYPE(CSPP, eSpatialPrimitive, eEchoesDemo, eEchoes)
REGISTER_RESOURCE_TYPE(CTWK, eTweak, ePrimeDemo, ePrime)
REGISTER_RESOURCE_TYPE(DCLN, eDynamicCollision, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(DGRP, eDependencyGroup, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(DPSC, eParticleDecal, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(DUMB, eBinaryData, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(EGMC, eStaticGeometryMap, eEchoesDemo, eCorruption)
REGISTER_RESOURCE_TYPE(ELSC, eParticleElectric, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(EVNT, eAnimEventData, ePrimeDemo, ePrime)
REGISTER_RESOURCE_TYPE(FONT, eFont, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(FRME, eGuiFrame, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(FSM2, eStateMachine2, eEchoesDemo, eCorruption)
REGISTER_RESOURCE_TYPE(FSMC, eStateMachine, eReturns, eReturns)
REGISTER_RESOURCE_TYPE(HINT, eHintSystem, ePrime, eCorruption)
REGISTER_RESOURCE_TYPE(KFAM, eGuiKeyFrame, ePrimeDemo, ePrimeDemo)
REGISTER_RESOURCE_TYPE(MAPA, eMapArea, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(MAPU, eMapUniverse, ePrimeDemo, eEchoes)
REGISTER_RESOURCE_TYPE(MAPW, eMapWorld, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(MLVL, eWorld, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(MREA, eArea, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(NTWK, eTweak, eEchoesDemo, eReturns)
REGISTER_RESOURCE_TYPE(PAK , ePackage, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(PART, eParticle, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(PATH, eNavMesh, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(PTLA, ePortalArea, eEchoesDemo, eCorruption)
REGISTER_RESOURCE_TYPE(RULE, eRuleSet, eEchoesDemo, eReturns)
REGISTER_RESOURCE_TYPE(SAND, eSourceAnimData, eCorruptionProto, eCorruption)
REGISTER_RESOURCE_TYPE(SAVA, eSaveArea, eCorruptionProto, eCorruption)
REGISTER_RESOURCE_TYPE(SAVW, eSaveWorld, ePrime, eReturns)
REGISTER_RESOURCE_TYPE(SCAN, eScan, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(SPSC, eParticleSpawn, eEchoesDemo, eReturns)
REGISTER_RESOURCE_TYPE(SRSC, eParticleSorted, eEchoesDemo, eEchoes)
REGISTER_RESOURCE_TYPE(STLC, eStringList, eEchoesDemo, eCorruptionProto)
REGISTER_RESOURCE_TYPE(STRG, eStringTable, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(STRM, eStreamedAudio, eCorruptionProto, eReturns)
REGISTER_RESOURCE_TYPE(SWHC, eParticleSwoosh, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(THP , eVideo, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(TXTR, eTexture, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(USRC, eUserEvaluatorData, eCorruptionProto, eCorruption)
REGISTER_RESOURCE_TYPE(XFSC, eParticleTransform, eReturns, eReturns)
REGISTER_RESOURCE_TYPE(WPSC, eParticleWeapon, ePrimeDemo, eCorruption)
