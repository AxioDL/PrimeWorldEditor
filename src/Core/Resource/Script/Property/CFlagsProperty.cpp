#include "Core/Resource/Script/Property/CFlagsProperty.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <string>

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

void CFlagsProperty::SerializeValue(void* pData, IArchive& rArc)
{
    rArc.SerializePrimitive(ValueRef(pData), SH_HexDisplay);
}

const char* CFlagsProperty::HashableTypeName() const
{
    if (mpArchetype)
    {
        return mpArchetype->HashableTypeName();
    }
    else if (IsArchetype() && IsRootParent() && !mName.IsEmpty())
    {
        return mName.CString();
    }
    else
    {
        return "Flags";
    }
}

void CFlagsProperty::InitFromArchetype(IProperty* pOther)
{
    TSerializeableTypedProperty::InitFromArchetype(pOther);
    auto* pOtherFlags = static_cast<CFlagsProperty*>(pOther);
    mBitFlags = pOtherFlags->mBitFlags;
    mAllFlags = pOtherFlags->mAllFlags;
}

TString CFlagsProperty::ValueAsString(const void* pData) const
{
    return std::to_string(Value(pData));
}

/**
 * Checks whether there are any unrecognized bits toggled on in the property value.
 * Returns the mask of any invalid bits. If all bits are valid, returns 0.
 */
uint32_t CFlagsProperty::HasValidValue(const void* pPropertyData) const
{
    if (!mAllFlags)
        return 0;

    return ValueRef(pPropertyData) & ~mAllFlags;
}
