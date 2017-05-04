#ifndef COMPRESSIONUTIL_H
#define COMPRESSIONUTIL_H

#include <Common/FileIO.h>
#include <Common/TString.h>
#include <Common/types.h>

namespace CompressionUtil
{
    TString ErrorText_zlib(s32 Error);
    TString ErrorText_LZO(s32 Error);

    // Decompression
    bool DecompressZlib(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen, u32& rTotalOut);
    bool DecompressLZO(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut);
    bool DecompressSegmentedData(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen);

    // Compression
    bool CompressZlib(u8 *pSrc, u32 SrcLen, u8 *pDst, u32 DstLen, u32& rTotalOut);
    bool CompressLZO(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut);
    bool CompressSegmentedData(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut, bool IsZlib, bool AllowUncompressedSegments);
    bool CompressZlibSegmented(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut, bool AllowUncompressedSegments);
    bool CompressLZOSegmented(u8 *pSrc, u32 SrcLen, u8 *pDst, u32& rTotalOut, bool AllowUncompressedSegments);
}

#endif // COMPRESSIONUTIL_H
