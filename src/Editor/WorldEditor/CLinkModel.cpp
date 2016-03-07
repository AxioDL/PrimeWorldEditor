#include "CLinkModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/CGameArea.h>
#include <Core/Resource/Script/CMasterTemplate.h>

CLinkModel::CLinkModel(QObject *pParent) : QAbstractTableModel(pParent)
{
    mpObject = nullptr;
    mType = eOutgoing;
}

void CLinkModel::SetObject(CScriptObject *pObj)
{
    mpObject = pObj;
    emit layoutChanged();
}

void CLinkModel::SetConnectionType(ELinkType type)
{
    mType = type;
    emit layoutChanged();
}

int CLinkModel::rowCount(const QModelIndex&) const
{
    if (mpObject)
        return mpObject->NumLinks(mType);

    else return 0;
}

int CLinkModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

QVariant CLinkModel::data(const QModelIndex &index, int role) const
{
    if (!mpObject) return QVariant::Invalid;

    else if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
    {
        CLink *pLink = mpObject->Link(mType, index.row());

        switch (index.column())
        {

        case 0: // Column 0 - Target Object
        {
            u32 TargetID = (mType == eIncoming ? pLink->SenderID() : pLink->ReceiverID());
            CScriptObject *pTarget = mpObject->Area()->GetInstanceByID(TargetID);

            if (pTarget) {
                QString ObjType = QString("[%1] ").arg(UICommon::ToQString(pTarget->Template()->Name()));
                return ObjType + UICommon::ToQString(pTarget->InstanceName());
            }
            else {
                QString strID = QString::number(TargetID, 16);
                while (strID.length() < 8) strID = "0" + strID;
                return QString("External: 0x") + strID;
            }
        }

        case 1: // Column 1 - State
        {
            TString StateName = mpObject->MasterTemplate()->StateByID(pLink->State()).Name;
            return UICommon::ToQString(StateName);
        }

        case 2: // Column 2 - Message
        {
            TString MessageName = mpObject->MasterTemplate()->MessageByID(pLink->Message()).Name;
            return UICommon::ToQString(MessageName);
        }

        default:
            return QVariant::Invalid;
        }
    }

    else return QVariant::Invalid;
}

QVariant CLinkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
        case 0: return (mType == eIncoming ? "Sender" : "Target");
        case 1: return "State";
        case 2: return "Message";
        default: return QVariant::Invalid;
        }
    }

    else return QVariant::Invalid;
}
