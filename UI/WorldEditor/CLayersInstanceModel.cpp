#include "CLayersInstanceModel.h"

/* The tree has 3 levels:
 * 1. Node Type (Script Object, Light) - represented with ID of 0
 * 2. Layer - represented with flags
 * 3. Instance - represented with pointer to instance (0x1 bit is guaranteed to be clear)
 *
 * Flags for Layer tree items:
 * AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC
 * A: Row index
 * B: Node type row index
 * C: Item type (ObjType, Instance)
 */
#define LAYERS_ROW_INDEX_MASK  0xFFFFFFE0
#define LAYERS_NODE_TYPE_MASK  0x0000001E
#define LAYERS_ITEM_TYPE_MASK  0x00000001
#define LAYERS_ROW_INDEX_SHIFT 5
#define LAYERS_NODE_TYPE_SHIFT 1
#define LAYERS_ITEM_TYPE_SHIFT 0

CLayersInstanceModel::CLayersInstanceModel(QObject *pParent) : QAbstractItemModel(pParent)
{

}

CLayersInstanceModel::~CLayersInstanceModel()
{

}

QVariant CLayersInstanceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
        case 0: return "Name";
        case 1: return "Type";
        case 2: return "Show";
        }
    }

    return QVariant::Invalid;
}

QModelIndex CLayersInstanceModel::index(int /*row*/, int /*column*/, const QModelIndex& /*parent*/) const
{
    return QModelIndex();
}

QModelIndex CLayersInstanceModel::parent(const QModelIndex& /*child*/) const
{
    return QModelIndex();
}

int CLayersInstanceModel::rowCount(const QModelIndex& /*parent*/) const
{
    return 0;
}

int CLayersInstanceModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

QVariant CLayersInstanceModel::data(const QModelIndex& /*index*/, int /*role*/) const
{
    return QVariant::Invalid;
}

void CLayersInstanceModel::SetEditor(CWorldEditor *pEditor)
{
    mpEditor = pEditor;
    mpScene = (pEditor ? pEditor->Scene() : nullptr);
    mpArea = (pEditor ? pEditor->ActiveArea() : nullptr);
}

void CLayersInstanceModel::NodeCreated(CSceneNode* /*pNode*/)
{
    emit layoutChanged();
}

void CLayersInstanceModel::NodeDeleted(CSceneNode* /*pNode*/)
{
    emit layoutChanged();
}

CScriptLayer* CLayersInstanceModel::IndexLayer(const QModelIndex& /*index*/) const
{
    return nullptr;
}

CScriptObject* CLayersInstanceModel::IndexObject(const QModelIndex& /*index*/) const
{
    return nullptr;
}

// ************ STATIC ************
CLayersInstanceModel::EIndexType CLayersInstanceModel::IndexType(const QModelIndex& index)
{
    if (!index.isValid()) return eRootIndex;
    else if (index.internalId() == 0) return eNodeTypeIndex;
    else if (((index.internalId() & LAYERS_ITEM_TYPE_MASK) >> LAYERS_ITEM_TYPE_SHIFT) == 1) return eLayerIndex;
    else return eInstanceIndex;
}

CLayersInstanceModel::ENodeType CLayersInstanceModel::IndexNodeType(const QModelIndex& index)
{
    EIndexType type = IndexType(index);

    switch (type)
    {
    case eRootIndex:     return eInvalidType;
    case eNodeTypeIndex: return (ENodeType) index.row();
    case eLayerIndex:    return (ENodeType) index.parent().row();
    case eInstanceIndex: return (ENodeType) index.parent().parent().row();
    default:             return eInvalidType;
    }
}
