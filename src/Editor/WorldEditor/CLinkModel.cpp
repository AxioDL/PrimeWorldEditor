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

void CLinkModel::SetConnectionType(EConnectionType type)
{
    mType = type;
    emit layoutChanged();
}

int CLinkModel::rowCount(const QModelIndex&) const
{
    if (mpObject)
    {
        if (mType == eIncoming)
            return mpObject->NumInLinks();
        else
            return mpObject->NumOutLinks();
    }

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
        SLink link = (mType == eIncoming ? mpObject->InLink(index.row()) : mpObject->OutLink(index.row()));

        switch (index.column())
        {

        case 0: // Column 0 - Target Object
        {
            CScriptObject *pTargetObj = mpObject->Area()->GetInstanceByID(link.ObjectID);

            if (pTargetObj) {
                QString ObjType = QString("[%1] ").arg(UICommon::ToQString(pTargetObj->Template()->Name()));
                return ObjType + UICommon::ToQString(pTargetObj->InstanceName());
            }
            else {
                QString strID = QString::number(link.ObjectID, 16);
                while (strID.length() < 8) strID = "0" + strID;
                return QString("External: 0x") + strID;
            }
        }

        case 1: // Column 1 - State
        {
            TString StateName = mpObject->MasterTemplate()->StateByID(link.State).Name;
            return UICommon::ToQString(StateName);
        }

        case 2: // Column 2 - Message
        {
            TString MessageName = mpObject->MasterTemplate()->MessageByID(link.Message).Name;
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
