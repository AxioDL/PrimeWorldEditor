#include "UICommon.h"

namespace UICommon
{

QMap<QString,QString> FilterMap = {
    { "AFSM", "AI Finite State Machine (*.AFSM)" },
    { "ANIM", "Animation (*.ANIM)" },
    { "ANCS", "Animation Character Set (*.ANCS)" },
    { "AGSC", "Audio Group (*.AGSC)" },
    { "ATBL", "Audio Lookup Table (*.ATBL)" },
    { "CAUD", "Audio Metadata (*.CAUD)" },
    { "CHAR", "Character (*.CHAR)" },
    { "CINF", "Skeleton (*.CINF)" },
    { "CMDL", "Model (*.CMDL)" },
    { "CRSC", "Collision Response Data (*.CRSC)" },
    { "CSKR", "Skin Rules (*.CSKR)" },
    { "CSMP", "Audio Sample (*.CSMP)" },
    { "CSNG", "MIDI Data (*.CSNG)" },
    { "CTWK", "Tweaks (*.CTWK)" },
    { "DCLN", "Collision Mesh (*.DCLN)" },
    { "DGRP", "Dependency Group (*.DGRP)" },
    { "DPSC", "Decal (*.DPSC)" },
    { "DSP",  "Music Track (*.DSP)" },
    { "DUMB", "Binary Data Dump (*.DUMB)" },
    { "ELSC", "Electric Particle (*.ELSC)" },
    { "EVNT", "Animation Event Data (*.EVNT)" },
    { "FRME", "GUI Frame (*.FRME)" },
    { "FSM2", "AI Finite State Machine (*.FSM2)" },
    { "FONT", "Font (*.FONT)" },
    { "HINT", "Hint System Data (*.HINT)" },
    { "MAPA", "Area Map (*.MAPA)" },
    { "MAPW", "World Map (*.MAPW)" },
    { "MAPU", "Universe Map (*.MAPU)" },
    { "MLVL", "World (*.MLVL)" },
    { "MREA", "Area (*.MREA)" },
    { "NTWK", "Tweaks (*.NTWK)" },
    { "PATH", "AI Navigation Mesh (*.PATH)" },
    { "PAK",  "Pack File (*.pak)" },
    { "PART", "Particle (*.PART)" },
    { "SAVW", "World Save Data (*.SAVW)" },
    { "SCAN", "Scannable Object Info (*.SCAN)" },
    { "STRG", "String Table (*.STRG)" },
    { "STRM", "Audio Stream (*.STRM)" },
    { "SWHC", "Swoosh Particle (*.SWHC)" },
    { "THP",  "Video (*.thp)" },
    { "TXTR", "Texture (*.TXTR)" },
    { "WPSC", "Projectile (*.WPSC)" }
};

QString ExtensionFilterString(const QString& extension)
{
    QMap<QString,QString>::const_iterator it = FilterMap.find(extension.toUpper());

    if (it != FilterMap.end())
        return it.value();
    else
        return "Unknown Extension (*." + extension + ")";
}

}
