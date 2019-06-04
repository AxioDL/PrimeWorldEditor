#ifndef ERESOURCETYPE
#define ERESOURCETYPE

#include <Common/EGame.h>
#include <Common/TString.h>

enum class EResourceType
{
    Animation,
    AnimCollisionPrimData,
    AnimEventData,
    AnimSet,
    Area,
    AreaCollision,
    AreaGeometry,
    AreaLights,
    AreaMaterials,
    AreaSurfaceBounds,
    AreaOctree,
    AreaVisibilityTree,
    AudioAmplitudeData,
    AudioGroup,
    AudioMacro,
    AudioSample,
    AudioLookupTable,
    BinaryData,
    BurstFireData,
    Character,
    DependencyGroup,
    DynamicCollision,
    Font,
    GuiFrame,
    GuiKeyFrame,
    HintSystem,
    MapArea,
    MapWorld,
    MapUniverse,
    Midi,
    Model,
    MusicTrack,
    Package,
    Particle,
    ParticleCollisionResponse,
    ParticleDecal,
    ParticleElectric,
    ParticleSorted,
    ParticleSpawn,
    ParticleSwoosh,
    ParticleTransform,
    ParticleWeapon,
    Pathfinding,
    PortalArea,
    Resource,
    RuleSet,
    SaveArea,
    SaveWorld,
    Scan,
    Skeleton,
    Skin,
    SourceAnimData,
    SpatialPrimitive,
    StateMachine,
    StateMachine2, // For distinguishing AFSM/FSM2
    StaticGeometryMap,
    StreamedAudio,
    StringList,
    StringTable,
    Texture,
    Tweaks,
    UserEvaluatorData,
    Video,
    World,

    Invalid = -1
};

// Defined in CResTypeInfo.cpp
void Serialize(IArchive& rArc, EResourceType& rType);

#endif // ERESOURCETYPE

