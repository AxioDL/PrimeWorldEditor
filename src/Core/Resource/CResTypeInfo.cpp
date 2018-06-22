#include "CResTypeInfo.h"
#include <Common/AssertMacro.h>
#include <algorithm>

std::unordered_map<EResType, CResTypeInfo*> CResTypeInfo::smTypeMap;

CResTypeInfo::CResTypeInfo(EResType Type, const TString& rkTypeName, const TString& rkRetroExtension)
    : mType(Type)
    , mTypeName(rkTypeName)
    , mRetroExtension(rkRetroExtension)
    , mCanBeSerialized(false)
    , mCanHaveDependencies(true)
{
#if !PUBLIC_RELEASE
    ASSERT(smTypeMap.find(Type) == smTypeMap.end());
#endif
    smTypeMap[Type] = this;
}

CResTypeInfo::~CResTypeInfo()
{
    // shouldn't happen - we want to just create these at launch and keep them around forever
    ASSERT(false);
}

bool CResTypeInfo::IsInGame(EGame Game) const
{
    for (u32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
    {
        if (mCookedExtensions[iGame].Game == Game)
            return true;
    }
    return false;
}

CFourCC CResTypeInfo::CookedExtension(EGame Game) const
{
    // Default to MP1
    if (Game == eUnknownGame)
        Game = ePrime;

    for (u32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
    {
        if (mCookedExtensions[iGame].Game == Game)
            return mCookedExtensions[iGame].CookedExt;
    }

    return "NONE";
}

// ************ STATIC ************
void CResTypeInfo::GetAllTypesInGame(EGame Game, std::list<CResTypeInfo*>& rOut)
{
    for (auto Iter = smTypeMap.begin(); Iter != smTypeMap.end(); Iter++)
    {
        CResTypeInfo *pType = Iter->second;

        if (pType->IsInGame(Game))
            rOut.push_back(pType);
    }
}

CResTypeInfo* CResTypeInfo::TypeForCookedExtension(EGame Game, CFourCC Ext)
{
    // Extensions can vary between games, but we're not likely to be calling this function for different games very often.
    // So, to speed things up a little, cache the lookup results in a map.
    static EGame sCachedGame = eUnknownGame;
    static std::map<CFourCC, CResTypeInfo*> sCachedTypeMap;
    Ext = Ext.ToUpper();

    // When the game changes, our cache is invalidated, so clear it
    if (sCachedGame != Game)
    {
        sCachedGame = Game;
        sCachedTypeMap.clear();
    }

    // Is this type cached?
    auto Iter = sCachedTypeMap.find(Ext);
    if (Iter != sCachedTypeMap.end())
        return Iter->second;

    // Not cached - do a slow lookup
    for (auto Iter = smTypeMap.begin(); Iter != smTypeMap.end(); Iter++)
    {
        CResTypeInfo *pType = Iter->second;

        if (pType->CookedExtension(Game) == Ext)
        {
            sCachedTypeMap[Ext] = pType;
            return pType;
        }
    }

    // Haven't found it; caller gave us an invalid type
    // Note UNKN is used to indicate unknown asset type
    if (Ext != FOURCC('UNKN'))
    {
        Log::Error("Failed to find resource type for cooked extension: " + Ext.ToString());
        DEBUG_BREAK;
    }
    sCachedTypeMap[Ext] = nullptr;
    return nullptr;
}

// ************ SERIALIZATION ************
void Serialize(IArchive& rArc, CResTypeInfo*& rpType)
{
    CFourCC Ext;

    if (rArc.IsWriter())
    {
        ASSERT(rpType != nullptr);
        Ext = rpType->CookedExtension(rArc.Game());
    }

    rArc.SerializePrimitive(Ext);

    if (rArc.IsReader())
    {
        rpType = CResTypeInfo::TypeForCookedExtension(rArc.Game(), Ext);
        ASSERT(rpType != nullptr);
    }
}

void Serialize(IArchive& rArc, EResType& rType)
{
    CFourCC Extension;

    if (rArc.IsWriter())
    {
        CResTypeInfo* pTypeInfo = CResTypeInfo::FindTypeInfo(rType);
        ASSERT(pTypeInfo != nullptr);
        Extension = pTypeInfo->CookedExtension(rArc.Game());
    }

    rArc.SerializePrimitive(Extension);

    if (rArc.IsReader())
    {
        CResTypeInfo* pTypeInfo = CResTypeInfo::TypeForCookedExtension(rArc.Game(), Extension);
        ASSERT(pTypeInfo != nullptr);
        rType = pTypeInfo->Type();
    }
}

// ************ CREATION ************
CResTypeInfo::CResTypeInfoFactory CResTypeInfo::smTypeInfoFactory;

CResTypeInfo::CResTypeInfoFactory::CResTypeInfoFactory()
{
    InitTypes();
}

void CResTypeInfo::CResTypeInfoFactory::AddExtension(CResTypeInfo *pType, CFourCC Ext, EGame FirstGame, EGame LastGame)
{
    ASSERT(FirstGame >= ePrimeDemo && LastGame <= eReturns && FirstGame <= LastGame);
    ASSERT(FirstGame != eUnknownGame && LastGame != eUnknownGame);

    for (int iGame = FirstGame; iGame <= LastGame; iGame++)
    {
#if !PUBLIC_RELEASE
        ASSERT(!pType->IsInGame((EGame) iGame));
#endif

        CResTypeInfo::SGameExtension Info { (EGame) iGame, Ext };
        pType->mCookedExtensions.push_back(Info);
    }

    std::sort(pType->mCookedExtensions.begin(), pType->mCookedExtensions.end(), [](const CResTypeInfo::SGameExtension& rkLeft, const CResTypeInfo::SGameExtension& rkRight) -> bool {
        return rkLeft.Game < rkRight.Game;
    });
}

void CResTypeInfo::CResTypeInfoFactory::InitTypes()
{
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimation, "Animation", "ani");
        AddExtension(pType, "ANIM", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimCollisionPrimData, "Animation Collision Primitive Data", "?");
        AddExtension(pType, "CPRM", eReturns, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimEventData, "Animation Event Data", "evnt");
        AddExtension(pType, "EVNT", ePrimeDemo, ePrime);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAnimSet, "Animation Character Set", "acs");
        AddExtension(pType, "ANCS", ePrimeDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eArea, "Area", "mrea");
        AddExtension(pType, "MREA", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioAmplitudeData, "Audio Amplitude Data", "?");
        AddExtension(pType, "CAAD", eCorruption, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioGroup, "Audio Group", "agsc");
        AddExtension(pType, "AGSC", ePrimeDemo, eEchoes);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioMacro, "Audio Macro", "caud");
        AddExtension(pType, "CAUD", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioSample, "Audio Sample", "csmp");
        AddExtension(pType, "CSMP", eCorruptionProto, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eAudioLookupTable, "Audio Lookup Table", "atbl");
        AddExtension(pType, "ATBL", ePrimeDemo, eCorruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eBinaryData, "Generic Data", "dat");
        AddExtension(pType, "DUMB", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eBurstFireData, "Burst Fire Data", "bfre.bfrc");
        AddExtension(pType, "BFRC", eCorruptionProto, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eCharacter, "Character", "char");
        AddExtension(pType, "CHAR", eCorruptionProto, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eDependencyGroup, "Dependency Group", "?");
        AddExtension(pType, "DGRP", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eDynamicCollision, "Dynamic Collision", "dcln");
        AddExtension(pType, "DCLN", ePrimeDemo, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eFont, "Font", "rpff");
        AddExtension(pType, "FONT", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eGuiFrame, "Gui Frame", "frme");
        AddExtension(pType, "FRME", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eGuiKeyFrame, "Gui Keyframe", "?");
        AddExtension(pType, "KFAM", ePrimeDemo, ePrimeDemo);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eHintSystem, "Hint System Data", "hint");
        AddExtension(pType, "HINT", ePrime, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapArea, "Area Map", "mapa");
        AddExtension(pType, "MAPA", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapWorld, "World Map", "mapw");
        AddExtension(pType, "MAPW", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMapUniverse, "Universe Map", "mapu");
        AddExtension(pType, "MAPU", ePrimeDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eMidi, "MIDI", "?");
        AddExtension(pType, "CSNG", ePrimeDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eModel, "Model", "cmdl");
        AddExtension(pType, "CMDL", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticle, "Particle System", "gpsm.part");
        AddExtension(pType, "PART", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleCollisionResponse, "Collision Response Particle System", "crsm.crsc");
        AddExtension(pType, "CRSC", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleDecal, "Decal Particle System", "dpsm.dpsc");
        AddExtension(pType, "DPSC", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleElectric, "Electric Particle System", "elsm.elsc");
        AddExtension(pType, "ELSC", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSorted, "Sorted Particle System", "srsm.srsc");
        AddExtension(pType, "SRSC", eEchoesDemo, eEchoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSpawn, "Spawn Particle System", "spsm.spsc");
        AddExtension(pType, "SPSC", eEchoesDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleSwoosh, "Swoosh Particle System", "swsh.swhc");
        AddExtension(pType, "SWHC", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleTransform, "Transform Particle System", "xfsm.xfsc");
        AddExtension(pType, "XFSC", eReturns, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eParticleWeapon, "Weapon Particle System", "wpsm.wpsc");
        AddExtension(pType, "WPSC", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(ePathfinding, "Pathfinding Mesh", "path");
        AddExtension(pType, "PATH", ePrimeDemo, eCorruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(ePortalArea, "Portal Area", "?");
        AddExtension(pType, "PTLA", eEchoesDemo, eCorruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eRuleSet, "Rule Set", "rule");
        AddExtension(pType, "RULE", eEchoesDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSaveArea, "Area Save Info", "sava");
        AddExtension(pType, "SAVA", eCorruptionProto, eCorruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSaveWorld, "World Save Info", "savw");
        AddExtension(pType, "SAVW", ePrime, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eScan, "Scan", "scan");
        AddExtension(pType, "SCAN", ePrimeDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSkeleton, "Skeleton", "cin");
        AddExtension(pType, "CINF", ePrimeDemo, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSkin, "Skin", "cskr");
        AddExtension(pType, "CSKR", ePrimeDemo, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSourceAnimData, "Source Animation Data", "sand");
        AddExtension(pType, "SAND", eCorruptionProto, eCorruption);
        pType->mCanHaveDependencies = false; // all dependencies are added to the CHAR dependency tree
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eSpatialPrimitive, "Spatial Primitive", "?");
        AddExtension(pType, "CSPP", eEchoesDemo, eEchoes);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStateMachine, "State Machine", "afsm");
        AddExtension(pType, "AFSM", ePrimeDemo, eEchoes);
        AddExtension(pType, "FSM2", eCorruptionProto, eCorruption);
        AddExtension(pType, "FSMC", eReturns, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStateMachine2, "State Machine 2", "fsm2");
        AddExtension(pType, "FSM2", eEchoesDemo, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStaticGeometryMap, "Static Scan Map", "egmc");
        AddExtension(pType, "EGMC", eEchoesDemo, eCorruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStreamedAudio, "Streamed Audio", "?");
        AddExtension(pType, "STRM", eCorruptionProto, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStringList, "String List", "stlc");
        AddExtension(pType, "STLC", eEchoesDemo, eCorruptionProto);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eStringTable, "String Table", "strg");
        AddExtension(pType, "STRG", ePrimeDemo, eReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eTexture, "Texture", "txtr");
        AddExtension(pType, "TXTR", ePrimeDemo, eReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eTweak, "Tweak Data", "ctwk");
        AddExtension(pType, "CTWK", ePrimeDemo, ePrime);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eUserEvaluatorData, "User Evaluator Data", "user.usrc");
        AddExtension(pType, "USRC", eCorruptionProto, eCorruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(eWorld, "World", "mwld");
        AddExtension(pType, "MLVL", ePrimeDemo, eReturns);
        pType->mCanBeSerialized = true;
    }
}
