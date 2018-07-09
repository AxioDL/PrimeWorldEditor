#ifndef CSTRUCTPROPERTY_H
#define CSTRUCTPROPERTY_H

#include "../IPropertyNew.h"

class CStructPropertyNew : public IPropertyNew
{
    friend class CTemplateLoader;

public:
    // Must be a valid type for TPropertyRef
    typedef void* ValueType;

protected:
    /** For archetypes, the filename of the template XML file. */
    TString mTemplateFileName;

public:
    virtual EPropertyTypeNew Type() const
    {
        return EPropertyTypeNew::Struct;
    }

    virtual u32 DataSize() const
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

    virtual u32 DataAlignment() const
    {
        // TODO. Should be aligned with the first child, but this function is called before children are loaded.
        // So for now just use 8 to ensure correct alignment for all child types, but this is wasteful...
        return 8;

        //return (mChildren.empty() ? 1 : mChildren[0]->DataAlignment());
    }

    virtual void Construct(void* pData) const
    {
        for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
        {
            mChildren[ChildIdx]->Construct(pData);
        }
    }

    virtual void Destruct(void* pData) const
    {
        for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
        {
            mChildren[ChildIdx]->Destruct(pData);
        }
    }

    virtual bool MatchesDefault(void* pData) const
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

    virtual void RevertToDefault(void* pData) const
    {
        for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
        {
            mChildren[ChildIdx]->RevertToDefault(pData);
        }
    }

    virtual const char* HashableTypeName() const
    {
        if (IsArchetype() || !mpArchetype)
            return *mName;
        else
            return mpArchetype->HashableTypeName();
    }

#if 0
    virtual void Serialize(IArchive& rArc)
    {
        IPropertyNew::Serialize(rArc);

        if (rArc.ParamBegin("SubProperties"))
        {
            u32 NumChildren;
            rArc.SerializeContainerSize(NumChildren, "Property");

            if (rArc.IsReader())
            {
                mChildren.resize(NumChildren);
            }

            for (u32 ChildIdx = 0; ChildIdx < NumChildren; ChildIdx++)
            {
                if (rArc.ParamBegin("Property"))
                {
                    EPropertyTypeNew Type = (rArc.IsWriter() ? mChildren[ChildIdx]->Type() : EPropertyTypeNew::Invalid);
                    rArc << SERIAL_AUTO(Type);

                    if (rArc.IsReader())
                    {
                        mChildren[ChildIdx] = Create(Type, this, mpMasterTemplate, mpScriptTemplate);
                    }

                    mChildren[ChildIdx]->Serialize(rArc);
                    rArc.ParamEnd();
                }
            }

            rArc.ParamEnd();
        }
    }
#endif

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
        {
            if (Arc.ParamBegin("Property"))
            {
                mChildren[ChildIdx]->SerializeValue(pData, Arc);
                Arc.ParamEnd();
            }
        }
    }

    virtual TString GetTemplateFileName()
    {
        ASSERT(IsArchetype() || mpArchetype);
        return IsArchetype() ? mTemplateFileName : mpArchetype->GetTemplateFileName();
    }

    inline static EPropertyTypeNew StaticType() { return EPropertyTypeNew::Struct; }
};

#endif
