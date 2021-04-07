#ifndef CDOLHEADER_H
#define CDOLHEADER_H

#include <Common/FileIO/IOutputStream.h>
#include <Common/FileIO/IInputStream.h>

class SDolHeader
{
public:
    static const size_t kNumTextSections = 7;
    static const size_t kNumDataSections = 11;
    static const size_t kNumSections = kNumTextSections + kNumDataSections;

    struct Section
    {
        uint32 Offset;
        uint32 BaseAddress;
        uint32 Size;

        bool IsEmpty() const {
            return Size == 0;
        }
    };

    Section Sections[kNumSections];
    uint32 BssAddress;
    uint32 BssSize;
    uint32 EntryPoint;

    explicit SDolHeader(IInputStream& rInput)
    {
        for (size_t i = 0; i < kNumSections; ++i)
        {
            Sections[i].Offset = rInput.ReadLong();
        }
        for (size_t i = 0; i < kNumSections; ++i)
        {
            Sections[i].BaseAddress = rInput.ReadLong();
        }
        for (size_t i = 0; i < kNumSections; ++i)
        {
            Sections[i].Size = rInput.ReadLong();
        }
        BssAddress = rInput.ReadLong();
        BssSize = rInput.ReadLong();
        EntryPoint = rInput.ReadLong();
    }

    void Write(IOutputStream& rOutput) const
    {
        for (size_t i = 0; i < kNumSections; ++i)
        {
            rOutput.WriteLong(Sections[i].Offset);
        }
        for (size_t i = 0; i < kNumSections; ++i)
        {
            rOutput.WriteLong(Sections[i].BaseAddress);
        }
        for (size_t i = 0; i < kNumSections; ++i)
        {
            rOutput.WriteLong(Sections[i].Size);
        }
        rOutput.WriteLong(BssAddress);
        rOutput.WriteLong(BssSize);
        rOutput.WriteLong(EntryPoint);
    }

    bool AddTextSection(uint32 address, uint32 fileOffset, uint32 size)
    {
        if ((size & 0x1f) != 0)
        {
            warnf("Unable to add text section: Size not 32-bit aligned.");
            return false;
        }

        for (size_t i = 0; i < kNumTextSections; ++i)
        {
            if (Sections[i].IsEmpty())
            {
                Sections[i].BaseAddress = address;
                Sections[i].Offset = fileOffset;
                Sections[i].Size = size;
                return true;
            }
        }

        warnf("Unable to add text section: no empty section found.");
        return false;
    }

    uint32 OffsetForAddress(uint32 address)
    {
        for (size_t i = 0; i < kNumSections; ++i)
        {
            auto& sec = Sections[i];
            if (address > sec.BaseAddress && address < sec.BaseAddress + sec.Size)
            {
                return sec.Offset + (address - sec.BaseAddress);
            }
        }
        warnf("Unable to add section for address: %x", address);
        return 0;
    }
};

#endif // SDOLHEADER_H
