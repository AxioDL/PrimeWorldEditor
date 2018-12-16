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
        uint32 ID;
        QString Name;

        SEntry() {}
        SEntry(uint32 _ID, const QString& rkName)
            : ID(_ID), Name(rkName) {}

        bool operator<(const SEntry& rkOther) const
        {
            return Name < rkOther.Name;
        }
    };
    QList<SEntry> mEntries;

    CGameTemplate *mpGame;
    CScriptTemplate *mpScript;
    EType mType;

public:
    explicit CStateMessageModel(EType Type, QObject *pParent = 0)
        : QAbstractListModel(pParent)
        , mType(Type)
        , mpGame(nullptr)
        , mpScript(nullptr)
    {}

    int rowCount(const QModelIndex& /*rkParent*/) const
    {
        return mEntries.size();
    }

    QVariant data(const QModelIndex& rkIndex, int Role) const
    {
        if (Role == Qt::DisplayRole)
        {
            return mEntries[rkIndex.row()].Name;
        }

        else return QVariant::Invalid;
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
                mEntries << SEntry(State.ID, TO_QSTRING(State.Name));
            }
        }

        else
        {
            for (uint32 iMsg = 0; iMsg < pGame->NumMessages(); iMsg++)
            {
                SMessage Message = pGame->MessageByIndex(iMsg);
                mEntries << SEntry(Message.ID, TO_QSTRING(Message.Name));
            }
        }

        qSort(mEntries);
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

        return -1;
    }

    uint32 MessageIndex(uint32 MessageID) const
    {
        if (mType == EType::States) return -1;

        for (int iMsg = 0; iMsg < mEntries.size(); iMsg++)
        {
            if (mEntries[iMsg].ID == MessageID)
                return iMsg;
        }

        return -1;
    }

    inline void SetScriptTemplate(CScriptTemplate *pScript)
    {
        mpScript = pScript;
    }

    inline uint32 State(uint32 Index) const
    {
        return (mType == EType::States ? mEntries[Index].ID : 0);
    }

    inline uint32 Message(uint32 Index) const
    {
        return (mType == EType::Messages ? mEntries[Index].ID : 0);
    }
};

#endif // CSTATEMESSAGEMODEL_H
