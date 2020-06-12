#ifndef CLINK_H
#define CLINK_H

#include "CScriptObject.h"
#include "Core/Resource/Area/CGameArea.h"
#include <Common/BasicTypes.h>
#include <Common/TString.h>

struct SState
{
    union {
        uint32 ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SState()
    {}

    SState(uint32 InID, const TString& kInName)
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
        uint32 ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SMessage()
    {}

    SMessage(uint32 InID, const TString& kInName)
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
    uint32 mStateID = UINT32_MAX;
    uint32 mMessageID = UINT32_MAX;
    uint32 mSenderID = UINT32_MAX;
    uint32 mReceiverID = UINT32_MAX;

public:
    explicit CLink(CGameArea *pArea)
        : mpArea(pArea)
    {}

    CLink(CGameArea *pArea, uint32 StateID, uint32 MessageID, uint32 SenderID, uint32 ReceiverID)
        : mpArea(pArea)
        , mStateID(StateID)
        , mMessageID(MessageID)
        , mSenderID(SenderID)
        , mReceiverID(ReceiverID)
    {}

    void SetSender(uint32 NewSenderID, uint32 Index = UINT32_MAX)
    {
        const uint32 OldSenderID = mSenderID;
        CScriptObject *pOldSender = mpArea->InstanceByID(OldSenderID);
        CScriptObject *pNewSender = mpArea->InstanceByID(NewSenderID);

        mSenderID = NewSenderID;

        if (pOldSender)
            pOldSender->RemoveLink(ELinkType::Outgoing, this);

        pNewSender->AddLink(ELinkType::Outgoing, this, Index);
    }

    void SetReceiver(uint32 NewReceiverID, uint32 Index = UINT32_MAX)
    {
        const uint32 OldReceiverID = mSenderID;
        CScriptObject *pOldReceiver = mpArea->InstanceByID(OldReceiverID);
        CScriptObject *pNewReceiver = mpArea->InstanceByID(NewReceiverID);

        mReceiverID = NewReceiverID;

        if (pOldReceiver)
            pOldReceiver->RemoveLink(ELinkType::Incoming, this);

        pNewReceiver->AddLink(ELinkType::Incoming, this, Index);
    }

    uint32 SenderIndex() const
    {
        CScriptObject *pSender = mpArea->InstanceByID(mSenderID);

        if (pSender)
        {
            for (uint32 iLink = 0; iLink < pSender->NumLinks(ELinkType::Outgoing); iLink++)
            {
                if (pSender->Link(ELinkType::Outgoing, iLink) == this)
                    return iLink;
            }
        }

        return UINT32_MAX;
    }

    uint32 ReceiverIndex() const
    {
        CScriptObject *pReceiver = mpArea->InstanceByID(mReceiverID);

        if (pReceiver)
        {
            for (uint32 iLink = 0; iLink < pReceiver->NumLinks(ELinkType::Incoming); iLink++)
            {
                if (pReceiver->Link(ELinkType::Incoming, iLink) == this)
                    return iLink;
            }
        }

        return UINT32_MAX;
    }

    // Operators
    bool operator==(const CLink& rkOther) const
    {
        return (mpArea == rkOther.mpArea) &&
               (mStateID == rkOther.mStateID) &&
               (mMessageID == rkOther.mMessageID) &&
               (mSenderID == rkOther.mSenderID) &&
               (mReceiverID == rkOther.mReceiverID);
    }

    bool operator!=(const CLink& rkOther) const
    {
        return (!(*this == rkOther));
    }

    // Accessors
    CGameArea* Area() const          { return mpArea; }
    uint32 State() const             { return mStateID; }
    uint32 Message() const           { return mMessageID; }
    uint32 SenderID() const          { return mSenderID; }
    uint32 ReceiverID() const        { return mReceiverID; }
    CScriptObject* Sender() const    { return mpArea->InstanceByID(mSenderID); }
    CScriptObject* Receiver() const  { return mpArea->InstanceByID(mReceiverID); }

    void SetState(uint32 StateID)       { mStateID = StateID; }
    void SetMessage(uint32 MessageID)   { mMessageID = MessageID; }
};


#endif // CLINK_H
