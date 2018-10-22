#include "CLinkModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Area/CGameArea.h>
#include <Core/Resource/Script/CGameTemplate.h>

CLinkModel::CLinkModel(QObject *pParent)
    : QAbstractTableModel(pParent)
    , mpObject(nullptr)
    , mType(eOutgoing)
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
        return mpObject->NumLinks(mType);

    else return 0;
}

int CLinkModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return 3;
}

QVariant CLinkModel::data(const QModelIndex& rkIndex, int Role) const
{
    if (!mpObject) return QVariant::Invalid;

    else if ((Role == Qt::DisplayRole) || (Role == Qt::ToolTipRole))
    {
        CLink *pLink = mpObject->Link(mType, rkIndex.row());

        switch (rkIndex.column())
        {

        case 0: // Column 0 - Target Object
        {
            u32 TargetID = (mType == eIncoming ? pLink->SenderID() : pLink->ReceiverID());
            CScriptObject *pTarget = mpObject->Area()->InstanceByID(TargetID);

            if (pTarget)
            {
                QString ObjType = QString("[%1] ").arg(UICommon::ToQString(pTarget->Template()->Name()));
                return ObjType + UICommon::ToQString(pTarget->InstanceName());
            }

            else
            {
                QString StrID = QString::number(TargetID, 16).toUpper();
                while (StrID.length() < 8) StrID = "0" + StrID;
                return QString("External: ") + StrID;
            }
        }

        case 1: // Column 1 - State
        {
            TString StateName = mpObject->GameTemplate()->StateByID(pLink->State()).Name;
            return UICommon::ToQString(StateName);
        }

        case 2: // Column 2 - Message
        {
            TString MessageName = mpObject->GameTemplate()->MessageByID(pLink->Message()).Name;
            return UICommon::ToQString(MessageName);
        }

        default:
            return QVariant::Invalid;
        }
    }

    else return QVariant::Invalid;
}

QVariant CLinkModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if ((Orientation == Qt::Horizontal) && (Role == Qt::DisplayRole))
    {
        switch (Section)
        {
        case 0: return (mType == eIncoming ? "Sender" : "Target");
        case 1: return "State";
        case 2: return "Message";
        default: return QVariant::Invalid;
        }
    }

    else return QVariant::Invalid;
}
