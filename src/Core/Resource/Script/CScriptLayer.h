#ifndef CSCRIPTLAYER_H
#define CSCRIPTLAYER_H

#include "CScriptObject.h"
#include "Core/Resource/CDependencyGroup.h"
#include <Common/BasicTypes.h>
#include <algorithm>
#include <string>
#include <vector>

class CScriptLayer
{
    CGameArea *mpArea;
    TString mLayerName{"New Layer"};
    bool mActive = true;
    bool mVisible = true;
    std::vector<CScriptObject*> mInstances;
public:
    explicit CScriptLayer(CGameArea *pArea)
        : mpArea(pArea)
    {
    }

    ~CScriptLayer()
    {
        for (auto* instance : mInstances)
            delete instance;
    }

    // Data Manipulation
    void AddInstance(CScriptObject *pObject, uint32 Index = UINT32_MAX)
    {
        if (Index != UINT32_MAX && Index < mInstances.size())
        {
            auto it = mInstances.begin();
            std::advance(it, Index);
            mInstances.insert(it, pObject);
        }
        else
        {
            mInstances.push_back(pObject);
        }
    }

    void RemoveInstance(CScriptObject *pInstance)
    {
        const auto it = std::find_if(mInstances.cbegin(), mInstances.cend(),
                                     [pInstance](const auto* instance) { return instance == pInstance; });

        if (it == mInstances.cend())
            return;

        mInstances.erase(it);
    }

    void RemoveInstanceByIndex(uint32 Index)
    {
        mInstances.erase(mInstances.begin() + Index);
    }

    void RemoveInstanceByID(uint32 ID)
    {
        const auto it = std::find_if(mInstances.cbegin(), mInstances.cend(),
                                     [ID](const auto* instance) { return instance->InstanceID() == ID; });

        if (it == mInstances.cend())
            return;

        mInstances.erase(it);
    }

    void Reserve(uint32 Amount)
    {
        mInstances.reserve(Amount);
    }

    // Accessors
    CGameArea* Area() const      { return mpArea; }
    TString Name() const         { return mLayerName; }
    bool IsActive() const        { return mActive; }
    bool IsVisible() const       { return mVisible; }
    uint32 NumInstances() const  { return mInstances.size(); }
    CScriptObject* InstanceByIndex(uint32 Index) const { return mInstances[Index]; }

    CScriptObject* InstanceByID(uint32 ID) const
    {
        const auto it = std::find_if(mInstances.begin(), mInstances.end(),
                                     [ID](const auto* instance) { return instance->InstanceID() == ID; });

        if (it == mInstances.cbegin())
            return nullptr;

        return *it;
    }

    void SetName(TString rkName)   { mLayerName = std::move(rkName); }
    void SetActive(bool Active)    { mActive = Active; }
    void SetVisible(bool Visible)  { mVisible = Visible; }

    uint32 AreaIndex() const
    {
        for (uint32 iLyr = 0; iLyr < mpArea->NumScriptLayers(); iLyr++)
        {
            if (mpArea->ScriptLayer(iLyr) == this)
                return iLyr;
        }

        return UINT32_MAX;
    }

    // Operators
    CScriptObject* operator[](uint32 Index) { return InstanceByIndex(Index); }
    const CScriptObject* operator[](uint32 Index) const { return InstanceByIndex(Index); }
};

#endif // CSCRIPTLAYER_H
