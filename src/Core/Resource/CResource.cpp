#include "CResource.h"
#include "Core/GameProject/CResourceStore.h"
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
    auto Find = gExtensionTypeMap.find(Extension.ToUpper().ToLong());

    if (Find == gExtensionTypeMap.end())
    {
        Log::Error("Couldn't find resource type for requested cooked extension: " + Extension.ToString());
        return eInvalidResType;
    }

    return Find->second;
}

// ************ GLOBAL ************
bool ResourceSupportsSerialization(EResType Type)
{
    switch (Type)
    {
    case eWorld:
        return true;
    default:
        return false;
    }
}

TString GetResourceTypeName(EResType Type)
{
    switch (Type)
    {
    case eAnimation:                    return "Animation";
    case eAnimCollisionPrimData:        return "Animation Collision Primitive Data";
    case eAnimEventData:                return "Animation Event Data";
    case eAnimSet:                      return "Animation Character Set";
    case eArea:                         return "Area";
    case eAreaCollision:                return "Area Collision";
    case eAreaGeometry:                 return "Area Geometry";
    case eAreaLights:                   return "Area Lights";
    case eAreaMaterials:                return "Area Materials";
    case eAreaSurfaceBounds:            return "Area Surface Bounds";
    case eAreaOctree:                   return "Area Octree";
    case eAreaVisibilityTree:           return "Area Visibility Tree";
    case eAudioGroup:                   return "Audio Group Set";
    case eAudioMacro:                   return "Audio Macro";
    case eAudioSample:                  return "Audio Sample";
    case eAudioLookupTable:             return "Audio Lookup Table";
    case eBinaryData:                   return "Binary Data";
    case eBurstFireData:                return "Burst Fire Data";
    case eCharacter:                    return "Character";
    case eDependencyGroup:              return "Dependency Group";
    case eDynamicCollision:             return "Dynamic Collision";
    case eFont:                         return "Font";
    case eGuiFrame:                     return "Gui Frame";
    case eGuiKeyFrame:                  return "Gui Keyframe";
    case eHintSystem:                   return "Hint System";
    case eMapArea:                      return "Area Map";
    case eMapWorld:                     return "World Map";
    case eMapUniverse:                  return "Universe Map";
    case eMidi:                         return "MIDI";
    case eModel:                        return "Model";
    case eMusicTrack:                   return "Music";
    case ePackage:                      return "Package";
    case eParticle:                     return "Particle System";
    case eParticleCollisionResponse:    return "Collision Response Particle System";
    case eParticleDecal:                return "Decal Particle System";
    case eParticleElectric:             return "Electric Particle System";
    case eParticleSorted:               return "Sorted Particle System";
    case eParticleSpawn:                return "Spawn Particle System";
    case eParticleSwoosh:               return "Swoosh Particle System";
    case eParticleTransform:            return "Transform Particle System";
    case eParticleWeapon:               return "Weapon Particle System";
    case ePathfinding:                  return "Pathfinding Mesh";
    case ePortalArea:                   return "Portal Area";
    case eResource:                     return "Resource";
    case eRuleSet:                      return "Rule Set";
    case eSaveArea:                     return "Area Save Info";
    case eSaveWorld:                    return "World Save Info";
    case eScan:                         return "Scan";
    case eSkeleton:                     return "Skeleton";
    case eSkin:                         return "Skin";
    case eSourceAnimData:               return "Source Animation Data";
    case eSpatialPrimitive:             return "Spatial Primitive";
    case eStateMachine:                 return "State Machine";
    case eStateMachine2:                return "State Machine";
    case eStaticGeometryMap:            return "Static Geometry Map";
    case eStreamedAudio:                return "Streamed Audio";
    case eStringList:                   return "String List";
    case eStringTable:                  return "String Table";
    case eTexture:                      return "Texture";
    case eTweak:                        return "Tweak Data";
    case eUnknown_CAAD:                 return "CAAD";
    case eUserEvaluatorData:            return "User Evaluator Data";
    case eVideo:                        return "Video";
    case eWorld:                        return "World";
    default:                            return "INVALID";
    }
}

TString GetResourceSerialName(EResType Type)
{
    TString Name = GetResourceTypeName(Type);
    Name.RemoveWhitespace();
    return Name;
}

TString GetResourceRawExtension(EResType Type, EGame /*Game*/)
{
    if (Type == eWorld) return "mwld";
    return "";
}

TString GetResourceCookedExtension(EResType Type, EGame Game)
{
    if (Game == eUnknownGame)
        Game = ePrime;

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
            ASSERT(FirstGame != eUnknownGame); \
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
REGISTER_RESOURCE_TYPE(AGSC, eAudioGroup, ePrimeDemo, eCorruptionProto)
REGISTER_RESOURCE_TYPE(ANCS, eAnimSet, ePrimeDemo, eEchoes)
REGISTER_RESOURCE_TYPE(ANIM, eAnimation, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(ATBL, eAudioLookupTable, ePrimeDemo, eCorruption)
REGISTER_RESOURCE_TYPE(BFRC, eBurstFireData, eCorruptionProto, eCorruption)
REGISTER_RESOURCE_TYPE(CAAD, eUnknown_CAAD, eCorruption, eCorruption)
REGISTER_RESOURCE_TYPE(CAUD, eAudioMacro, eCorruptionProto, eReturns)
REGISTER_RESOURCE_TYPE(CHAR, eAnimSet, eCorruptionProto, eReturns)
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
REGISTER_RESOURCE_TYPE(PATH, ePathfinding, ePrimeDemo, eCorruption)
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
// Split Area Data
REGISTER_RESOURCE_TYPE(AMAT, eAreaMaterials, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(AGEO, eAreaGeometry, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(AOCT, eAreaOctree, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(ABOX, eAreaSurfaceBounds, eEchoesDemo, eReturns)
REGISTER_RESOURCE_TYPE(ACLN, eAreaCollision, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(ALIT, eAreaLights, ePrimeDemo, eReturns)
REGISTER_RESOURCE_TYPE(AVIS, eAreaVisibilityTree, ePrimeDemo, eReturns)
