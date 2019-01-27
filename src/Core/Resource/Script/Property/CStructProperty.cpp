#include "CStructProperty.h"
#include "Core/Resource/Script/CGameTemplate.h"

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
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->Construct(pData);
    }
}

void CStructProperty::Destruct(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->Destruct(pData);
    }
}

bool CStructProperty::MatchesDefault(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (!mChildren[ChildIdx]->MatchesDefault(pData))
        {
            return false;
        }
    }
    return true;
}

void CStructProperty::RevertToDefault(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->RevertToDefault(pData);
    }
}

void CStructProperty::SetDefaultFromData(void* pData)
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->SetDefaultFromData(pData);
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
        CStructProperty* pArchetype = static_cast<CStructProperty*>(mpArchetype);
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

            for (uint32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
            {
                if (mChildren[ChildIdx]->ShouldSerialize())
                {
                    PropertiesToSerialize.push_back(mChildren[ChildIdx]);
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
    for (uint32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (Arc.ParamBegin("Property", 0))
        {
            mChildren[ChildIdx]->SerializeValue(pData, Arc);
            Arc.ParamEnd();
        }
    }
}

void CStructProperty::InitFromArchetype(IProperty* pOther)
{
    IProperty::InitFromArchetype(pOther);

    // Copy children
    _ClearChildren();
    mChildren.reserve( pOther->NumChildren() );

    for (uint32 ChildIdx = 0; ChildIdx < pOther->NumChildren(); ChildIdx++)
    {
        IProperty* pChild = CreateCopy( pOther->ChildByIndex(ChildIdx) );
        mChildren.push_back( pChild );
    }
}

bool CStructProperty::ShouldSerialize() const
{
    if (IProperty::ShouldSerialize())
        return true;

    for (uint32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (mChildren[ChildIdx]->ShouldSerialize())
            return true;
    }

    return false;
}
