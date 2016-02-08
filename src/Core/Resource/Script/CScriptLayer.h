#ifndef CSCRIPTLAYER_H
#define CSCRIPTLAYER_H

#include "CScriptObject.h"
#include <Common/types.h>
#include <string>
#include <vector>

class CScriptLayer
{
    TString mLayerName;
    bool mActive;
    bool mVisible;
    std::vector<CScriptObject*> mInstances;
public:
    CScriptLayer()
        : mLayerName("New Layer")
        , mActive(true)
        , mVisible(true)
    {
    }

    ~CScriptLayer()
    {
        for (auto it = mInstances.begin(); it != mInstances.end(); it++)
            delete *it;
    }

    // Data Manipulation
    void AddInstance(CScriptObject *pObject)
    {
        mInstances.push_back(pObject);
    }

    void RemoveInstance(CScriptObject *pInstance)
    {
        for (auto it = mInstances.begin(); it != mInstances.end(); it++)
        {
            if (*it == pInstance)
            {
                mInstances.erase(it);
                break;
            }
        }
    }

    void RemoveInstanceByIndex(u32 Index)
    {
        mInstances.erase(mInstances.begin() + Index);
    }

    void RemoveInstanceByID(u32 ID)
    {
        for (auto it = mInstances.begin(); it != mInstances.end(); it++)
        {
            if ((*it)->InstanceID() == ID)
            {
                mInstances.erase(it);
                break;
            }
        }
    }

    void Reserve(u32 Amount)
    {
        mInstances.reserve(Amount);
    }

    // Accessors
    inline TString Name() const         { return mLayerName; }
    inline bool IsActive() const        { return mActive; }
    inline bool IsVisible() const       { return mVisible; }
    inline u32 NumInstances() const     { return mInstances.size(); }
    inline CScriptObject* InstanceByIndex(u32 Index) const { return mInstances[Index]; }

    inline CScriptObject* InstanceByID(u32 ID) const
    {
        for (auto it = mInstances.begin(); it != mInstances.end(); it++)
        {
            if ((*it)->InstanceID() == ID)
                return *it;
        }

        return nullptr;
    }

    inline void SetName(const TString& rkName)  { mLayerName = rkName; }
    inline void SetActive(bool Active)          { mActive = Active; }
    inline void SetVisible(bool Visible)        { mVisible = Visible; }

    // Operators
    CScriptObject* operator[](u32 Index) { return InstanceByIndex(Index); }
};

#endif // CSCRIPTLAYER_H
