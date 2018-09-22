#ifndef CFLAGSPROPERTY_H
#define CFLAGSPROPERTY_H

#include "../IPropertyNew.h"

class CFlagsProperty : public TSerializeableTypedProperty<u32, EPropertyTypeNew::Flags>
{
    friend class CTemplateLoader;
    friend class IPropertyNew;

    struct SBitFlag
    {
        TString Name;
        u32 Mask;

        SBitFlag()
            : Mask(0)
        {}

        SBitFlag(const TString& rkInName, u32 InMask)
            : Name(rkInName), Mask(InMask)
        {}

        bool operator==(const SBitFlag& rkOther) const
        {
            return( Name == rkOther.Name && Mask == rkOther.Mask );
        }

        void Serialize(IArchive& rArc)
        {
            rArc << SerialParameter("Name", Name, SH_Attribute)
                 << SerialParameter("Mask", Mask, SH_Attribute | SH_HexDisplay);
        }
    };
    std::vector<SBitFlag> mBitFlags;
    u32 mAllFlags;

    /** XML template file that this enum originated from; for archetypes */
    TString mSourceFile;

    CFlagsProperty(EGame Game)
        : TSerializeableTypedProperty(Game)
        , mAllFlags(0)
    {}

public:
    inline u32 NumFlags() const
    {
        return mBitFlags.size();
    }

    inline TString FlagName(u32 Idx) const
    {
        ASSERT(Idx >= 0 && Idx < mBitFlags.size());
        return mBitFlags[Idx].Name;
    }

    inline u32 FlagMask(u32 Idx) const
    {
        ASSERT(Idx >= 0 && Idx < mBitFlags.size());
        return mBitFlags[Idx].Mask;
    }

    virtual void Serialize(IArchive& rArc);
    virtual void PostInitialize();
    virtual void SerializeValue(void* pData, IArchive& rArc) const;
    virtual void InitFromArchetype(IPropertyNew* pOther);
    virtual TString GetTemplateFileName();

    /**
     * Checks whether there are any unrecognized bits toggled on in the property value.
     * Returns the mask of any invalid bits. If all bits are valid, returns 0.
     */
    u32 HasValidValue(void* pPropertyData);
};

#endif // CFLAGSPROPERTY_H
