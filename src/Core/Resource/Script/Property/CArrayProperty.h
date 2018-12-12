#ifndef CARRAYPROPERTY_H
#define CARRAYPROPERTY_H

#include "IProperty.h"

struct SScriptArray
{
    int Count;
    std::vector<char> Array;

    SScriptArray()
        : Count(0)
    {}

    inline bool operator==(const SScriptArray& rkOther) const
    {
        return( Count == rkOther.Count && Array == rkOther.Array );
    }
};

/** You probably shouldn't use this on intrinsic classes; script only */
/** @todo proper support of default values for arrays (this would be used for prefabs) */
class CArrayProperty : public TTypedProperty<uint32, EPropertyType::Array>
{
    friend class IProperty;

    /** This class inherits from TTypedPropertyNew<int> in order to expose the array
     *  count value (the first member of SScriptArray). Outside users can edit this
     *  value and we respond by updating the allocated space, handling item destruction
     *  and construction, etc.
     */
    IProperty* mpItemArchetype;

    /** Internal functions */
    SScriptArray& _GetInternalArray(void* pData) const
    {
        return *( (SScriptArray*) RawValuePtr(pData) );
    }

    uint32 _InternalArrayCount(void* pPropertyData) const
    {
        std::vector<char>& rArray = _GetInternalArray(pPropertyData).Array;
        return rArray.size() / ItemSize();
    }

protected:
    CArrayProperty(EGame Game)
        : TTypedProperty(Game)
        , mpItemArchetype(nullptr)
    {}

public:
    virtual uint32 DataSize() const
    {
        return sizeof(SScriptArray);
    }

    virtual uint32 DataAlignment() const
    {
        return alignof(SScriptArray);
    }

    virtual void Construct(void* pData) const
    {
        new(ValuePtr(pData)) SScriptArray;
    }

    virtual void Destruct(void* pData) const
    {
        RevertToDefault(pData);
        TTypedProperty::Destruct(pData);
    }

    virtual bool MatchesDefault(void* pData) const
    {
        return ArrayCount(pData) == 0;
    }

    virtual void RevertToDefault(void* pData) const
    {
        Resize(pData, 0);
        ValueRef(pData) = 0;
    }

    virtual bool CanHaveDefault() const
    {
        return true;
    }

    virtual bool IsPointerType() const
    {
        return true;
    }

    virtual void* GetChildDataPointer(void* pPropertyData) const
    {
        return _GetInternalArray(pPropertyData).Array.data();
    }

    virtual void PropertyValueChanged(void* pPropertyData)
    {
        SScriptArray& rArray = _GetInternalArray(pPropertyData);
        rArray.Count = Math::Max(rArray.Count, 0);
        Resize(pPropertyData, rArray.Count);
    }

    virtual void Serialize(IArchive& rArc)
    {
        TTypedProperty::Serialize(rArc);
        rArc << SerialParameter("ItemArchetype", mpItemArchetype);
    }

    virtual void SerializeValue(void* pData, IArchive& Arc) const
    {
        uint32 Count = ArrayCount(pData);
        Arc.SerializeArraySize(Count);

        if (Arc.IsReader())
            Resize(pData, Count);

        for (uint32 ItemIdx = 0; ItemIdx < Count; ItemIdx++)
        {
            if (Arc.ParamBegin("ArrayElement", 0))
            {
                void* pItemData = ItemPointer(pData, ItemIdx);
                mpItemArchetype->SerializeValue(pItemData, Arc);
                Arc.ParamEnd();
            }
        }
    }

    virtual void InitFromArchetype(IProperty* pOther)
    {
        TTypedProperty::InitFromArchetype(pOther);
        CArrayProperty* pOtherArray = static_cast<CArrayProperty*>(pOther);
        mpItemArchetype = IProperty::CreateCopy(pOtherArray->mpItemArchetype);
    }

    virtual void PostInitialize()
    {
        TTypedProperty::PostInitialize();
        mpItemArchetype->Initialize(this, mpScriptTemplate, 0);
    }

    uint32 ArrayCount(void* pPropertyData) const
    {
        return ValueRef(pPropertyData);
    }

    void Resize(void* pPropertyData, uint32 NewCount) const
    {
        uint32 OldCount = _InternalArrayCount(pPropertyData);

        if (OldCount != NewCount)
        {
            SScriptArray& rArray = _GetInternalArray(pPropertyData);

            // Handle destruction of old elements
            if (OldCount > NewCount)
            {
                for (uint32 ItemIdx = NewCount; ItemIdx < OldCount; ItemIdx++)
                {
                    void* pItemPtr = ItemPointer(pPropertyData, ItemIdx);
                    mpItemArchetype->Destruct(pItemPtr);
                }
            }

            uint32 NewSize = NewCount * ItemSize();
            rArray.Array.resize(NewSize);
            rArray.Count = NewCount;

            // Handle construction of new elements
            if (NewCount > OldCount)
            {
                for (uint32 ItemIdx = OldCount; ItemIdx < NewCount; ItemIdx++)
                {
                    void* pItemPtr = ItemPointer(pPropertyData, ItemIdx);
                    mpItemArchetype->Construct(pItemPtr);
                }
            }
        }
    }

    void* ItemPointer(void* pPropertyData, uint32 ItemIndex) const
    {
        ASSERT(_InternalArrayCount(pPropertyData) > ItemIndex);
        std::vector<char>& rArray = _GetInternalArray(pPropertyData).Array;
        uint32 MyItemSize = ItemSize();
        ASSERT(rArray.size() >= (MyItemSize * (ItemIndex+1)));
        return rArray.data() + (MyItemSize * ItemIndex);
    }

    uint32 ItemSize() const
    {
        uint32 ItemAlign = mpItemArchetype->DataAlignment();
        uint32 ItemSize = ALIGN(mpItemArchetype->DataSize(), ItemAlign);
        return ItemSize;
    }

    /** Accessors */
    IProperty* ItemArchetype() const { return mpItemArchetype; }
};

#endif // CARRAYPROPERTY_H
