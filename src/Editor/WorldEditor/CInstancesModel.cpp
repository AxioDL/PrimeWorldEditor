#include "CInstancesModel.h"
#include "Editor/UICommon.h"
#include <Core/Resource/Script/CScriptLayer.h>
#include <Core/Resource/Script/NGameList.h>
#include <Core/Scene/CScriptNode.h>
#include <QApplication>
#include <QIcon>

/*
 * The tree has 3 levels:
 * 1. Node Type (Script Object, Light, World Mesh, etc) - represented with ID of 0
 * 2. Object Type (Actor, Platform, SpawnPoint, etc) - represented with flags
 * 3. Instance - represented with pointer to instance (0x1 bit is guaranteed to be clear)
 *
 * Flags for Object Type tree items
 * AAAAAAAAAAAAAAAAAAAAAAAAAAABBBBC
 * A: Row index
 * B: Node type row index
 * C: Item type (ObjType, Instance)
 *
 * Also this class is kind of a mess, it would be nice to rewrite it at some point
 */
#define TYPES_ROW_INDEX_MASK  0xFFFFFFE0
#define TYPES_NODE_TYPE_MASK  0x0000001E
#define TYPES_ITEM_TYPE_MASK  0x00000001
#define TYPES_ROW_INDEX_SHIFT 5
#define TYPES_NODE_TYPE_SHIFT 1
#define TYPES_ITEM_TYPE_SHIFT 0

CInstancesModel::CInstancesModel(CWorldEditor *pEditor, QObject *pParent)
    : QAbstractItemModel(pParent)
    , mpEditor(pEditor)
    , mpScene(pEditor->Scene())
    , mpArea(nullptr)
    , mpCurrentGame(nullptr)
    , mModelType(EInstanceModelType::Layers)
    , mShowColumnEnabled(true)
    , mChangingLayout(false)
{
    mBaseItems << "Script";

    connect(gpEdApp, SIGNAL(ActiveProjectChanged(CGameProject*)), this, SLOT(OnActiveProjectChanged(CGameProject*)));
    connect(mpEditor, SIGNAL(MapChanged(CWorld*,CGameArea*)), this, SLOT(OnMapChange()));
    connect(mpEditor, SIGNAL(NodeAboutToBeSpawned()), this, SLOT(NodeAboutToBeCreated()));
    connect(mpEditor, SIGNAL(NodeSpawned(CSceneNode*)), this, SLOT(NodeCreated(CSceneNode*)));
    connect(mpEditor, SIGNAL(NodeAboutToBeDeleted(CSceneNode*)), this, SLOT(NodeAboutToBeDeleted(CSceneNode*)));
    connect(mpEditor, SIGNAL(NodeDeleted()), this, SLOT(NodeDeleted()));
    connect(mpEditor, SIGNAL(PropertyModified(IProperty*,CScriptObject*)), this, SLOT(PropertyModified(IProperty*,CScriptObject*)));
    connect(mpEditor, SIGNAL(InstancesLayerAboutToChange()), this, SLOT(InstancesLayerPreChange()));
    connect(mpEditor, SIGNAL(InstancesLayerChanged(QList<CScriptNode*>)), this, SLOT(InstancesLayerPostChange(QList<CScriptNode*>)));
}

CInstancesModel::~CInstancesModel()
{
}

QVariant CInstancesModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if ((Orientation == Qt::Horizontal) && (Role == Qt::DisplayRole))
    {
        switch (Section)
        {
        case 0: return "Name";
        case 1: return (mModelType == EInstanceModelType::Layers ? "Type" : "Layer");
        case 2: return "Show";
        }
    }
    return QVariant::Invalid;
}

QModelIndex CInstancesModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    EIndexType Type = IndexType(rkParent);

    // Parent is root - child is Node type index
    if (Type == EIndexType::Root)
    {
        if (Row < mBaseItems.count())
            return createIndex(Row, Column, quint64(0));
        else
            return QModelIndex();
    }

    // Parent is node - child is Object type index
    else if (Type == EIndexType::NodeType)
        return createIndex(Row, Column, ((Row << TYPES_ROW_INDEX_SHIFT) | (rkParent.row() << TYPES_NODE_TYPE_SHIFT) | 1));

    // Parent is object - child is Instance index
    else if (Type == EIndexType::ObjectType)
    {
        uint32 RootRow = rkParent.parent().row();

        // Object
        if (RootRow == 0)
        {
            if (mModelType == EInstanceModelType::Layers)
            {
                CScriptLayer *pLayer = mpArea->ScriptLayer(rkParent.row());
                if ((uint32) Row >= pLayer->NumInstances())
                    return QModelIndex();
                else
                    return createIndex(Row, Column, (*pLayer)[Row]);
            }

            else if (mModelType == EInstanceModelType::Types)
            {
                const std::list<CScriptObject*>& list = mTemplateList[rkParent.row()]->ObjectList();
                if ((uint32) Row >= list.size())
                    return QModelIndex();
                else
                {
                    auto it = std::next(list.begin(), Row);
                    return createIndex(Row, Column, *it);
                }
            }
        }

        // todo: implement getters for other types
    }

    return QModelIndex();
}

QModelIndex CInstancesModel::parent(const QModelIndex& rkChild) const
{
    EIndexType Type = IndexType(rkChild);

    // Root parent
    if (Type == EIndexType::NodeType)
        return QModelIndex();

    // Node type parent
    if (Type == EIndexType::ObjectType)
    {
        uint32 NodeTypeRow = (rkChild.internalId() & TYPES_NODE_TYPE_MASK) >> TYPES_NODE_TYPE_SHIFT;
        return createIndex(NodeTypeRow, 0, quint64(0));
    }

    // Object type parent
    else if (Type == EIndexType::Instance)
    {
        CScriptObject *pObj = static_cast<CScriptObject*>   (rkChild.internalPointer());

        if (mModelType == EInstanceModelType::Layers)
        {
            CScriptLayer *pLayer = pObj->Layer();

            for (uint32 iLyr = 0; iLyr < mpArea->NumScriptLayers(); iLyr++)
            {
                if (mpArea->ScriptLayer(iLyr) == pLayer)
                    return createIndex(iLyr, 0, (iLyr << TYPES_ROW_INDEX_SHIFT) | 1);
            }
        }

        else if (mModelType == EInstanceModelType::Types)
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

int CInstancesModel::rowCount(const QModelIndex& rkParent) const
{
    EIndexType Type = IndexType(rkParent);

    // Node types
    if (Type == EIndexType::Root)
        return mBaseItems.count();

    // Object types
    else if (Type == EIndexType::NodeType)
    {
        // Script Objects
        if (rkParent.row() == 0)
        {
            if (mModelType == EInstanceModelType::Layers)
                return (mpArea ? mpArea->NumScriptLayers() : 0);
            else
                return mTemplateList.size();
        }
        else
            return 0;
    }

    // Instances
    else if (Type == EIndexType::ObjectType)
    {
        uint32 RowIndex = ((rkParent.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
        if (mModelType == EInstanceModelType::Layers)
            return (mpArea ? mpArea->ScriptLayer(RowIndex)->NumInstances() : 0);
        else
            return mTemplateList[RowIndex]->NumObjects();
    }

    else
        return 0;
}

int CInstancesModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return (mShowColumnEnabled ? 3 : 2);
}

QVariant CInstancesModel::data(const QModelIndex& rkIndex, int Role) const
{
    EIndexType Type = IndexType(rkIndex);

    // Name/Layer
    if ((Role == Qt::DisplayRole) || (Role == Qt::ToolTipRole))
    {
        // Node types
        if (Type == EIndexType::NodeType)
        {
            if (rkIndex.column() == 0)
                return mBaseItems[rkIndex.row()];

            else
                return QVariant::Invalid;
        }

        // Object types
        else if (Type == EIndexType::ObjectType)
        {
            if (rkIndex.column() == 0)
            {
                if (mModelType == EInstanceModelType::Layers)
                    return TO_QSTRING(mpEditor->ActiveArea()->ScriptLayer(rkIndex.row())->Name());
                else
                    return TO_QSTRING(mTemplateList[rkIndex.row()]->Name());
            }
            // todo: show/hide button in column 2
            else
                return QVariant::Invalid;
        }

        // Instances
        else if (Type == EIndexType::Instance)
        {
            // todo: show/hide button
            CScriptObject *pObj = static_cast<CScriptObject*>(rkIndex.internalPointer());

            if (rkIndex.column() == 0)
                return TO_QSTRING(pObj->InstanceName());

            else if (rkIndex.column() == 1)
            {
                if (mModelType == EInstanceModelType::Layers)
                    return TO_QSTRING(pObj->Template()->Name());
                else if (mModelType == EInstanceModelType::Types)
                    return TO_QSTRING(pObj->Layer()->Name());
            }

            else
                return QVariant::Invalid;
        }
    }

    // Show/Hide Buttons
    else if ((Role == Qt::DecorationRole) && (rkIndex.column() == 2))
    {
        if (!mpScene) return QVariant::Invalid;

        static QIcon Visible(":/icons/Show.png");
        static QIcon Invisible(":/icons/Hide.png");

        // Show/Hide Node Types
        if (Type == EIndexType::NodeType)
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
        else if (Type == EIndexType::ObjectType)
        {
            if (mModelType == EInstanceModelType::Layers)
            {
                CScriptLayer *pLayer = IndexLayer(rkIndex);
                if (pLayer->IsVisible()) return Visible;
                else return Invisible;
            }

            else if (mModelType == EInstanceModelType::Types)
            {
                CScriptTemplate *pTemp = IndexTemplate(rkIndex);
                if (pTemp->IsVisible()) return Visible;
                else return Invisible;
            }
        }

        // Show/Hide Instance
        else if (Type == EIndexType::Instance)
        {
            CScriptObject *pObj = IndexObject(rkIndex);
            CScriptNode *pNode = mpScene->NodeForInstance(pObj);
            if (pNode->MarkedVisible()) return Visible;
            else return Invisible;
        }
    }

    return QVariant::Invalid;
}

void CInstancesModel::SetModelType(EInstanceModelType Type)
{
    mModelType = Type;
}

void CInstancesModel::SetShowColumnEnabled(bool Enabled)
{
    mShowColumnEnabled = Enabled;
    emit layoutChanged();
}

CScriptLayer* CInstancesModel::IndexLayer(const QModelIndex& rkIndex) const
{
    if ((mModelType != EInstanceModelType::Layers) || (IndexNodeType(rkIndex) != ENodeType::Script) || (IndexType(rkIndex) != EIndexType::ObjectType))
        return nullptr;

    uint32 RowIndex = ((rkIndex.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
    return mpArea->ScriptLayer(RowIndex);
}

CScriptTemplate* CInstancesModel::IndexTemplate(const QModelIndex& rkIndex) const
{
    if ((mModelType != EInstanceModelType::Types) || (IndexNodeType(rkIndex) != ENodeType::Script) || (IndexType(rkIndex) != EIndexType::ObjectType))
        return nullptr;

    uint32 RowIndex = ((rkIndex.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
    return mTemplateList[RowIndex];
}

CScriptObject* CInstancesModel::IndexObject(const QModelIndex& rkIndex) const
{
    if ((IndexNodeType(rkIndex) != ENodeType::Script) || (IndexType(rkIndex) != EIndexType::Instance))
        return nullptr;

    return static_cast<CScriptObject*>(rkIndex.internalPointer());
}

// ************ PUBLIC SLOTS ************
void CInstancesModel::OnActiveProjectChanged(CGameProject *pProj)
{
    if (mModelType == EInstanceModelType::Types)
    {
        if (pProj)
            mpCurrentGame = NGameList::GetGameTemplate( pProj->Game() );
        else
            mpCurrentGame = nullptr;

        GenerateList();
    }
}

void CInstancesModel::OnMapChange()
{
    if (mModelType == EInstanceModelType::Types)
        GenerateList();

    else
    {
        beginResetModel();
        mpArea = mpEditor->ActiveArea();
        endResetModel();
    }
}

void CInstancesModel::NodeAboutToBeCreated()
{
    if (!mChangingLayout)
    {
        emit layoutAboutToBeChanged();
        mChangingLayout = true;
    }
}

void CInstancesModel::NodeCreated(CSceneNode *pNode)
{
    if (mChangingLayout)
    {
        emit layoutChanged();
        mChangingLayout = false;
    }

    if (mModelType == EInstanceModelType::Types)
    {
        if (pNode->NodeType() == ENodeType::Script)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
            CScriptObject *pObj = pScript->Instance();
            pObj->Template()->SortObjects();

            if (pObj->Template()->NumObjects() == 1)
            {
                QModelIndex ScriptRootIdx = index(0, 0, QModelIndex());
                int NewIndex = 0;

                for (; NewIndex < mTemplateList.size(); NewIndex++)
                {
                    if (mTemplateList[NewIndex]->Name() > pObj->Template()->Name())
                        break;
                }

                beginInsertRows(ScriptRootIdx, NewIndex, NewIndex);
                mTemplateList.insert(NewIndex, pObj->Template());
                endInsertRows();
            }
        }
    }
}

void CInstancesModel::NodeAboutToBeDeleted(CSceneNode *pNode)
{
    if (pNode->NodeType() == ENodeType::Script)
    {
        if (mModelType == EInstanceModelType::Types)
        {
            CScriptNode *pScript = static_cast<CScriptNode*>(pNode);
            CScriptObject *pObj = pScript->Instance();

            if (pObj->Template()->NumObjects() <= 1)
            {
                QModelIndex ScriptRootIdx = index(0, 0, QModelIndex());
                int TempIdx = mTemplateList.indexOf(pObj->Template());
                beginRemoveRows(ScriptRootIdx, TempIdx, TempIdx);
                mTemplateList.removeOne(pObj->Template());
                endRemoveRows();
            }

            else if (!mChangingLayout)
            {
                emit layoutAboutToBeChanged();
                mChangingLayout = true;
            }
        }

        else if (!mChangingLayout)
        {
            emit layoutAboutToBeChanged();
            mChangingLayout = true;
        }
    }
}

void CInstancesModel::NodeDeleted()
{
    if (mChangingLayout)
    {
        emit layoutChanged();
        mChangingLayout = false;
    }
}

void CInstancesModel::PropertyModified(IProperty *pProp, CScriptObject *pInst)
{
    if (pProp->Name() == "Name")
    {
        QModelIndex ScriptRoot = index(0, 0, QModelIndex());

        if (mModelType == EInstanceModelType::Layers)
        {
            uint32 Index = pInst->Layer()->AreaIndex();
            QModelIndex LayerIndex = index(Index, 0, ScriptRoot);
            QModelIndex InstIndex = index(pInst->LayerIndex(), 0, LayerIndex);
            emit dataChanged(InstIndex, InstIndex);
        }

        else
        {
            uint32 Index = mTemplateList.indexOf(pInst->Template());
            QModelIndex TempIndex = index(Index, 0, ScriptRoot);

            QList<CScriptObject*> InstList = QList<CScriptObject*>::fromStdList(pInst->Template()->ObjectList());
            uint32 InstIdx = InstList.indexOf(pInst);
            QModelIndex InstIndex = index(InstIdx, 0, TempIndex);
            emit dataChanged(InstIndex, InstIndex);
        }
    }
}

void CInstancesModel::InstancesLayerPreChange()
{
    // This is only really needed on layers, which have rows moved.
    // Types just need to update column 1 so we can handle that when the change is finished.
    if (mModelType == EInstanceModelType::Layers)
        emit layoutAboutToBeChanged();
}

void CInstancesModel::InstancesLayerPostChange(const QList<CScriptNode*>& rkInstanceList)
{
    QList<CScriptObject*> InstanceList;
    foreach (CScriptNode *pNode, rkInstanceList)
        InstanceList << pNode->Instance();

    QModelIndex ScriptIdx = index(0, 0, QModelIndex());

    // For types, just find the instances that have changed layers and emit dataChanged for column 1.
    if (mModelType == EInstanceModelType::Types)
    {
        for (int iType = 0; iType < rowCount(ScriptIdx); iType++)
        {
            QModelIndex TypeIdx = index(iType, 0, ScriptIdx);

            for (int iInst = 0; iInst < rowCount(TypeIdx); iInst++)
            {
                QModelIndex InstIdx = index(iInst, 1, TypeIdx);
                CScriptObject *pInst = IndexObject(InstIdx);

                if (InstanceList.contains(pInst))
                    emit dataChanged(InstIdx, InstIdx);
            }
        }
    }

    // For layers we just need to emit layoutChanged() and done
    else
        emit layoutChanged();
}

// ************ STATIC ************
CInstancesModel::EIndexType CInstancesModel::IndexType(const QModelIndex& rkIndex)
{
    if (!rkIndex.isValid()) return EIndexType::Root;
    else if (rkIndex.internalId() == 0) return EIndexType::NodeType;
    else if (((rkIndex.internalId() & TYPES_ITEM_TYPE_MASK) >> TYPES_ITEM_TYPE_SHIFT) == 1) return EIndexType::ObjectType;
    else return EIndexType::Instance;
}

ENodeType CInstancesModel::IndexNodeType(const QModelIndex& rkIndex)
{
    EIndexType type = IndexType(rkIndex);
    const ENodeType kTypes[] = { ENodeType::Script, ENodeType::Light };

    switch (type)
    {
    case EIndexType::Root:       return ENodeType::None;
    case EIndexType::NodeType:   return (ENodeType) kTypes[ rkIndex.row() ];
    case EIndexType::ObjectType: return (ENodeType) kTypes[ rkIndex.parent().row() ];
    case EIndexType::Instance:   return (ENodeType) kTypes[ rkIndex.parent().parent().row() ];
    default:                     return ENodeType::None;
    }
}

// ************ PRIVATE ************
void CInstancesModel::GenerateList()
{
    beginResetModel();

    mTemplateList.clear();

    if (mpCurrentGame)
    {
        uint32 NumTemplates = mpCurrentGame->NumScriptTemplates();

        for (uint32 iTemp = 0; iTemp < NumTemplates; iTemp++)
        {
            CScriptTemplate *pTemp = mpCurrentGame->TemplateByIndex(iTemp);

            if (pTemp->NumObjects() > 0)
                mTemplateList << pTemp;
        }

        qSort(mTemplateList.begin(), mTemplateList.end(), [](CScriptTemplate *pLeft, CScriptTemplate *pRight) -> bool {
            return (pLeft->Name() < pRight->Name());
        });
    }

    endResetModel();
}
