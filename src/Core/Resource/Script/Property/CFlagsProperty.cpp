#include "CFlagsProperty.h"
#include "Core/Resource/Script/CGameTemplate.h"

void CFlagsProperty::Serialize(IArchive& rArc)
{
    TSerializeableTypedProperty::Serialize(rArc);

    CFlagsProperty* pArchetype = static_cast<CFlagsProperty*>(mpArchetype);

    if (!pArchetype || !rArc.CanSkipParameters() || mBitFlags != pArchetype->mBitFlags)
    {
        rArc << SerialParameter("Flags", mBitFlags);
    }
}

void CFlagsProperty::PostInitialize()
{
    TSerializeableTypedProperty::PostInitialize();

    // Create AllFlags mask
    mAllFlags = 0;

    for (int FlagIdx = 0; FlagIdx < mBitFlags.size(); FlagIdx++)
        mAllFlags |= mBitFlags[FlagIdx].Mask;
}

void CFlagsProperty::SerializeValue(void* pData, IArchive& rArc) const
{
    rArc.SerializePrimitive( (u32&) ValueRef(pData), SH_HexDisplay );
}

void CFlagsProperty::InitFromArchetype(IProperty* pOther)
{
    TSerializeableTypedProperty::InitFromArchetype(pOther);
    CFlagsProperty* pOtherFlags = static_cast<CFlagsProperty*>(pOther);
    mBitFlags = pOtherFlags->mBitFlags;
    mAllFlags = pOtherFlags->mAllFlags;
}

/**
 * Checks whether there are any unrecognized bits toggled on in the property value.
 * Returns the mask of any invalid bits. If all bits are valid, returns 0.
 */
u32 CFlagsProperty::HasValidValue(void* pPropertyData)
{
    if (!mAllFlags) return 0;
    return ValueRef(pPropertyData) & ~mAllFlags;
}
