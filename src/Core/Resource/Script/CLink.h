#ifndef CLINK_H
#define CLINK_H

#include "Core/NRangeUtils.h"
#include "Core/Resource/Area/CGameArea.h"
#include "Core/Resource/Script/CScriptObject.h"
#include <Common/TString.h>
#include <cstdint>

struct SState
{
    union {
        uint32_t ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SState()
    {}

    SState(uint32_t InID, TString kInName)
        : ID(InID)
        , Name(std::move(kInName))
    {}

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay);
        else
            Arc << SerialParameter("ID", ID_4CC, SH_Attribute);

        Arc << SerialParameter("Name", Name, SH_Attribute);
    }

    static SState MakeInvalid()
    {
        return {UINT32_MAX, "Invalid"};
    }
};

struct SMessage
{
    union {
        uint32_t ID;
        CFourCC ID_4CC;
    };
    TString Name;

    SMessage()
    {}

    SMessage(uint32_t InID, TString kInName)
        : ID(InID)
        , Name(std::move(kInName))
    {}

    void Serialize(IArchive& Arc)
    {
        if (Arc.Game() <= EGame::Prime)
            Arc << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay);
        else
            Arc << SerialParameter("ID", ID_4CC, SH_Attribute);

        Arc << SerialParameter("Name", Name, SH_Attribute);
    }

    static SMessage MakeInvalid()
    {
        return {UINT32_MAX, "Invalid"};
    }
};

class CLink
{
    CGameArea *mpArea;
    uint32_t mStateID = UINT32_MAX;
    uint32_t mMessageID = UINT32_MAX;
    CInstanceID mSenderID{};
    CInstanceID mReceiverID{};

public:
    explicit CLink(CGameArea *pArea)
        : mpArea(pArea)
    {}

    CLink(CGameArea *pArea, uint32_t StateID, uint32_t MessageID, CInstanceID SenderID, CInstanceID ReceiverID)
        : mpArea(pArea)
        , mStateID(StateID)
        , mMessageID(MessageID)
        , mSenderID(SenderID)
        , mReceiverID(ReceiverID)
    {}

    void SetSender(CInstanceID NewSenderID, uint32_t Index = UINT32_MAX)
    {
        const auto OldSenderID = mSenderID;
        CScriptObject *pOldSender = mpArea->InstanceByID(OldSenderID);
        CScriptObject *pNewSender = mpArea->InstanceByID(NewSenderID);

        mSenderID = NewSenderID;

        if (pOldSender)
            pOldSender->RemoveLink(ELinkType::Outgoing, this);

        pNewSender->AddLink(ELinkType::Outgoing, this, Index);
    }

    void SetReceiver(CInstanceID NewReceiverID, uint32_t Index = UINT32_MAX)
    {
        const auto OldReceiverID = mSenderID;
        CScriptObject *pOldReceiver = mpArea->InstanceByID(OldReceiverID);
        CScriptObject *pNewReceiver = mpArea->InstanceByID(NewReceiverID);

        mReceiverID = NewReceiverID;

        if (pOldReceiver)
            pOldReceiver->RemoveLink(ELinkType::Incoming, this);

        pNewReceiver->AddLink(ELinkType::Incoming, this, Index);
    }

    uint32_t SenderIndex() const
    {
        if (const CScriptObject* pSender = mpArea->InstanceByID(mSenderID))
        {
            for (auto&& [idx, link] : Utils::enumerate(pSender->Links(ELinkType::Outgoing)))
            {
                if (link == this)
                    return uint32_t(idx);
            }
        }

        return UINT32_MAX;
    }

    uint32_t ReceiverIndex() const
    {
        if (const CScriptObject* pReceiver = mpArea->InstanceByID(mReceiverID))
        {
            for (auto&& [idx, link] : Utils::enumerate(pReceiver->Links(ELinkType::Incoming)))
            {
                if (link == this)
                    return uint32_t(idx);
            }
        }

        return UINT32_MAX;
    }

    // Operators
    bool operator==(const CLink&) const = default;

    // Accessors
    CGameArea* Area() const          { return mpArea; }
    uint32_t State() const           { return mStateID; }
    uint32_t Message() const         { return mMessageID; }
    CInstanceID SenderID() const     { return mSenderID; }
    CInstanceID ReceiverID() const   { return mReceiverID; }
    CScriptObject* Sender() const    { return mpArea->InstanceByID(mSenderID); }
    CScriptObject* Receiver() const  { return mpArea->InstanceByID(mReceiverID); }

    void SetState(uint32_t StateID)       { mStateID = StateID; }
    void SetMessage(uint32_t MessageID)   { mMessageID = MessageID; }
};


#endif // CLINK_H
