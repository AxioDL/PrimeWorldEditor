#ifndef COMPRESSIONUTIL_H
#define COMPRESSIONUTIL_H

#include <Common/BasicTypes.h>
#include <Common/FileIO.h>
#include <Common/TString.h>

namespace CompressionUtil
{
    // Decompression
    bool DecompressZlib(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut);
    bool DecompressLZO(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut);
    bool DecompressSegmentedData(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen);

    // Compression
    bool CompressZlib(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut);
    bool CompressLZO(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut);
    bool CompressSegmentedData(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool IsZlib, bool AllowUncompressedSegments);
    bool CompressZlibSegmented(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool AllowUncompressedSegments);
    bool CompressLZOSegmented(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool AllowUncompressedSegments);
}

#endif // COMPRESSIONUTIL_H
