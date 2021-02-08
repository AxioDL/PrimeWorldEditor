#include "CVertexArrayManager.h"

// ************ STATIC MEMBER INITIALIZATION ************
std::vector<CVertexArrayManager*> CVertexArrayManager::sVAManagers;
CVertexArrayManager *CVertexArrayManager::spCurrentManager;

// ************ CONSTRUCTORS/DESTRUCTORS ************
CVertexArrayManager::CVertexArrayManager()
{
    mVectorIndex = sVAManagers.size();
    sVAManagers.push_back(this);
}

CVertexArrayManager::~CVertexArrayManager()
{
    for (auto it = mVBOMap.begin(); it != mVBOMap.end(); it = mVBOMap.begin())
        DeleteVAO(it->first);

    for (auto it = mDynamicVBOMap.begin(); it != mDynamicVBOMap.end(); it = mDynamicVBOMap.begin())
        DeleteVAO(it->first);

    sVAManagers.erase(sVAManagers.begin() + mVectorIndex);

    if (sVAManagers.size() > mVectorIndex)
    {
        for (auto it = sVAManagers.begin() + mVectorIndex; it != sVAManagers.end(); it++)
            (*it)->mVectorIndex--;
    }
}

// ************ PUBLIC ************
void CVertexArrayManager::SetCurrent()
{
    spCurrentManager = this;
}

void CVertexArrayManager::BindVAO(CVertexBuffer *pVBO)
{
    const auto it = mVBOMap.find(pVBO);

    if (it != mVBOMap.cend())
    {
        glBindVertexArray(it->second);
    }
    else
    {
        const GLuint VAO = pVBO->CreateVAO();
        mVBOMap.insert_or_assign(pVBO, VAO);
        glBindVertexArray(VAO);
    }
}

void CVertexArrayManager::BindVAO(CDynamicVertexBuffer *pVBO)
{
    // Overload for CDynamicVertexBuffer
    const auto it = mDynamicVBOMap.find(pVBO);

    if (it != mDynamicVBOMap.cend())
    {
        glBindVertexArray(it->second);
    }
    else
    {
        const GLuint VAO = pVBO->CreateVAO();
        mDynamicVBOMap.insert_or_assign(pVBO, VAO);
        glBindVertexArray(VAO);
    }
}

void CVertexArrayManager::DeleteVAO(CVertexBuffer *pVBO)
{
    const auto it = mVBOMap.find(pVBO);

    if (it == mVBOMap.cend())
        return;

    glDeleteVertexArrays(1, &it->second);
    mVBOMap.erase(it);
}

void CVertexArrayManager::DeleteVAO(CDynamicVertexBuffer *pVBO)
{
    // Overload for CDynamicVertexBuffer
    const auto it = mDynamicVBOMap.find(pVBO);

    if (it == mDynamicVBOMap.cend())
        return;

    glDeleteVertexArrays(1, &it->second);
    mDynamicVBOMap.erase(it);
}

// ************ STATIC ************
CVertexArrayManager* CVertexArrayManager::Current()
{
    return spCurrentManager;
}

void CVertexArrayManager::DeleteAllArraysForVBO(CVertexBuffer *pVBO)
{
    for (auto* vam : sVAManagers)
        vam->DeleteVAO(pVBO);
}

void CVertexArrayManager::DeleteAllArraysForVBO(CDynamicVertexBuffer *pVBO)
{
    for (auto* vam : sVAManagers)
        vam->DeleteVAO(pVBO);
}
