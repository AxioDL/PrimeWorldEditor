#include "CResTypeInfo.h"
#include <Common/Macros.h>
#include <algorithm>

std::unordered_map<EResourceType, CResTypeInfo*> CResTypeInfo::smTypeMap;

CResTypeInfo::CResTypeInfo(EResourceType Type, const TString& rkTypeName, const TString& rkRetroExtension)
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

bool CResTypeInfo::IsInGame(EGame Game) const
{
    for (uint32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
    {
        if (mCookedExtensions[iGame].Game == Game)
            return true;
    }
    return false;
}

CFourCC CResTypeInfo::CookedExtension(EGame Game) const
{
    // Default to MP1
    if (Game == EGame::Invalid)
        Game = EGame::Prime;

    for (uint32 iGame = 0; iGame < mCookedExtensions.size(); iGame++)
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
    static EGame sCachedGame = EGame::Invalid;
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
        errorf("Failed to find resource type for cooked extension: %s", *Ext.ToString());
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

    rArc.SerializePrimitive(Ext, 0);

    if (rArc.IsReader())
    {
        rpType = CResTypeInfo::TypeForCookedExtension(rArc.Game(), Ext);
        ASSERT(rpType != nullptr);
    }
}

void Serialize(IArchive& rArc, EResourceType& rType)
{
    CFourCC Extension;

    if (rArc.IsWriter())
    {
        CResTypeInfo* pTypeInfo = CResTypeInfo::FindTypeInfo(rType);
        ASSERT(pTypeInfo != nullptr);
        Extension = pTypeInfo->CookedExtension(rArc.Game());
    }

    rArc.SerializePrimitive(Extension, 0);

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
    ASSERT(FirstGame >= EGame::PrimeDemo && LastGame <= EGame::DKCReturns && FirstGame <= LastGame);
    ASSERT(FirstGame != EGame::Invalid && LastGame != EGame::Invalid);

    for (int GameIdx = (int) FirstGame; GameIdx <= (int) LastGame; GameIdx++)
    {
#if !PUBLIC_RELEASE
        ASSERT(!pType->IsInGame((EGame) GameIdx));
#endif

        CResTypeInfo::SGameExtension Info { (EGame) GameIdx, Ext };
        pType->mCookedExtensions.push_back(Info);
    }

    std::sort(pType->mCookedExtensions.begin(), pType->mCookedExtensions.end(), [](const CResTypeInfo::SGameExtension& rkLeft, const CResTypeInfo::SGameExtension& rkRight) -> bool {
        return rkLeft.Game < rkRight.Game;
    });
}

void CResTypeInfo::CResTypeInfoFactory::InitTypes()
{
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Animation, "Animation", "ani");
        AddExtension(pType, "ANIM", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AnimCollisionPrimData, "Animation Collision Primitive Data", "?");
        AddExtension(pType, "CPRM", EGame::DKCReturns, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AnimEventData, "Animation Event Data", "evnt");
        AddExtension(pType, "EVNT", EGame::PrimeDemo, EGame::Prime);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AnimSet, "Animation Character Set", "acs");
        AddExtension(pType, "ANCS", EGame::PrimeDemo, EGame::Echoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Area, "Area", "mrea");
        AddExtension(pType, "MREA", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AudioAmplitudeData, "Audio Amplitude Data", "?");
        AddExtension(pType, "CAAD", EGame::Corruption, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AudioGroup, "Audio Group", "agsc");
        AddExtension(pType, "AGSC", EGame::PrimeDemo, EGame::CorruptionProto);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AudioMacro, "Audio Macro", "caud");
        AddExtension(pType, "CAUD", EGame::CorruptionProto, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AudioSample, "Audio Sample", "csmp");
        AddExtension(pType, "CSMP", EGame::CorruptionProto, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::AudioLookupTable, "Audio Lookup Table", "atbl");
        AddExtension(pType, "ATBL", EGame::PrimeDemo, EGame::Corruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::BinaryData, "Generic Data", "dat");
        AddExtension(pType, "DUMB", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::BurstFireData, "Burst Fire Data", "bfre.bfrc");
        AddExtension(pType, "BFRC", EGame::CorruptionProto, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Character, "Character", "char");
        AddExtension(pType, "CHAR", EGame::CorruptionProto, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::DependencyGroup, "Dependency Group", "?");
        AddExtension(pType, "DGRP", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::DynamicCollision, "Dynamic Collision", "dcln");
        AddExtension(pType, "DCLN", EGame::PrimeDemo, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Font, "Font", "rpff");
        AddExtension(pType, "FONT", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::GuiFrame, "Gui Frame", "frme");
        AddExtension(pType, "FRME", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::GuiKeyFrame, "Gui Keyframe", "?");
        AddExtension(pType, "KFAM", EGame::PrimeDemo, EGame::PrimeDemo);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::HintSystem, "Hint System Data", "hint");
        AddExtension(pType, "HINT", EGame::Prime, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::MapArea, "Area Map", "mapa");
        AddExtension(pType, "MAPA", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::MapWorld, "World Map", "mapw");
        AddExtension(pType, "MAPW", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::MapUniverse, "Universe Map", "mapu");
        AddExtension(pType, "MAPU", EGame::PrimeDemo, EGame::Echoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Midi, "MIDI", "?");
        AddExtension(pType, "CSNG", EGame::PrimeDemo, EGame::Echoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Model, "Model", "cmdl");
        AddExtension(pType, "CMDL", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Particle, "Particle System", "gpsm.part");
        AddExtension(pType, "PART", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleCollisionResponse, "Collision Response Particle System", "crsm.crsc");
        AddExtension(pType, "CRSC", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleDecal, "Decal Particle System", "dpsm.dpsc");
        AddExtension(pType, "DPSC", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleElectric, "Electric Particle System", "elsm.elsc");
        AddExtension(pType, "ELSC", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleSorted, "Sorted Particle System", "srsm.srsc");
        AddExtension(pType, "SRSC", EGame::EchoesDemo, EGame::Echoes);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleSpawn, "Spawn Particle System", "spsm.spsc");
        AddExtension(pType, "SPSC", EGame::EchoesDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleSwoosh, "Swoosh Particle System", "swsh.swhc");
        AddExtension(pType, "SWHC", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleTransform, "Transform Particle System", "xfsm.xfsc");
        AddExtension(pType, "XFSC", EGame::DKCReturns, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::ParticleWeapon, "Weapon Particle System", "wpsm.wpsc");
        AddExtension(pType, "WPSC", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Pathfinding, "Pathfinding Mesh", "path");
        AddExtension(pType, "PATH", EGame::PrimeDemo, EGame::Corruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::PortalArea, "Portal Area", "?");
        AddExtension(pType, "PTLA", EGame::EchoesDemo, EGame::Corruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::RuleSet, "Rule Set", "rule");
        AddExtension(pType, "RULE", EGame::EchoesDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::SaveArea, "Area Save Info", "sava");
        AddExtension(pType, "SAVA", EGame::CorruptionProto, EGame::Corruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::SaveWorld, "World Save Info", "savw");
        AddExtension(pType, "SAVW", EGame::Prime, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Scan, "Scan", "scan");
        AddExtension(pType, "SCAN", EGame::PrimeDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Skeleton, "Skeleton", "cin");
        AddExtension(pType, "CINF", EGame::PrimeDemo, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Skin, "Skin", "cskr");
        AddExtension(pType, "CSKR", EGame::PrimeDemo, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::SourceAnimData, "Source Animation Data", "sand");
        AddExtension(pType, "SAND", EGame::CorruptionProto, EGame::Corruption);
        pType->mCanHaveDependencies = false; // all dependencies are added to the CHAR dependency tree
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::SpatialPrimitive, "Spatial Primitive", "?");
        AddExtension(pType, "CSPP", EGame::EchoesDemo, EGame::Echoes);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StateMachine, "State Machine", "afsm");
        AddExtension(pType, "AFSM", EGame::PrimeDemo, EGame::Echoes);
        AddExtension(pType, "FSM2", EGame::CorruptionProto, EGame::Corruption);
        AddExtension(pType, "FSMC", EGame::DKCReturns, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StateMachine2, "State Machine 2", "fsm2");
        AddExtension(pType, "FSM2", EGame::EchoesDemo, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StaticGeometryMap, "Static Scan Map", "egmc");
        AddExtension(pType, "EGMC", EGame::EchoesDemo, EGame::Corruption);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StreamedAudio, "Streamed Audio", "?");
        AddExtension(pType, "STRM", EGame::CorruptionProto, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StringList, "String List", "stlc");
        AddExtension(pType, "STLC", EGame::EchoesDemo, EGame::CorruptionProto);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::StringTable, "String Table", "strg");
        AddExtension(pType, "STRG", EGame::PrimeDemo, EGame::DKCReturns);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Texture, "Texture", "txtr");
        AddExtension(pType, "TXTR", EGame::PrimeDemo, EGame::DKCReturns);
        pType->mCanHaveDependencies = false;
        pType->mCanBeSerialized = true;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::Tweaks, "Tweak Data", "ctwk");
        AddExtension(pType, "CTWK", EGame::PrimeDemo, EGame::Prime);
        pType->mCanHaveDependencies = false;
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::UserEvaluatorData, "User Evaluator Data", "user.usrc");
        AddExtension(pType, "USRC", EGame::CorruptionProto, EGame::Corruption);
    }
    {
        CResTypeInfo *pType = new CResTypeInfo(EResourceType::World, "World", "mwld");
        AddExtension(pType, "MLVL", EGame::PrimeDemo, EGame::DKCReturns);
        pType->mCanBeSerialized = true;
    }
}
