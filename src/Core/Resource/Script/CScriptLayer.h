#ifndef SSCRIPTLAYER_H
#define SSCRIPTLAYER_H

#include "CScriptObject.h"
#include <Common/types.h>
#include <string>
#include <vector>

class CScriptLayer
{
    TString mLayerName;
    bool mActive;
    bool mVisible;
    std::vector<CScriptObject*> mObjects;
public:
    CScriptLayer();
    ~CScriptLayer();

    // Data Manipulation
    void AddObject(CScriptObject* object);
    void DeleteObjectByIndex(u32 index);
    void DeleteObjectByID(u32 ID);
    void Reserve(u32 amount);

    // Getters and Setters
    TString Name();
    bool IsActive();
    bool IsVisible();
    u32 GetNumObjects();
    CScriptObject* ObjectByIndex(u32 index);
    CScriptObject* ObjectByID(u32 ID);

    void SetName(const TString& name);
    void SetActive(bool active);
    void SetVisible(bool visible);

    // Operators
    CScriptObject* operator[](u32 index);
};

// ************* INLINE FUNCTIONS *************
inline CScriptObject* CScriptLayer::operator[](u32 index)
{
    return mObjects[index];
}


#endif // SSCRIPTLAYER_H
