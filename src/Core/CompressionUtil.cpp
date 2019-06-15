#include "CompressionUtil.h"
#include <Common/Common.h>

#if USE_LZOKAY
#include <lzokay.hpp>
#else
#include <lzo/lzo1x.h>
#endif

#include <zlib.h>

namespace CompressionUtil
{
    const char* ErrorText_zlib(int32 Error)
    {
        switch (Error)
        {
        case Z_OK:              return "Z_OK";
        case Z_STREAM_END:      return "Z_STREAM_END";
        case Z_NEED_DICT:       return "Z_NEED_DICT";
        case Z_ERRNO:           return "Z_ERRNO";
        case Z_STREAM_ERROR:    return "Z_STREAM_ERROR";
        case Z_DATA_ERROR:      return "Z_DATA_ERROR";
        case Z_MEM_ERROR:       return "Z_MEM_ERROR";
        case Z_BUF_ERROR:       return "Z_BUF_ERROR";
        case Z_VERSION_ERROR:   return "Z_VERSION_ERROR";
        default:                return "UNKNOWN ZLIB ERROR";
        }
    }

#if USE_LZOKAY
    const char* ErrorText_LZO(lzokay::EResult Result)
    {
        switch (Result)
        {
        case lzokay::EResult::LookbehindOverrun:    return "LookbehindOverrun";
        case lzokay::EResult::OutputOverrun:        return "OutputOverrun";
        case lzokay::EResult::InputOverrun:         return "InputOverrun";
        case lzokay::EResult::Error:                return "Error";
        case lzokay::EResult::Success:              return "Success";
        case lzokay::EResult::InputNotConsumed:     return "InputNotConsumed";
        default:                                    return "UNKNOWN LZO ERROR";
        }
    }
#else
    const char* ErrorText_LZO(int32 Error)
    {
        switch (Error)
        {
        case LZO_E_OK:                  return "LZO_E_OK";
        case LZO_E_ERROR:               return "LZO_E_ERROR";
        case LZO_E_EOF_NOT_FOUND:       return "LZO_E_EOF_NOT_FOUND";
        case LZO_E_INPUT_NOT_CONSUMED:  return "LZO_E_INPUT_NOT_CONSUMED";
        case LZO_E_INPUT_OVERRUN:       return "LZO_E_INPUT_OVERRUN";
        case LZO_E_INTERNAL_ERROR:      return "LZO_E_INTERNAL_ERROR";
        case LZO_E_INVALID_ALIGNMENT:   return "LZO_E_INVALID_ALIGNMENT";
        case LZO_E_INVALID_ARGUMENT:    return "LZO_E_INVALID_ARGUMENT";
        case LZO_E_LOOKBEHIND_OVERRUN:  return "LZO_E_LOOKBEHIND_OVERRUN";
        case LZO_E_NOT_COMPRESSIBLE:    return "LZO_E_NOT_COMPRESSIBLE";
        case LZO_E_NOT_YET_IMPLEMENTED: return "LZO_E_NOT_YET_IMPLEMENTED";
        case LZO_E_OUTPUT_NOT_CONSUMED: return "LZO_E_OUTPUT_NOT_CONSUMED";
        case LZO_E_OUTPUT_OVERRUN:      return "LZO_E_OUTPUT_OVERRUN";
        case LZO_E_OUT_OF_MEMORY:       return "LZO_E_OUT_OF_MEMORY";
        default:                        return "UNKNOWN LZO ERROR";
        }
    }
#endif

    // ************ DECOMPRESS ************
    bool DecompressZlib(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut)
    {
        // Initialize z_stream
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = SrcLen;
        z.next_in = pSrc;
        z.avail_out = DstLen;
        z.next_out = pDst;

        // Attempt decompress
        int32 Error = inflateInit(&z);

        if (!Error)
        {
            Error = inflate(&z, Z_NO_FLUSH);

            if (!Error || Error == Z_STREAM_END)
                Error = inflateEnd(&z);

            rTotalOut = z.total_out;
        }

        // Check for errors
        if (Error && Error != Z_STREAM_END)
        {
            errorf("zlib error: %s", ErrorText_zlib(Error));
            return false;
        }

        else return true;
    }

    bool DecompressLZO(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut)
    {
#if USE_LZOKAY
        size_t TotalOut;
        lzokay::EResult Result = lzokay::decompress(pSrc, (size_t) SrcLen, pDst, DstLen, TotalOut);
        rTotalOut = TotalOut;

        if (Result < lzokay::EResult::Success)
        {
            errorf("LZO error: %s", ErrorText_LZO(Result));
            return false;
        }

        return true;
#else
        lzo_init();
        lzo_uint TotalOut;
        int32 Error = lzo1x_decompress(pSrc, SrcLen, pDst, &TotalOut, LZO1X_MEM_DECOMPRESS);
        rTotalOut = (uint32) TotalOut;

        if (Error)
        {
            errorf("LZO error: %s", ErrorText_LZO(Error));
            return false;
        }

        return true;
#endif
    }

    bool DecompressSegmentedData(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen)
    {
        uint8 *pSrcEnd = pSrc + SrcLen;
        uint8 *pDstEnd = pDst + DstLen;

        while ((pSrc < pSrcEnd) && (pDst < pDstEnd))
        {
            // Read size value (this method is Endian-independent)
            uint8 ByteA = *pSrc++;
            uint8 ByteB = *pSrc++;
            int16 Size = (ByteA << 8) | ByteB;

            uint32 TotalOut;

            // Negative size denotes uncompressed data.
            if (Size < 0)
            {
                Size = -Size;
                memcpy(pDst, pSrc, Size);
                pSrc += Size;
                pDst += Size;
            }

            // If size is positive then we have compressed data.
            else
            {
                // Check for zlib magic
                uint8 ByteC = pSrc[0];
                uint8 ByteD = pSrc[1];
                uint16 PeekMagic = (ByteC << 8) | ByteD;

                if (PeekMagic == 0x78DA || PeekMagic == 0x789C || PeekMagic == 0x7801)
                {
                    bool Success = DecompressZlib(pSrc, Size, pDst, (uint32) (pDstEnd - pDst), TotalOut);
                    if (!Success) return false;
                }

                // No zlib magic - this is LZO
                else
                {
                    bool Success = DecompressLZO(pSrc, Size, pDst, (uint32) (pDstEnd - pDst), TotalOut);
                    if (!Success) return false;
                }

                pSrc += Size;
                pDst += TotalOut;
            }
        }

        return ((pSrc == pSrcEnd) && (pDst == pDstEnd));
    }

    // ************ COMPRESS ************
    bool CompressZlib(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut)
    {
        z_stream z;
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;
        z.avail_in = SrcLen;
        z.next_in = pSrc;
        z.avail_out = DstLen;
        z.next_out = pDst;

        int32 Error = deflateInit(&z, 9);

        if (!Error)
        {
            Error = deflate(&z, Z_FINISH);

            if (!Error || Error == Z_STREAM_END)
                Error = deflateEnd(&z);

            rTotalOut = z.total_out;
        }

        if (Error && Error != Z_STREAM_END)
        {
            errorf("zlib error: %s", ErrorText_zlib(Error));
            return false;
        }

        else return true;
    }

    bool CompressLZO(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32 DstLen, uint32& rTotalOut)
    {
#if USE_LZOKAY
        size_t TotalOut;
        lzokay::EResult Result = lzokay::compress(pSrc, (size_t) SrcLen, pDst, DstLen, TotalOut);
        rTotalOut = TotalOut;

        if (Result < lzokay::EResult::Success)
        {
            errorf("LZO error: %s", ErrorText_LZO(Result));
            return false;
        }

        return true;
#else
        lzo_init();

        uint8 *pWorkMem = new uint8[LZO1X_999_MEM_COMPRESS];
        int32 Error = lzo1x_999_compress(pSrc, SrcLen, pDst, (lzo_uint*) &rTotalOut, pWorkMem);
        delete[] pWorkMem;

        if (Error)
        {
            errorf("LZO error: %s", ErrorText_LZO(Error));
            return false;
        }

        return true;
#endif
    }

    bool CompressSegmentedData(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool IsZlib, bool AllowUncompressedSegments)
    {
        uint8 *pSrcEnd = pSrc + SrcLen;
        uint8 *pDstStart = pDst;

        while (pSrc < pSrcEnd)
        {
            // Each segment is compressed separately. Segment size should always be 0x4000 unless there's less than 0x4000 bytes left.
            uint16 Size;
            uint32 Remaining = (uint32) (pSrcEnd - pSrc);

            if (Remaining < 0x4000) Size = (uint16) Remaining;
            else Size = 0x4000;

            std::vector<uint8> Compressed(Size * 2);
            uint32 TotalOut;

            if (IsZlib)
                CompressZlib(pSrc, Size, Compressed.data(), Compressed.size(), TotalOut);
            else
                CompressLZO(pSrc, Size, Compressed.data(), Compressed.size(), TotalOut);

            // Verify that the compressed data is actually smaller.
            if (AllowUncompressedSegments && TotalOut >= Size)
            {
                // Write negative size value to destination (which signifies uncompressed)
                *pDst++ = -Size >> 8;
                *pDst++ = -Size & 0xFF;

                // Write original uncompressed data to destination
                memcpy(pDst, pSrc, Size);
                TotalOut = Size;
            }

            // If it IS smaller, write the compressed data
            else
            {
                // Write new compressed size + data to destination
                *pDst++ = (TotalOut >> 8) & 0xFF;
                *pDst++ = (TotalOut & 0xFF);
                memcpy(pDst, Compressed.data(), TotalOut);
            }

            pSrc += Size;
            pDst += TotalOut;
        }

        rTotalOut = (uint32) (pDst - pDstStart);
        return true;
    }

    bool CompressZlibSegmented(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool AllowUncompressedSegments)
    {
        return CompressSegmentedData(pSrc, SrcLen, pDst, rTotalOut, true, AllowUncompressedSegments);
    }

    bool CompressLZOSegmented(uint8 *pSrc, uint32 SrcLen, uint8 *pDst, uint32& rTotalOut, bool AllowUncompressedSegments)
    {
        return CompressSegmentedData(pSrc, SrcLen, pDst, rTotalOut, false, AllowUncompressedSegments);
    }
}
