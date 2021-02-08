#ifndef CTEXTUREENCODER_H
#define CTEXTUREENCODER_H

#include "Core/Resource/CTexture.h"
#include "Core/Resource/TResPtr.h"

// Class contains basic functionality right now - only supports directly converting DXT1 to CMPR
// More advanced functions (including actual encoding!) coming later
class CTextureEncoder
{
    TResPtr<CTexture> mpTexture{nullptr};
    ETexelFormat mSourceFormat{};
    ETexelFormat mOutputFormat{};

    CTextureEncoder();
    void WriteTXTR(IOutputStream& rTXTR);
    void DetermineBestOutputFormat();
    void ReadSubBlockCMPR(IInputStream& rSource, IOutputStream& rDest);

public:
    static void EncodeTXTR(IOutputStream& rTXTR, CTexture *pTex);
    static void EncodeTXTR(IOutputStream& rTXTR, CTexture *pTex, ETexelFormat OutputFormat);
    static ETexelFormat GetGXFormat(ETexelFormat Format);
    static ETexelFormat GetFormat(ETexelFormat Format);
};

#endif // CTEXTUREENCODER_H
