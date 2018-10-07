#ifndef CLINK_H
#define CLINK_H

#include "CScriptObject.h"
#include "Core/Resource/Area/CGameArea.h"
#include <Common/TString.h>
#include <Common/types.h>

struct SState
{
    union {
        u32 ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SState()
    {}

    SState(u32 InID, const TString& kInName)
        : ID(InID)
        , Name(kInName)
    {}

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay);
        else
            Arc << SerialParameter("ID", ID_4CC, SH_Attribute);

        Arc << SerialParameter("Name", Name, SH_Attribute);
    }
};

struct SMessage
{
    union {
        u32 ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SMessage()
    {}

    SMessage(u32 InID, const TString& kInName)
        : ID(InID)
        , Name(kInName)
    {}

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay);
        else
            Arc << SerialParameter("ID", ID_4CC, SH_Attribute);

        Arc << SerialParameter("Name", Name, SH_Attribute);
    }
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
        CScriptObject *pOldSender = mpArea->InstanceByID(OldSenderID);
        CScriptObject *pNewSender = mpArea->InstanceByID(NewSenderID);

        mSenderID = NewSenderID;
        if (pOldSender) pOldSender->RemoveLink(eOutgoing, this);
        pNewSender->AddLink(eOutgoing, this, Index);
    }

    void SetReceiver(u32 NewReceiverID, u32 Index = -1)
    {
        u32 OldReceiverID = mSenderID;
        CScriptObject *pOldReceiver = mpArea->InstanceByID(OldReceiverID);
        CScriptObject *pNewReceiver = mpArea->InstanceByID(NewReceiverID);

        mReceiverID = NewReceiverID;
        if (pOldReceiver) pOldReceiver->RemoveLink(eIncoming, this);
        pNewReceiver->AddLink(eIncoming, this, Index);
    }

    u32 SenderIndex() const
    {
        CScriptObject *pSender = mpArea->InstanceByID(mSenderID);

        if (pSender)
        {
            for (u32 iLink = 0; iLink < pSender->NumLinks(eOutgoing); iLink++)
            {
                if (pSender->Link(eOutgoing, iLink) == this)
                    return iLink;
            }
        }

        return -1;
    }

    u32 ReceiverIndex() const
    {
        CScriptObject *pReceiver = mpArea->InstanceByID(mReceiverID);

        if (pReceiver)
        {
            for (u32 iLink = 0; iLink < pReceiver->NumLinks(eIncoming); iLink++)
            {
                if (pReceiver->Link(eIncoming, iLink) == this)
                    return iLink;
            }
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
    inline CGameArea* Area() const          { return mpArea; }
    inline u32 State() const                { return mStateID; }
    inline u32 Message() const              { return mMessageID; }
    inline u32 SenderID() const             { return mSenderID; }
    inline u32 ReceiverID() const           { return mReceiverID; }
    inline CScriptObject* Sender() const    { return mpArea->InstanceByID(mSenderID); }
    inline CScriptObject* Receiver() const  { return mpArea->InstanceByID(mReceiverID); }

    inline void SetState(u32 StateID)       { mStateID = StateID; }
    inline void SetMessage(u32 MessageID)   { mMessageID = MessageID; }
};


#endif // CLINK_H
