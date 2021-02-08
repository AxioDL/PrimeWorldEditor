#include "CFlagsProperty.h"
#include "Core/Resource/Script/CGameTemplate.h"

void CFlagsProperty::Serialize(IArchive& rArc)
{
    TSerializeableTypedProperty::Serialize(rArc);

    auto* pArchetype = static_cast<CFlagsProperty*>(mpArchetype);

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

    for (const auto& flag : mBitFlags)
        mAllFlags |= flag.Mask;
}

void CFlagsProperty::SerializeValue(void* pData, IArchive& rArc) const
{
    rArc.SerializePrimitive((uint32&)ValueRef(pData), SH_HexDisplay);
}

void CFlagsProperty::InitFromArchetype(IProperty* pOther)
{
    TSerializeableTypedProperty::InitFromArchetype(pOther);
    auto* pOtherFlags = static_cast<CFlagsProperty*>(pOther);
    mBitFlags = pOtherFlags->mBitFlags;
    mAllFlags = pOtherFlags->mAllFlags;
}

TString CFlagsProperty::ValueAsString(void* pData) const
{
    return TString::FromInt32(Value(pData), 0, 10);
}

/**
 * Checks whether there are any unrecognized bits toggled on in the property value.
 * Returns the mask of any invalid bits. If all bits are valid, returns 0.
 */
uint32 CFlagsProperty::HasValidValue(void* pPropertyData) const
{
    if (!mAllFlags)
        return 0;

    return ValueRef(pPropertyData) & ~mAllFlags;
}
