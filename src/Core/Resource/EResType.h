#ifndef ERESTYPE
#define ERESTYPE

#include "EGame.h"
#include <Common/TString.h>

enum EResType
{
    eAnimation,
    eAnimCollisionPrimData,
    eAnimEventData,
    eAnimSet,
    eArea,
    eAudioMacro,
    eAudioGroupSet,
    eAudioSample,
    eStreamedAudio,
    eAudioLookupTable,
    eBinaryData,
    eBurstFireData,
    eCharacter,
    eDependencyGroup,
    eDynamicCollision,
    eFont,
    eGuiFrame,
    eGuiKeyFrame,
    eHintSystem,
    eInvalidResType,
    eMapArea,
    eMapWorld,
    eMapUniverse,
    eMidi,
    eModel,
    eMusicTrack,
    eNavMesh,
    ePackage,
    eParticle,
    eParticleCollisionResponse,
    eParticleDecal,
    eParticleElectric,
    eParticleSorted,
    eParticleSpawn,
    eParticleSwoosh,
    eParticleTransform,
    eParticleWeapon,
    ePortalArea,
    eResource,
    eRuleSet,
    eSaveArea,
    eSaveWorld,
    eScan,
    eSkeleton,
    eSkin,
    eSourceAnimData,
    eSpatialPrimitive,
    eStateMachine,
    eStateMachine2, // For distinguishing AFSM/FSM2
    eStaticGeometryMap,
    eStringList,
    eStringTable,
    eTexture,
    eTweak,
    eUnknown_CAAD,
    eUserEvaluatorData,
    eVideo,
    eWorld
};

// defined in CResource.cpp
TString GetTypeName(EResType Type);
TString GetRawExtension(EResType Type, EGame Game);
TString GetCookedExtension(EResType Type, EGame Game);

#endif // ERESTYPE

