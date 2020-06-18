#ifndef CSCRIPTLAYER_H
#define CSCRIPTLAYER_H

#include "CScriptObject.h"
#include "Core/Resource/CDependencyGroup.h"
#include <Common/BasicTypes.h>
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
        for (auto it = mInstances.begin(); it != mInstances.end(); it++)
            delete *it;
    }

    // Data Manipulation
    void AddInstance(CScriptObject *pObject, uint32 Index = -1)
    {
        if (Index != -1 && Index < mInstances.size())
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
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
        {
            if (*it == pInstance)
            {
                mInstances.erase(it);
                break;
            }
        }
    }

    void RemoveInstanceByIndex(uint32 Index)
    {
        mInstances.erase(mInstances.begin() + Index);
    }

    void RemoveInstanceByID(uint32 ID)
    {
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
        {
            if ((*it)->InstanceID() == ID)
            {
                mInstances.erase(it);
                break;
            }
        }
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
        for (auto it = mInstances.begin(); it != mInstances.end(); ++it)
        {
            if ((*it)->InstanceID() == ID)
                return *it;
        }

        return nullptr;
    }

    void SetName(const TString& rkName)  { mLayerName = rkName; }
    void SetActive(bool Active)          { mActive = Active; }
    void SetVisible(bool Visible)        { mVisible = Visible; }

    uint32 AreaIndex() const
    {
        for (uint32 iLyr = 0; iLyr < mpArea->NumScriptLayers(); iLyr++)
        {
            if (mpArea->ScriptLayer(iLyr) == this)
                return iLyr;
        }

        return -1;
    }

    // Operators
    CScriptObject* operator[](uint32 Index) { return InstanceByIndex(Index); }
};

#endif // CSCRIPTLAYER_H
