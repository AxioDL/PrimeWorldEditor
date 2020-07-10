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

    void SetNode(CSceneNode *pNode)
    {
        mNodeID = pNode ? pNode->ID() : 0;
        mpScene = pNode ? pNode->Scene() : nullptr;
        mValid = pNode ? true : false;
    }

    bool Valid() const       { return mValid; }
    uint32 NodeID() const    { return mNodeID; }
    CScene* Scene() const    { return mpScene; }
    CSceneNode* operator* () const { return mValid ? mpScene->NodeByID(mNodeID) : nullptr; }
    CSceneNode* operator->() const { return mValid ? mpScene->NodeByID(mNodeID) : nullptr; }
    CNodePtr& operator=(CSceneNode *pNode) { SetNode(pNode); return *this; }

    bool operator==(const CNodePtr& rkOther) const
    {
        return (mNodeID == rkOther.mNodeID && mpScene == rkOther.mpScene);
    }

    bool operator!=(const CNodePtr& other) const
    {
        return !operator==(other);
    }
};

class CInstancePtr
{
    CInstanceID mInstanceID;
    CGameArea *mpArea;
    bool mValid;

public:
    CInstancePtr()                      { SetInstance(nullptr); }
    CInstancePtr(CScriptObject *pInst)  { SetInstance(pInst); }

    void SetInstance(CScriptObject *pInst)
    {
        mInstanceID = pInst ? pInst->InstanceID() : CInstanceID();
        mpArea = pInst ? pInst->Area() : nullptr;
        mValid = pInst ? true : false;
    }

    CInstanceID InstanceID() const   { return mInstanceID; }
    CGameArea* Area() const  { return mpArea; }
    CScriptObject* operator* () const { return mValid ? mpArea->InstanceByID(mInstanceID) : nullptr; }
    CScriptObject* operator->() const { return mValid ? mpArea->InstanceByID(mInstanceID) : nullptr; }
    CInstancePtr& operator=(CScriptObject *pInst) { SetInstance(pInst); return *this; }

    bool operator==(const CInstancePtr& rkOther) const
    {
        return (mInstanceID == rkOther.mInstanceID && mpArea == rkOther.mpArea);
    }

    bool operator!=(const CInstancePtr& other) const
    {
        return !operator==(other);
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

    void SetLink(CLink *pLink)
    {
        mpInstance = pLink ? pLink->Sender() : 0;
        mLinkIndex = pLink ? pLink->SenderIndex() : 0;
        mValid = pLink ? true : false;
    }

    uint32 LinkIndex() const     { return mLinkIndex; }
    CLink* operator* () const    { return mValid ? mpInstance->Link(ELinkType::Outgoing, mLinkIndex) : nullptr; }
    CLink* operator->() const    { return mValid ? mpInstance->Link(ELinkType::Outgoing, mLinkIndex) : nullptr; }
    CLinkPtr& operator=(CLink *pLink) { SetLink(pLink); return *this; }

    bool operator==(const CLinkPtr& rkOther) const
    {
        return (mpInstance == rkOther.mpInstance && mLinkIndex == rkOther.mLinkIndex);
    }

    bool operator!=(const CLinkPtr& other) const
    {
        return !operator==(other);
    }
};

DEFINE_PTR_LIST_CLASS(CNodePtrList, CNodePtr, CSceneNode*)
DEFINE_PTR_LIST_CLASS(CInstancePtrList, CInstancePtr, CScriptObject*)
DEFINE_PTR_LIST_CLASS(CLinkPtrList, CLinkPtr, CLink*)

#endif // OBJREFERENCES

