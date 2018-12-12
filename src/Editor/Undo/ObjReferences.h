#ifndef OBJREFERENCES
#define OBJREFERENCES

#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/Script/CLink.h>
#include <Core/Resource/Script/CScriptObject.h>
#include <Core/Scene/CScene.h>
#include <Core/Scene/CSceneNode.h>
#include <QList>

/*
 * The basic idea here is that over the course of editing and generally making changes
 * in the editor, stuff tends to get created and deleted. When things are deleted they
 * are actually deleted from memory, and recreated from scratch on undo. This poses a
 * problem for undo actions that need to operate on a specific object because if their
 * target object has been deleted and remade then the undo actions are stuck with a
 * garbage pointer to deleted data. This generally results in crashes.
 *
 * These pointer classes solve the problem by encapsulating a reference to the object
 * using IDs and indices instead of storing a direct pointer to the object itself. This
 * way the correct object is always looked up from the area/scene/etc that it's from and
 * the pointer is always valid. They are designed to be as easy and pain-free to use as
 * possible.
 *
 * Most of this stuff could be moved to Core if it turns out to be useful in other places.
 * QList dependencies would need to be removed but they could easily be replaced with
 * std::list instead.
 */

#define DEFINE_PTR_LIST_CLASS(ClassName, PtrClassName, DereferencedClassName) \
    class ClassName : public QList<PtrClassName> \
    { \
    public: \
        ClassName() : QList<PtrClassName>() {} \
        \
        ClassName(const QList<DereferencedClassName>& rkIn) \
        { \
            foreach (DereferencedClassName InObj, rkIn) \
            { \
                *this << InObj; \
            } \
        } \
        \
        QList<DereferencedClassName> DereferenceList() const \
        { \
            QList<DereferencedClassName> Out; \
            \
            foreach (const PtrClassName& rkPtr, *this) \
                Out << *rkPtr; \
            \
            return Out; \
        } \
    };

class CNodePtr
{
    uint32 mNodeID;
    CScene *mpScene;
    bool mValid;

public:
    CNodePtr()                  { SetNode(nullptr); }
    CNodePtr(CSceneNode *pNode) { SetNode(pNode); }

    inline void SetNode(CSceneNode *pNode)
    {
        mNodeID = pNode ? pNode->ID() : 0;
        mpScene = pNode ? pNode->Scene() : nullptr;
        mValid = pNode ? true : false;
    }

    inline bool Valid() const       { return mValid; }
    inline uint32 NodeID() const    { return mNodeID; }
    inline CScene* Scene() const    { return mpScene; }
    inline CSceneNode* operator* () const { return mValid ? mpScene->NodeByID(mNodeID) : nullptr; }
    inline CSceneNode* operator->() const { return mValid ? mpScene->NodeByID(mNodeID) : nullptr; }
    inline CNodePtr& operator=(CSceneNode *pNode) { SetNode(pNode); return *this; }

    inline bool operator==(const CNodePtr& rkOther) const
    {
        return (mNodeID == rkOther.mNodeID && mpScene == rkOther.mpScene);
    }
};

class CInstancePtr
{
    uint32 mInstanceID;
    CGameArea *mpArea;
    bool mValid;

public:
    CInstancePtr()                      { SetInstance(nullptr); }
    CInstancePtr(CScriptObject *pInst)  { SetInstance(pInst); }

    inline void SetInstance(CScriptObject *pInst)
    {
        mInstanceID = pInst ? pInst->InstanceID() : 0;
        mpArea = pInst ? pInst->Area() : nullptr;
        mValid = pInst ? true : false;
    }

    inline uint32 InstanceID() const   { return mInstanceID; }
    inline CGameArea* Area() const  { return mpArea; }
    inline CScriptObject* operator* () const { return mValid ? mpArea->InstanceByID(mInstanceID) : nullptr; }
    inline CScriptObject* operator->() const { return mValid ? mpArea->InstanceByID(mInstanceID) : nullptr; }
    inline CInstancePtr& operator=(CScriptObject *pInst) { SetInstance(pInst); return *this; }

    inline bool operator==(const CInstancePtr& rkOther) const
    {
        return (mInstanceID == rkOther.mInstanceID && mpArea == rkOther.mpArea);
    }
};

class CLinkPtr
{
    CInstancePtr mpInstance;
    uint32 mLinkIndex;
    bool mValid;

public:
    CLinkPtr()              { SetLink(nullptr); }
    CLinkPtr(CLink *pLink)  { SetLink(pLink); }

    inline void SetLink(CLink *pLink)
    {
        mpInstance = pLink ? pLink->Sender() : 0;
        mLinkIndex = pLink ? pLink->SenderIndex() : 0;
        mValid = pLink ? true : false;
    }

    inline uint32 LinkIndex() const        { return mLinkIndex; }
    inline CLink* operator* () const    { return mValid ? mpInstance->Link(eOutgoing, mLinkIndex) : nullptr; }
    inline CLink* operator->() const    { return mValid ? mpInstance->Link(eOutgoing, mLinkIndex) : nullptr; }
    inline CLinkPtr& operator=(CLink *pLink) { SetLink(pLink); return *this; }

    inline bool operator==(const CLinkPtr& rkOther) const
    {
        return (mpInstance == rkOther.mpInstance && mLinkIndex == rkOther.mLinkIndex);
    }
};

DEFINE_PTR_LIST_CLASS(CNodePtrList, CNodePtr, CSceneNode*)
DEFINE_PTR_LIST_CLASS(CInstancePtrList, CInstancePtr, CScriptObject*)
DEFINE_PTR_LIST_CLASS(CLinkPtrList, CLinkPtr, CLink*)

#endif // OBJREFERENCES

