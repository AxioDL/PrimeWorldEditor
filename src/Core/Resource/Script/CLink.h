#ifndef CLINK_H
#define CLINK_H

#include "CScriptObject.h"
#include "Core/Resource/CGameArea.h"
#include <Common/TString.h>
#include <Common/types.h>

struct SState
{
    u32 ID;
    TString Name;

    SState() {}
    SState(u32 _ID, const TString& rkName) : ID(_ID), Name(rkName) {}
};

struct SMessage
{
    u32 ID;
    TString Name;

    SMessage() {}
    SMessage(u32 _ID, const TString& rkName) : ID(_ID), Name(rkName) {}
};

class CLink
{
    CGameArea *mpArea;
    u32 mStateID;
    u32 mMessageID;
    u32 mSenderID;
    u32 mReceiverID;

public:
    CLink(CGameArea *pArea)
        : mpArea(pArea)
        , mStateID(-1)
        , mMessageID(-1)
        , mSenderID(-1)
        , mReceiverID(-1)
    {}

    CLink(CGameArea *pArea, u32 StateID, u32 MessageID, u32 SenderID, u32 ReceiverID)
        : mpArea(pArea)
        , mStateID(StateID)
        , mMessageID(MessageID)
        , mSenderID(SenderID)
        , mReceiverID(ReceiverID)
    {}

    void SetSender(u32 NewSenderID, u32 Index = -1)
    {
        u32 OldSenderID = mSenderID;
        CScriptObject *pOldSender = mpArea->GetInstanceByID(OldSenderID);
        CScriptObject *pNewSender = mpArea->GetInstanceByID(NewSenderID);

        mSenderID = NewSenderID;
        pOldSender->RemoveLink(eOutgoing, this);
        pNewSender->AddLink(eOutgoing, this, Index);
    }

    void SetReceiver(u32 NewReceiverID, u32 Index = -1)
    {
        u32 OldReceiverID = mSenderID;
        CScriptObject *pOldReceiver = mpArea->GetInstanceByID(OldReceiverID);
        CScriptObject *pNewReceiver = mpArea->GetInstanceByID(NewReceiverID);

        mReceiverID = NewReceiverID;
        pOldReceiver->RemoveLink(eIncoming, this);
        pNewReceiver->AddLink(eIncoming, this, Index);
    }

    u32 SenderIndex() const
    {
        CScriptObject *pSender = mpArea->GetInstanceByID(mSenderID);

        for (u32 iLink = 0; iLink < pSender->NumLinks(eOutgoing); iLink++)
        {
            if (pSender->Link(eOutgoing, iLink) == this)
                return iLink;
        }

        return -1;
    }

    u32 ReceiverIndex() const
    {
        CScriptObject *pReceiver = mpArea->GetInstanceByID(mReceiverID);

        for (u32 iLink = 0; iLink < pReceiver->NumLinks(eIncoming); iLink++)
        {
            if (pReceiver->Link(eIncoming, iLink) == this)
                return iLink;
        }

        return -1;
    }

    // Operators
    bool operator==(const CLink& rkOther)
    {
        return ( (mpArea == rkOther.mpArea) &&
                 (mStateID == rkOther.mStateID) &&
                 (mMessageID == rkOther.mMessageID) &&
                 (mSenderID == rkOther.mSenderID) &&
                 (mReceiverID == rkOther.mReceiverID) );
    }

    bool operator!=(const CLink& rkOther)
    {
        return (!(*this == rkOther));
    }

    // Accessors
    u32 State() const               { return mStateID; }
    u32 Message() const             { return mMessageID; }
    u32 SenderID() const            { return mSenderID; }
    u32 ReceiverID() const          { return mReceiverID; }
    CScriptObject* Sender() const   { return mpArea->GetInstanceByID(mSenderID); }
    CScriptObject* Receiver() const { return mpArea->GetInstanceByID(mReceiverID); }

    void SetState(u32 StateID)      { mStateID = StateID; }
    void SetMessage(u32 MessageID)  { mMessageID = MessageID; }
};


#endif // CLINK_H
