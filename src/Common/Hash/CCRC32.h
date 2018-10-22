#ifndef CCRC32_H
#define CCRC32_H

#include "Common/TString.h"
#include "Common/types.h"

/**
 * CRC32 hash implementation
 */
class CCRC32
{
    /** Current hash value */
    u32 mHash;

public:
    /** Default constructor, initializes the hash to the default value */
    CCRC32();

    /** Allows the hash to be initialized to an arbitrary value */
    CCRC32(u32 InitialValue);

    /** Hash arbitrary data */
    void Hash(const void* pkData, int Size);

    /** Retrieve the final output hash. (You can keep adding data to the hash after calling this.) */
    u32 Digest() const;

    /** Convenience hash methods */
    void Hash(u8 v);
    void Hash(u16 v);
    void Hash(u32 v);
    void Hash(u64 v);
    void Hash(float v);
    void Hash(double v);
    void Hash(char v);
    void Hash(const char* pkString);

    static u32 StaticHashString(const char* pkString);
};

#endif // CCRC32_H
