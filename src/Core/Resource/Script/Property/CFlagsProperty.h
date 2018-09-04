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
            rArc << SerialParameter("Name", Name)
                 << SerialParameter("Mask", Mask, SH_HexDisplay);
        }
    };
    std::vector<SBitFlag> mBitFlags;
    u32 mAllFlags;

    /** XML template file that this enum originated from; for archetypes */
    TString mSourceFile;

    CFlagsProperty()
        : TSerializeableTypedProperty()
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

    virtual void Serialize(IArchive& rArc)
    {
        TSerializeableTypedProperty::Serialize(rArc);
        rArc << SerialParameter("Flags", mBitFlags);
    }

    virtual void PostInitialize()
    {
        TTypedPropertyNew::PostInitialize();

        // Create AllFlags mask
        mAllFlags = 0;

        for (int FlagIdx = 0; FlagIdx < mBitFlags.size(); FlagIdx++)
            mAllFlags |= mBitFlags[FlagIdx].Mask;
    }

    virtual void SerializeValue(void* pData, IArchive& rArc) const
    {
        rArc.SerializePrimitive( (u32&) ValueRef(pData), SH_HexDisplay );
    }

    virtual void InitFromArchetype(IPropertyNew* pOther)
    {
        TTypedPropertyNew::InitFromArchetype(pOther);
        CFlagsProperty* pOtherFlags = static_cast<CFlagsProperty*>(pOther);
        mBitFlags = pOtherFlags->mBitFlags;
        mAllFlags = pOtherFlags->mAllFlags;
    }

    virtual TString GetTemplateFileName()
    {
        ASSERT(IsArchetype() || mpArchetype);
        return IsArchetype() ? mSourceFile : mpArchetype->GetTemplateFileName();
    }

    /**
     * Checks whether there are any unrecognized bits toggled on in the property value.
     * Returns the mask of any invalid bits. If all bits are valid, returns 0.
     */
    u32 HasValidValue(void* pPropertyData)
    {
        return ValueRef(pPropertyData) & ~mAllFlags;
    }
};

#endif // CFLAGSPROPERTY_H
