#include "CLinkModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/Script/CGameTemplate.h>

CLinkModel::CLinkModel(QObject *pParent)
    : QAbstractTableModel(pParent)
{
}

void CLinkModel::SetObject(CScriptObject *pObj)
{
    mpObject = pObj;
    emit layoutChanged();
}

void CLinkModel::SetConnectionType(ELinkType Type)
{
    mType = Type;
    emit layoutChanged();
}

int CLinkModel::rowCount(const QModelIndex&) const
{
    if (mpObject)
        return static_cast<int>(mpObject->NumLinks(mType));

    return 0;
}

int CLinkModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 3;
}

QVariant CLinkModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!mpObject)
        return QVariant::Invalid;

    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        CLink *pLink = mpObject->Link(mType, rkIndex.row());

        switch (rkIndex.column())
        {
        case 0: // Column 0 - Target Object
        {
            const uint32 TargetID = (mType == ELinkType::Incoming ? pLink->SenderID() : pLink->ReceiverID());
            const CScriptObject *pTarget = mpObject->Area()->InstanceByID(TargetID);

            if (pTarget)
            {
                const QString ObjType = tr("[%1] ").arg(UICommon::ToQString(pTarget->Template()->Name()));
                return ObjType + UICommon::ToQString(pTarget->InstanceName());
            }
            else
            {
                QString StrID = QString::number(TargetID, 16).toUpper();
                while (StrID.length() < 8)
                    StrID = QLatin1Char{'0'} + StrID;
                return tr("External: %1").arg(StrID);
            }
        }

        case 1: // Column 1 - State
        {
            const TString StateName = mpObject->GameTemplate()->StateByID(pLink->State()).Name;
            return UICommon::ToQString(StateName);
        }

        case 2: // Column 2 - Message
        {
            const TString MessageName = mpObject->GameTemplate()->MessageByID(pLink->Message()).Name;
            return UICommon::ToQString(MessageName);
        }

        default:
            return QVariant::Invalid;
        }
    }

    return QVariant::Invalid;
}

QVariant CLinkModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
    {
        switch (Section)
        {
        case 0: return (mType == ELinkType::Incoming ? tr("Sender") : tr("Target"));
        case 1: return tr("State");
        case 2: return tr("Message");
        default: return QVariant::Invalid;
        }
    }

    return QVariant::Invalid;
}
