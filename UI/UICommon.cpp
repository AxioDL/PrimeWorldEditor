#include "UICommon.h"

// This array is intended to be used with the EResType enum
const QString gskResourceFilters[] = {
   "Animation (*.ANIM)",               // eAnimation
   "Animation Event Data (*.EVNT)",    // eAnimEventData
   "Area (*.MREA)",                    // eArea
   "Audio Metadata (*.CAUD)",          // eAudioData
   "Audio Group (*.AGSC)",             // eAudioGrp
   "Audio Sample (*.CSMP)",            // eAudioSample
   "Audio Stream (*.STRM)",            // eAudioStream
   "Audio Lookup Table (*.ATBL)",      // eAudioTable
   "Character (*.ANCS *.CHAR)",        // eCharacter
   "Collision Mesh (*.DCLN)",          // eCollisionMesh
   "Collision Response Data (*.CRSC)", // eCollisionResponse
   "Data Dump (*.DUMB)",               // eDataDump
   "Decal (*.DPSC)",                   // eDecal
   "Dependency Group (*.DGRP)",        // eDependencyGroup
   "Font (*.FONT)",                    // eFont
   "GUI Frame (*.FRME)",               // eGuiFrame
   "Hint System Data (*.HINT)",        // eHintSystem
   "Invalid resource type",            // eInvalid
   "Area Map (*.MAPA)",                // eMapArea
   "World Map (*.MAPW)",               // eMapWorld
   "Universe Map (*.MAPU)",            // eMapUniverse
   "MIDI Data (*.CSNG)",               // eMidi
   "Model (*.CMDL)",                   // eModel
   "Music Track (*.DSP)",              // eMusicTrack
   "Navigation Mesh (*.PATH)",         // eNavMesh
   "Pack File (*.pak)",                // ePackFile
   "Particle (*.PART)",                // eParticle
   "Electricity Particle (*.ELSC)",    // eParticleElectric
   "Swoosh Particle (*.SWHC)",         // eParticleSwoosh
   "Projectile (*.WPSC)",              // eProjectile
   "Invalid resource type",            // eResource
   "World Save Data (*.SAVW)",         // eSaveWorld
   "Scannable Object Info (*.SCAN)",   // eScan
   "Skeleton (*.CINF)",                // eSkeleton
   "Skin (*.CSKR)",                    // eSkin
   "State Machine (*.AFSM *.FSM2)",    // eStateMachine
   "String Table (*.STRG)",            // eStringTable
   "Texture (*.TXTR)",                 // eTexture
   "Tweak (*.CTWK *.ntwk)",            // eTweak
   "Video (*.thp)",                    // eVideo
   "World (*.MLVL)"                    // eWorld
};
