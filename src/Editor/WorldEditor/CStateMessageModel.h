#ifndef CSTATEMESSAGEMODEL_H
#define CSTATEMESSAGEMODEL_H

#include "Editor/UICommon.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <Core/Resource/Script/CScriptTemplate.h>
#include <QAbstractListModel>

// todo: support pulling states/messages from script templates instead of game
class CStateMessageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class EType
    {
        States,
        Messages
    };

private:
    struct SEntry
    {
        uint32 ID = 0;
        QString Name;

        SEntry() = default;
        SEntry(uint32 _ID, const QString& rkName)
            : ID(_ID), Name(rkName) {}

        bool operator<(const SEntry& rkOther) const
        {
            return Name < rkOther.Name;
        }
    };
    QList<SEntry> mEntries;

    CGameTemplate *mpGame = nullptr;
    CScriptTemplate *mpScript = nullptr;
    EType mType;

public:
    explicit CStateMessageModel(EType Type, QObject *pParent = nullptr)
        : QAbstractListModel(pParent)
        , mType(Type)
    {}

    int rowCount(const QModelIndex& /*rkParent*/) const override
    {
        return mEntries.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const override
    {
        if (Role == Qt::DisplayRole)
        {
            return mEntries[rkIndex.row()].Name;
        }

        return QVariant::Invalid;
    }

    void SetGameTemplate(CGameTemplate *pGame)
    {
        beginResetModel();

        mpGame = pGame;
        mEntries.clear();

        if (mType == EType::States)
        {
            for (uint32 iState = 0; iState < pGame->NumStates(); iState++)
            {
                SState State = pGame->StateByIndex(iState);
                mEntries.push_back(SEntry(State.ID, TO_QSTRING(State.Name)));
            }
        }

        else
        {
            for (uint32 iMsg = 0; iMsg < pGame->NumMessages(); iMsg++)
            {
                SMessage Message = pGame->MessageByIndex(iMsg);
                mEntries.push_back(SEntry(Message.ID, TO_QSTRING(Message.Name)));
            }
        }

        std::sort(mEntries.begin(), mEntries.end());
        endResetModel();
    }

    uint32 StateIndex(uint32 StateID) const
    {
        if (mType == EType::Messages) return -1;

        for (int iState = 0; iState < mEntries.size(); iState++)
        {
            if (mEntries[iState].ID == StateID)
                return iState;
        }

        return UINT32_MAX;
    }

    uint32 MessageIndex(uint32 MessageID) const
    {
        if (mType == EType::States) return -1;

        for (int iMsg = 0; iMsg < mEntries.size(); iMsg++)
        {
            if (mEntries[iMsg].ID == MessageID)
                return iMsg;
        }

        return UINT32_MAX;
    }

    void SetScriptTemplate(CScriptTemplate *pScript)
    {
        mpScript = pScript;
    }

    uint32 State(uint32 Index) const
    {
        return (mType == EType::States ? mEntries[Index].ID : 0);
    }

    uint32 Message(uint32 Index) const
    {
        return (mType == EType::Messages ? mEntries[Index].ID : 0);
    }
};

#endif // CSTATEMESSAGEMODEL_H
