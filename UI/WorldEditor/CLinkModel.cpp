#include "CLinkModel.h"
#include <Resource/CGameArea.h>
#include <Resource/script/CMasterTemplate.h>

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

int CLinkModel::columnCount(const QModelIndex &parent) const
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

            if (role == Qt::DisplayRole) {
                if (pTargetObj) return QString::fromStdString(pTargetObj->GetInstanceName());
                else return QString("0x") + QString::number(link.ObjectID, 16);
            }
            else {
                QString ObjType = QString("[%1] ").arg(QString::fromStdString(pTargetObj->Template()->TemplateName()));
                if (pTargetObj) return ObjType + QString::fromStdString(pTargetObj->GetInstanceName());
                else return ObjType + QString("0x") + QString::number(link.ObjectID, 16);
            }
        }

        case 1: // Column 1 - State
        {
            std::string StateName = mpObject->MasterTemplate()->StateByID(link.State);
            return QString::fromStdString(StateName);
        }

        case 2: // Column 2 - Message
        {
            std::string MessageName = mpObject->MasterTemplate()->MessageByID(link.Message);
            return QString::fromStdString(MessageName);
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
