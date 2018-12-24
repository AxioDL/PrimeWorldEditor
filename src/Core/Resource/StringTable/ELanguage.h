#ifndef ELANGUAGE_H
#define ELANGUAGE_H

#include <Common/CFourCC.h>

/** A language in the game's localization system */
enum class ELanguage
{
    // The original release of Metroid Prime only supported English
    English                 = FOURCC('ENGL'),
    // Support for these languages was added in the PAL version of Metroid Prime
    German                  = FOURCC('GERM'),
    French                  = FOURCC('FREN'),
    Spanish                 = FOURCC('SPAN'),
    Italian                 = FOURCC('ITAL'),
    Dutch                   = FOURCC('DUTC'), // Unused
    Japanese                = FOURCC('JAPN'),
    // The rest of these languages were added in Donkey Kong Country Returns
    SimplifiedChinese       = FOURCC('SCHN'), // Unused
    TraditionalChinese      = FOURCC('TCHN'), // Unused
    UKEnglish               = FOURCC('UKEN'),
    Korean                  = FOURCC('KORE'),
    NAFrench                = FOURCC('NAFR'),
    NASpanish               = FOURCC('NASP'),
    // Invalid
    Invalid                 = FOURCC('INVD')
};

#endif // ELANGUAGE_H
