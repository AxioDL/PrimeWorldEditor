#include "CStructProperty.h"
#include "Core/Resource/Script/CMasterTemplate.h"

EPropertyTypeNew CStructPropertyNew::Type() const
{
    return EPropertyTypeNew::Struct;
}

u32 CStructPropertyNew::DataSize() const
{
    if (!mChildren.empty())
    {
        IPropertyNew* pLastChild = mChildren.back();
        return (pLastChild->Offset() - Offset()) + pLastChild->DataSize();
    }
    else
    {
        return 0;
    }
}

u32 CStructPropertyNew::DataAlignment() const
{
    // TODO. Should be aligned with the first child, but this function is called before children are loaded.
    // So for now just use 8 to ensure correct alignment for all child types, but this is wasteful...
    // It's also problematic for casting property data to a struct
    return 8;

    //return (mChildren.empty() ? 1 : mChildren[0]->DataAlignment());
}

void CStructPropertyNew::Construct(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->Construct(pData);
    }
}

void CStructPropertyNew::Destruct(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->Destruct(pData);
    }
}

bool CStructPropertyNew::MatchesDefault(void* pData) const
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

void CStructPropertyNew::RevertToDefault(void* pData) const
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        mChildren[ChildIdx]->RevertToDefault(pData);
    }
}

const char* CStructPropertyNew::HashableTypeName() const
{
    if (IsArchetype() || !mpArchetype)
        return *mName;
    else
        return mpArchetype->HashableTypeName();
}

void CStructPropertyNew::Serialize(IArchive& rArc)
{
    IPropertyNew::Serialize(rArc);

    // Serialize archetype
    if (mpArchetype)
    {
        CStructPropertyNew* pArchetype = static_cast<CStructPropertyNew*>(mpArchetype);
        ASSERT(pArchetype != nullptr);

        if (rArc.IsReader())
        {
            // We've initialized from the archetypes, now serialize parameter overrides
            if (rArc.ParamBegin("SubProperties", 0))
            {
                u32 NumChildOverrides;
                rArc.SerializeArraySize(NumChildOverrides);

                for (u32 ChildIdx = 0; ChildIdx < NumChildOverrides; ChildIdx++)
                {
                    if (rArc.ParamBegin("Element", SH_IgnoreName))
                    {
                        // Serialize type and ID, then look up the matching property and serialize it.
                        // We don't really need the type, but it's a good sanity check, and it's also helpful
                        // to guarantee that parameters are read in order, as some serializers are order-dependent.
                        EPropertyTypeNew ChildType;
                        u32 ChildID;

                        rArc << SerialParameter("Type", ChildType, SH_Attribute)
                             << SerialParameter("ID", ChildID, SH_Attribute);

                        IPropertyNew* pChild = ChildByID(ChildID);
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
            std::vector<IPropertyNew*> PropertiesToSerialize;

            for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
            {
                if (mChildren[ChildIdx]->ShouldSerialize())
                {
                    PropertiesToSerialize.push_back(mChildren[ChildIdx]);
                }
            }

            u32 NumChildOverrides = PropertiesToSerialize.size();

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

void CStructPropertyNew::SerializeValue(void* pData, IArchive& Arc) const
{
    for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (Arc.ParamBegin("Property", 0))
        {
            mChildren[ChildIdx]->SerializeValue(pData, Arc);
            Arc.ParamEnd();
        }
    }
}

bool CStructPropertyNew::ShouldSerialize() const
{
    if (IPropertyNew::ShouldSerialize())
        return true;

    for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (mChildren[ChildIdx]->ShouldSerialize())
            return true;
    }

    return false;
}

TString CStructPropertyNew::GetTemplateFileName()
{
    ASSERT(IsArchetype() || mpArchetype);
    return IsArchetype() ? mTemplateFileName : mpArchetype->GetTemplateFileName();
}
