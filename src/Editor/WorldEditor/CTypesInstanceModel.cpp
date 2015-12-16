#include "CTypesInstanceModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Scene/CScriptNode.h>
#include <QApplication>
#include <QIcon>

/* The tree has 3 levels:
 * 1. Node Type (Script Object, Light, World Mesh, etc) - represented with ID of 0
 * 2. Object Type (Actor, Platform, SpawnPoint, etc) - represented with flags
 * 3. Instance - represented with pointer to instance (0x1 bit is guaranteed to be clear)
 *
 * Flags for Object Type tree items
 * AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC
 * A: Row index
 * B: Node type row index
 * C: Item type (ObjType, Instance)
 */
#define TYPES_ROW_INDEX_MASK  0xFFFFFFE0
#define TYPES_NODE_TYPE_MASK  0x0000001E
#define TYPES_ITEM_TYPE_MASK  0x00000001
#define TYPES_ROW_INDEX_SHIFT 5
#define TYPES_NODE_TYPE_SHIFT 1
#define TYPES_ITEM_TYPE_SHIFT 0

bool SortTemplatesAlphabetical(CScriptTemplate *pA, CScriptTemplate *pB)
{
    return (pA->TemplateName() < pB->TemplateName());
}

CTypesInstanceModel::CTypesInstanceModel(QObject *pParent) : QAbstractItemModel(pParent)
{
    mpEditor = nullptr;
    mpScene = nullptr;
    mpArea = nullptr;
    mpCurrentMaster = nullptr;
    mModelType = eLayers;
    mBaseItems << "Script";
}

CTypesInstanceModel::~CTypesInstanceModel()
{
}

QVariant CTypesInstanceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        switch (section)
        {
        case 0: return "Name";
        case 1: return (mModelType == eLayers ? "Type" : "Layer");
        case 2: return "Show";
        }
    }
    return QVariant::Invalid;
}

QModelIndex CTypesInstanceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    EIndexType type = IndexType(parent);

    // Node type index
    if (type == eRootIndex)
    {
        if (row < mBaseItems.count())
            return createIndex(row, column, quint32(0));
        else
            return QModelIndex();
    }

    // Object type index
    else if (type == eNodeTypeIndex)
        return createIndex(row, column, ((row << TYPES_ROW_INDEX_SHIFT) | (parent.row() << TYPES_NODE_TYPE_SHIFT) | 1));

    // Instance index
    else if (type == eObjectTypeIndex)
    {
        u32 RootRow = parent.parent().row();

        // Object
        if (RootRow == 0)
        {
            if (mModelType == eLayers)
            {
                CScriptLayer *pLayer = mpArea->GetScriptLayer(parent.row());
                if ((u32) row >= pLayer->GetNumObjects())
                    return QModelIndex();
                else
                    return createIndex(row, column, (*pLayer)[row]);
            }

            else if (mModelType == eTypes)
            {
                const std::list<CScriptObject*>& list = mTemplateList[parent.row()]->ObjectList();
                if ((u32) row >= list.size())
                    return QModelIndex();
                else
                {
                    auto it = std::next(list.begin(), row);
                    return createIndex(row, column, *it);
                }
            }
        }

        // todo: implement getters for other types
    }

    return QModelIndex();
}

QModelIndex CTypesInstanceModel::parent(const QModelIndex &child) const
{
    EIndexType type = IndexType(child);

    // Root parent
    if (type == eNodeTypeIndex)
        return QModelIndex();

    // Node type parent
    if (type == eObjectTypeIndex)
    {
        u32 NodeTypeRow = (child.internalId() & TYPES_NODE_TYPE_MASK) >> TYPES_NODE_TYPE_SHIFT;
        return createIndex(NodeTypeRow, 0, quint32(0));
    }

    // Object type parent
    else if (type == eInstanceIndex)
    {
        CScriptObject *pObj = static_cast<CScriptObject*>   (child.internalPointer());

        if (mModelType == eLayers)
        {
            CScriptLayer *pLayer = pObj->Layer();

            for (u32 iLyr = 0; iLyr < mpArea->GetScriptLayerCount(); iLyr++)
            {
                if (mpArea->GetScriptLayer(iLyr) == pLayer)
                    return createIndex(iLyr, 0, (iLyr << TYPES_ROW_INDEX_SHIFT) | 1);
            }
        }

        else if (mModelType == eTypes)
        {
            CScriptTemplate *pTemp = pObj->Template();

            for (int iTemp = 0; iTemp < mTemplateList.size(); iTemp++)
            {
                if (mTemplateList[iTemp] == pTemp)
                    return createIndex(iTemp, 0, (iTemp << TYPES_ROW_INDEX_SHIFT) | 1);
            }
        }
    }

    return QModelIndex();
}

int CTypesInstanceModel::rowCount(const QModelIndex &parent) const
{
    EIndexType type = IndexType(parent);

    // Node types
    if (type == eRootIndex)
        return mBaseItems.count();

    // Object types
    else if (type == eNodeTypeIndex)
    {
        // Script Objects
        if (parent.row() == 0) {
            if (mModelType == eLayers)
                return (mpArea ? mpArea->GetScriptLayerCount() : 0);
            else
                return mTemplateList.size();
        }
        else
            return 0;
    }

    // Instances
    else if (type == eObjectTypeIndex)
    {
        u32 RowIndex = ((parent.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
        if (mModelType == eLayers)
            return (mpArea ? mpArea->GetScriptLayer(RowIndex)->GetNumObjects() : 0);
        else
            return mTemplateList[RowIndex]->NumObjects();
    }

    else
        return 0;
}

int CTypesInstanceModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

QVariant CTypesInstanceModel::data(const QModelIndex &index, int role) const
{
    EIndexType type = IndexType(index);

    // Name/Layer
    if ((role == Qt::DisplayRole) || (role == Qt::ToolTipRole))
    {
        // Node types
        if (type == eNodeTypeIndex)
        {
            if (index.column() == 0)
                return mBaseItems[index.row()];

            else
                return QVariant::Invalid;
        }

        // Object types
        else if (type == eObjectTypeIndex)
        {
            if (index.column() == 0) {
                if (mModelType == eLayers)
                    return TO_QSTRING(mpEditor->ActiveArea()->GetScriptLayer(index.row())->Name());
                else
                    return TO_QSTRING(mTemplateList[index.row()]->TemplateName());
            }
            // todo: show/hide button in column 2
            else
                return QVariant::Invalid;
        }

        // Instances
        else if (type == eInstanceIndex)
        {
            // todo: show/hide button
            CScriptObject *pObj = static_cast<CScriptObject*>(index.internalPointer());

            if (index.column() == 0)
                return TO_QSTRING(pObj->InstanceName());

            else if (index.column() == 1)
            {
                if (mModelType == eLayers)
                    return TO_QSTRING(pObj->Template()->TemplateName());
                else if (mModelType == eTypes)
                    return TO_QSTRING(pObj->Layer()->Name());
            }

            else
                return QVariant::Invalid;
        }
    }

    // Show/Hide Buttons
    else if ((role == Qt::DecorationRole) && (index.column() == 2))
    {
        if (!mpScene) return QVariant::Invalid;

        static QIcon Visible(":/icons/Show.png");
        static QIcon Invisible(":/icons/Hide.png");

        // Show/Hide Node Types
        if (type == eNodeTypeIndex)
        {
            // Commented out pending a proper implementation of turning node types on/off from the instance view
            /*bool IsVisible;

            switch (index.row())
            {
            case 0: IsVisible = mpScene->AreScriptObjectsEnabled();
            case 1: IsVisible = mpScene->AreLightsEnabled();
            default: IsVisible = false;
            }

            if (IsVisible) return Visible;
            else return Invisible;*/
        }

        // Show/Hide Object Types
        else if (type == eObjectTypeIndex)
        {
            if (mModelType == eLayers)
            {
                CScriptLayer *pLayer = IndexLayer(index);
                if (pLayer->IsVisible()) return Visible;
                else return Invisible;
            }

            else if (mModelType == eTypes)
            {
                CScriptTemplate *pTemp = IndexTemplate(index);
                if (pTemp->IsVisible()) return Visible;
                else return Invisible;
            }
        }

        // Show/Hide Instance
        else if (type == eInstanceIndex)
        {
            CScriptObject *pObj = IndexObject(index);
            CScriptNode *pNode = mpScene->NodeForObject(pObj);
            if (pNode->MarkedVisible()) return Visible;
            else return Invisible;
        }
    }

    return QVariant::Invalid;
}

void CTypesInstanceModel::SetEditor(CWorldEditor *pEditor)
{
    mpEditor = pEditor;
    mpScene = (pEditor ? pEditor->Scene() : nullptr);
}

void CTypesInstanceModel::SetMaster(CMasterTemplate *pMaster)
{
    mpCurrentMaster = pMaster;
    GenerateList();
}

void CTypesInstanceModel::SetArea(CGameArea *pArea)
{
    beginResetModel();
    mpArea = pArea;
    endResetModel();
}

void CTypesInstanceModel::SetModelType(EInstanceModelType type)
{
    mModelType = type;
}

void CTypesInstanceModel::NodeCreated(CSceneNode *pNode)
{
    if (mModelType == eTypes)
    {
        if (pNode->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
            CScriptObject *pObj = pScript->Object();
            pObj->Template()->SortObjects();

            if (pObj->Template()->NumObjects() == 1)
            {
                mTemplateList << pObj->Template();
                qSort(mTemplateList.begin(), mTemplateList.end(), SortTemplatesAlphabetical);
                emit layoutChanged();
            }
        }
    }
}

void CTypesInstanceModel::NodeDeleted(CSceneNode *pNode)
{
    if (mModelType = eTypes)
    {
        if (pNode->NodeType() == eScriptNode)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
            CScriptObject *pObj = pScript->Object();

            if (pObj->Template()->NumObjects() == 0)
            {
                for (auto it = mTemplateList.begin(); it != mTemplateList.end(); it++)
                {
                    if (*it == pObj->Template())
                    {
                        mTemplateList.erase(it);
                        break;
                    }
                }

                emit layoutChanged();
            }
        }
    }
}

CScriptLayer* CTypesInstanceModel::IndexLayer(const QModelIndex& index) const
{
    if ((mModelType != eLayers) || (IndexNodeType(index) != eScriptType) || (IndexType(index) != eObjectTypeIndex))
        return nullptr;

    u32 RowIndex = ((index.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
    return mpArea->GetScriptLayer(RowIndex);
}

CScriptTemplate* CTypesInstanceModel::IndexTemplate(const QModelIndex& index) const
{
    if ((mModelType != eTypes) || (IndexNodeType(index) != eScriptType) || (IndexType(index) != eObjectTypeIndex))
        return nullptr;

    u32 RowIndex = ((index.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
    return mTemplateList[RowIndex];
}

CScriptObject* CTypesInstanceModel::IndexObject(const QModelIndex& index) const
{
    if ((IndexNodeType(index) != eScriptType) || (IndexType(index) != eInstanceIndex))
        return nullptr;

    return static_cast<CScriptObject*>(index.internalPointer());
}

// ************ STATIC ************
CTypesInstanceModel::EIndexType CTypesInstanceModel::IndexType(const QModelIndex& index)
{
    if (!index.isValid()) return eRootIndex;
    else if (index.internalId() == 0) return eNodeTypeIndex;
    else if (((index.internalId() & TYPES_ITEM_TYPE_MASK) >> TYPES_ITEM_TYPE_SHIFT) == 1) return eObjectTypeIndex;
    else return eInstanceIndex;
}

CTypesInstanceModel::ENodeType CTypesInstanceModel::IndexNodeType(const QModelIndex& index)
{
    EIndexType type = IndexType(index);

    switch (type)
    {
    case eRootIndex:       return eInvalidType;
    case eNodeTypeIndex:   return (ENodeType) index.row();
    case eObjectTypeIndex: return (ENodeType) index.parent().row();
    case eInstanceIndex:   return (ENodeType) index.parent().parent().row();
    default:               return eInvalidType;
    }
}

// ************ PRIVATE ************
void CTypesInstanceModel::GenerateList()
{
    beginResetModel();

    mTemplateList.clear();

    if (mpCurrentMaster)
    {
        u32 NumTemplates = mpCurrentMaster->NumScriptTemplates();

        for (u32 iTemp = 0; iTemp < NumTemplates; iTemp++)
        {
            CScriptTemplate *pTemp = mpCurrentMaster->TemplateByIndex(iTemp);

            if (pTemp->NumObjects() > 0)
                mTemplateList << pTemp;
        }

        qSort(mTemplateList.begin(), mTemplateList.end(), SortTemplatesAlphabetical);
    }

    endResetModel();
}
