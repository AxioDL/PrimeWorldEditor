#include "CScriptLayer.h"

CScriptLayer::CScriptLayer()
{
    mLayerName = "New Layer";
    mActive = true;
    mVisible = true;
}

CScriptLayer::~CScriptLayer()
{
    for (auto it = mObjects.begin(); it != mObjects.end(); it++)
        delete *it;
}

// ************* DATA MANIPULATION *************
void CScriptLayer::AddObject(CScriptObject* object)
{
    mObjects.push_back(object);
}

void CScriptLayer::DeleteObjectByIndex(u32 index)
{
    delete mObjects[index];
    mObjects.erase(mObjects.begin() + index, mObjects.begin() + index);
}

void CScriptLayer::DeleteObjectByID(u32 ID)
{
    for (auto it = mObjects.begin(); it != mObjects.end(); it++)
    {
        if ((*it)->InstanceID() == ID)
        {
            delete *it;
            mObjects.erase(it, it);
            break;
        }
    }
}

void CScriptLayer::Reserve(u32 amount)
{
    mObjects.reserve(amount);
}

// ************* GETTERS *************
TString CScriptLayer::Name()
{
    return mLayerName;
}

bool CScriptLayer::IsActive()
{
    return mActive;
}

bool CScriptLayer::IsVisible()
{
    return mVisible;
}

u32 CScriptLayer::GetNumObjects()
{
    return mObjects.size();
}

CScriptObject* CScriptLayer::ObjectByIndex(u32 index)
{
    return mObjects[index];
}

CScriptObject* CScriptLayer::ObjectByID(u32 ID)
{
    for (auto it = mObjects.begin(); it != mObjects.end(); it++)
        if ((*it)->InstanceID() == ID)
            return *it;

    return nullptr;
}

// ************* SETTERS *************
void CScriptLayer::SetName(const TString& name)
{
    mLayerName = name;
}

void CScriptLayer::SetActive(bool active)
{
    mActive = active;
}

void CScriptLayer::SetVisible(bool visible)
{
    mVisible = visible;
}
