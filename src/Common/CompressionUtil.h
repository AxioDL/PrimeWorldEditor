#ifndef COMPRESSIONUTIL_H
#define COMPRESSIONUTIL_H

#include "types.h"

namespace CompressionUtil
{
    bool DecompressAreaLZO(u8 *src, u32 src_len, u8 *dst, u32 dst_len);
}

#endif // COMPRESSIONUTIL_H
