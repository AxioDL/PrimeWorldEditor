#ifndef ERESTYPE
#define ERESTYPE

#include <Common/EGame.h>
#include <Common/TString.h>

enum EResType
{
    eAnimation,
    eAnimCollisionPrimData,
    eAnimEventData,
    eAnimSet,
    eArea,
    eAreaCollision,
    eAreaGeometry,
    eAreaLights,
    eAreaMaterials,
    eAreaSurfaceBounds,
    eAreaOctree,
    eAreaVisibilityTree,
    eAudioGroup,
    eAudioMacro,
    eAudioSample,
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
    eMapArea,
    eMapWorld,
    eMapUniverse,
    eMidi,
    eModel,
    eMusicTrack,
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
    ePathfinding,
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
    eStreamedAudio,
    eStringList,
    eStringTable,
    eTexture,
    eTweak,
    eUnknown_CAAD,
    eUserEvaluatorData,
    eVideo,
    eWorld,

    eInvalidResType = -1
};

#endif // ERESTYPE

