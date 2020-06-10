#ifndef CARRAYPROPERTY_H
#define CARRAYPROPERTY_H

#include "IProperty.h"

struct SScriptArray
{
    int Count = 0;
    std::vector<char> Array;

    SScriptArray() = default;

    bool operator==(const SScriptArray& other) const
    {
        return Count == other.Count && Array == other.Array;
    }

    bool operator!=(const SScriptArray& other) const
    {
        return !operator==(other);
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
    IProperty* mpItemArchetype = nullptr;

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
    explicit CArrayProperty(EGame Game)
        : TTypedProperty(Game)
    {}

public:
    ~CArrayProperty() override { delete mpItemArchetype; }

    uint32 DataSize() const override
    {
        return sizeof(SScriptArray);
    }

    uint32 DataAlignment() const override
    {
        return alignof(SScriptArray);
    }

    void Construct(void* pData) const override
    {
        new(ValuePtr(pData)) SScriptArray;
    }

    void Destruct(void* pData) const override
    {
        RevertToDefault(pData);
        _GetInternalArray(pData).~SScriptArray();
    }

    bool MatchesDefault(void* pData) const override
    {
        return ArrayCount(pData) == 0;
    }

    void RevertToDefault(void* pData) const override
    {
        Resize(pData, 0);
        ValueRef(pData) = 0;
    }

    bool CanHaveDefault() const override
    {
        return true;
    }

    bool IsPointerType() const override
    {
        return true;
    }

    void* GetChildDataPointer(void* pPropertyData) const override
    {
        return _GetInternalArray(pPropertyData).Array.data();
    }

    void PropertyValueChanged(void* pPropertyData) override
    {
        SScriptArray& rArray = _GetInternalArray(pPropertyData);
        rArray.Count = Math::Max(rArray.Count, 0);
        Resize(pPropertyData, rArray.Count);
    }

    void Serialize(IArchive& rArc) override
    {
        TTypedProperty::Serialize(rArc);
        rArc << SerialParameter("ItemArchetype", mpItemArchetype);
    }

    void SerializeValue(void* pData, IArchive& Arc) const override
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

    void InitFromArchetype(IProperty* pOther) override
    {
        TTypedProperty::InitFromArchetype(pOther);
        CArrayProperty* pOtherArray = static_cast<CArrayProperty*>(pOther);
        mpItemArchetype = IProperty::CreateCopy(pOtherArray->mpItemArchetype);
    }

    void PostInitialize() override
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
        uint32 ItemSize = VAL_ALIGN(mpItemArchetype->DataSize(), ItemAlign);
        return ItemSize;
    }

    /** Accessors */
    IProperty* ItemArchetype() const { return mpItemArchetype; }
};

#endif // CARRAYPROPERTY_H
