#include "CStructProperty.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include <algorithm>

EPropertyType CStructProperty::Type() const
{
    return EPropertyType::Struct;
}

void CStructProperty::PostInitialize()
{
    IProperty::PostInitialize();

    // All structs should have an archetype.
    ASSERT( IsRootParent() || mpArchetype != nullptr );
}

uint32 CStructProperty::DataSize() const
{
    if (!mChildren.empty())
    {
        IProperty* pLastChild = mChildren.back();
        return (pLastChild->Offset() - Offset()) + pLastChild->DataSize();
    }
    else
    {
        return 0;
    }
}

uint32 CStructProperty::DataAlignment() const
{
    // Structs are aligned to the first child property.
    return (mChildren.empty() ? 1 : mChildren[0]->DataAlignment());
}

void CStructProperty::Construct(void* pData) const
{
    for (auto* child : mChildren)
    {
        child->Construct(pData);
    }
}

void CStructProperty::Destruct(void* pData) const
{
    for (auto* child : mChildren)
    {
        child->Destruct(pData);
    }
}

bool CStructProperty::MatchesDefault(void* pData) const
{
    return std::any_of(mChildren.cbegin(), mChildren.cend(),
                       [pData](const auto* child) { return child->MatchesDefault(pData); });
}

void CStructProperty::RevertToDefault(void* pData) const
{
    for (auto* child : mChildren)
    {
        child->RevertToDefault(pData);
    }
}

void CStructProperty::SetDefaultFromData(void* pData)
{
    for (auto* child : mChildren)
    {
        child->SetDefaultFromData(pData);
    }
}

const char* CStructProperty::HashableTypeName() const
{
    return mpArchetype ? mpArchetype->HashableTypeName() : *mName;
}

void CStructProperty::Serialize(IArchive& rArc)
{
    IProperty::Serialize(rArc);

    // Serialize atomic flag
    // Only serialize this if we don't have an archetype. Otherwise we just inherit the archetype's atomic flag.
    if (!mpArchetype)
    {
        bool Atomic = IsAtomic();
        rArc << SerialParameter("Atomic", Atomic, SH_Optional, false);

        if (rArc.IsReader() && Atomic)
        {
            mFlags.SetFlag(EPropertyFlag::IsAtomic);
        }
    }

    // Serialize archetype
    if (mpArchetype)
    {
        [[maybe_unused]] const CStructProperty* pArchetype = static_cast<CStructProperty*>(mpArchetype);
        ASSERT(pArchetype != nullptr);

        if (rArc.IsReader())
        {
            // We've initialized from the archetypes, now serialize parameter overrides
            if (rArc.ParamBegin("SubProperties", 0))
            {
                uint32 NumChildOverrides;
                rArc.SerializeArraySize(NumChildOverrides);

                for (uint32 ChildIdx = 0; ChildIdx < NumChildOverrides; ChildIdx++)
                {
                    if (rArc.ParamBegin("Element", SH_IgnoreName))
                    {
                        // Serialize type and ID, then look up the matching property and serialize it.
                        // We don't really need the type, but it's a good sanity check, and it's also good practice
                        // to guarantee that parameters are read in order, as some serializers are order-dependent.
                        EPropertyType ChildType;
                        uint32 ChildID;

                        rArc << SerialParameter("Type", ChildType, SH_Attribute)
                             << SerialParameter("ID", ChildID, SH_Attribute | SH_HexDisplay );

                        IProperty* pChild = ChildByID(ChildID);
                        ASSERT(pChild != nullptr && pChild->Type() == ChildType);
                        pChild->Serialize(rArc);

                        rArc.ParamEnd();
                    }
                }

                rArc.ParamEnd();
            }
        }
        else
        {
            // Check if any properties need to override parameters from their archetype.
            std::vector<IProperty*> PropertiesToSerialize;

            for (auto* child : mChildren)
            {
                if (child->ShouldSerialize())
                {
                    PropertiesToSerialize.push_back(child);
                }
            }

            uint32 NumChildOverrides = PropertiesToSerialize.size();

            if (NumChildOverrides > 0)
            {
                rArc << SerialParameter("SubProperties", PropertiesToSerialize);
            }
        }
    }
    else
    {
        rArc << SerialParameter("SubProperties", mChildren);
    }
}

void CStructProperty::SerializeValue(void* pData, IArchive& Arc) const
{
    for (auto* child : mChildren)
    {
        if (Arc.ParamBegin("Property", 0))
        {
            child->SerializeValue(pData, Arc);
            Arc.ParamEnd();
        }
    }
}

void CStructProperty::InitFromArchetype(IProperty* pOther)
{
    IProperty::InitFromArchetype(pOther);

    // Copy children
    _ClearChildren();
    mChildren.reserve(pOther->NumChildren());

    for (size_t ChildIdx = 0; ChildIdx < pOther->NumChildren(); ChildIdx++)
    {
        IProperty* pChild = CreateCopy(pOther->ChildByIndex(ChildIdx));
        mChildren.push_back(pChild);
    }
}

bool CStructProperty::ShouldSerialize() const
{
    if (IProperty::ShouldSerialize())
        return true;

    return std::any_of(mChildren.cbegin(), mChildren.cend(),
                       [](const auto* child) { return child->ShouldSerialize(); });
}
