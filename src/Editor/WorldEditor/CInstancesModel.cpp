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
{
    mBaseItems.push_back(QStringLiteral("Script"));

    connect(gpEdApp, &CEditorApplication::ActiveProjectChanged, this, &CInstancesModel::OnActiveProjectChanged);
    connect(mpEditor, &CWorldEditor::MapChanged, this, &CInstancesModel::OnMapChange);
    connect(mpEditor, &CWorldEditor::NodeAboutToBeSpawned, this, &CInstancesModel::NodeAboutToBeCreated);
    connect(mpEditor, &CWorldEditor::NodeSpawned, this, &CInstancesModel::NodeCreated);
    connect(mpEditor, &CWorldEditor::NodeAboutToBeDeleted, this, &CInstancesModel::NodeAboutToBeDeleted);
    connect(mpEditor, &CWorldEditor::NodeDeleted, this, &CInstancesModel::NodeDeleted);
    connect(mpEditor, &CWorldEditor::PropertyModified, this, &CInstancesModel::PropertyModified);
    connect(mpEditor, &CWorldEditor::InstancesLayerAboutToChange, this, &CInstancesModel::InstancesLayerPreChange);
    connect(mpEditor, &CWorldEditor::InstancesLayerChanged, this, &CInstancesModel::InstancesLayerPostChange);
}

CInstancesModel::~CInstancesModel() = default;

QVariant CInstancesModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if ((Orientation == Qt::Horizontal) && (Role == Qt::DisplayRole))
    {
        switch (Section)
        {
        case 0: return tr("Name");
        case 1: return (mModelType == EInstanceModelType::Layers ? tr("Type") : tr("Layer"));
        case 2: return tr("Show");
        }
    }
    return QVariant::Invalid;
}

QModelIndex CInstancesModel::index(int Row, int Column, const QModelIndex& rkParent) const
{
    if (!hasIndex(Row, Column, rkParent))
        return QModelIndex();

    const EIndexType Type = IndexType(rkParent);

    // Parent is root - child is Node type index
    if (Type == EIndexType::Root)
    {
        if (Row < mBaseItems.count())
            return createIndex(Row, Column, quint64(0));
        
        return QModelIndex();
    }

    // Parent is node - child is Object type index
    if (Type == EIndexType::NodeType)
        return createIndex(Row, Column, ((Row << TYPES_ROW_INDEX_SHIFT) | (rkParent.row() << TYPES_NODE_TYPE_SHIFT) | 1));

    // Parent is object - child is Instance index
    if (Type == EIndexType::ObjectType)
    {
        const uint32 RootRow = rkParent.parent().row();

        // Object
        if (RootRow == 0)
        {
            if (mModelType == EInstanceModelType::Layers)
            {
                CScriptLayer *pLayer = mpArea->ScriptLayer(static_cast<size_t>(rkParent.row()));
                if (static_cast<size_t>(Row) >= pLayer->NumInstances())
                    return QModelIndex();

                return createIndex(Row, Column, (*pLayer)[Row]);
            }

            if (mModelType == EInstanceModelType::Types)
            {
                const std::list<CScriptObject*>& list = mTemplateList[rkParent.row()]->ObjectList();
                if (static_cast<size_t>(Row) >= list.size())
                {
                    return QModelIndex();
                }
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
    const EIndexType Type = IndexType(rkChild);

    // Root parent
    if (Type == EIndexType::NodeType)
        return QModelIndex();

    // Node type parent
    if (Type == EIndexType::ObjectType)
    {
        const uint32 NodeTypeRow = (rkChild.internalId() & TYPES_NODE_TYPE_MASK) >> TYPES_NODE_TYPE_SHIFT;
        return createIndex(NodeTypeRow, 0, quint64(0));
    }

    // Object type parent
    if (Type == EIndexType::Instance)
    {
        CScriptObject *pObj = static_cast<CScriptObject*>   (rkChild.internalPointer());

        if (mModelType == EInstanceModelType::Layers)
        {
            CScriptLayer *pLayer = pObj->Layer();

            for (size_t iLyr = 0; iLyr < mpArea->NumScriptLayers(); iLyr++)
            {
                if (mpArea->ScriptLayer(iLyr) == pLayer)
                    return createIndex(iLyr, 0, (iLyr << TYPES_ROW_INDEX_SHIFT) | 1);
            }
        }
        else if (mModelType == EInstanceModelType::Types)
        {
            CScriptTemplate *pTemp = pObj->Template();

            for (size_t iTemp = 0; iTemp < mTemplateList.size(); iTemp++)
            {
                if (mTemplateList[iTemp] == pTemp)
                    return createIndex(static_cast<int>(iTemp), 0, static_cast<quintptr>((iTemp << TYPES_ROW_INDEX_SHIFT) | 1));
            }
        }
    }

    return QModelIndex();
}

int CInstancesModel::rowCount(const QModelIndex& rkParent) const
{
    const EIndexType Type = IndexType(rkParent);

    // Node types
    if (Type == EIndexType::Root)
    {
        return mBaseItems.count();
    }

    // Object types
    if (Type == EIndexType::NodeType)
    {
        // Script Objects
        if (rkParent.row() == 0)
        {
            if (mModelType == EInstanceModelType::Layers)
                return mpArea ? static_cast<int>(mpArea->NumScriptLayers()) : 0;

            return static_cast<int>(mTemplateList.size());
        }

        return 0;
    }

    // Instances
    if (Type == EIndexType::ObjectType)
    {
        const uint32 RowIndex = ((rkParent.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
        if (mModelType == EInstanceModelType::Layers)
            return mpArea ? static_cast<int>(mpArea->ScriptLayer(RowIndex)->NumInstances()) : 0;

        return static_cast<int>(mTemplateList[RowIndex]->NumObjects());
    }

    return 0;
}

int CInstancesModel::columnCount(const QModelIndex& /*rkParent*/) const
{
    return (mShowColumnEnabled ? 3 : 2);
}

QVariant CInstancesModel::data(const QModelIndex& rkIndex, int Role) const
{
    const EIndexType Type = IndexType(rkIndex);

    // Name/Layer
    if ((Role == Qt::DisplayRole) || (Role == Qt::ToolTipRole))
    {
        // Node types
        if (Type == EIndexType::NodeType)
        {
            if (rkIndex.column() == 0)
                return mBaseItems[rkIndex.row()];

            return QVariant::Invalid;
        }

        // Object types
        if (Type == EIndexType::ObjectType)
        {
            if (rkIndex.column() == 0)
            {
                if (mModelType == EInstanceModelType::Layers)
                    return TO_QSTRING(mpEditor->ActiveArea()->ScriptLayer(static_cast<size_t>(rkIndex.row()))->Name());
                else
                    return TO_QSTRING(mTemplateList[rkIndex.row()]->Name());
            }

            // todo: show/hide button in column 2
            return QVariant::Invalid;
        }

        // Instances
        if (Type == EIndexType::Instance)
        {
            // todo: show/hide button
            const CScriptObject *pObj = static_cast<CScriptObject*>(rkIndex.internalPointer());

            if (rkIndex.column() == 0)
                return TO_QSTRING(pObj->InstanceName());

            if (rkIndex.column() == 1)
            {
                if (mModelType == EInstanceModelType::Layers)
                    return TO_QSTRING(pObj->Template()->Name());
                if (mModelType == EInstanceModelType::Types)
                    return TO_QSTRING(pObj->Layer()->Name());
            }
            else
            {
                return QVariant::Invalid;
            }
        }
    }

    // Show/Hide Buttons
    else if ((Role == Qt::DecorationRole) && (rkIndex.column() == 2))
    {
        if (!mpScene)
            return QVariant::Invalid;

        static const QIcon Visible(QStringLiteral(":/icons/Show.svg"));
        static const QIcon Invisible(QStringLiteral(":/icons/Hide.svg"));

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
                const CScriptLayer *pLayer = IndexLayer(rkIndex);
                if (pLayer->IsVisible())
                    return Visible;
                return Invisible;
            }

            if (mModelType == EInstanceModelType::Types)
            {
                const CScriptTemplate *pTemp = IndexTemplate(rkIndex);
                if (pTemp->IsVisible())
                    return Visible;
                return Invisible;
            }
        }
        // Show/Hide Instance
        else if (Type == EIndexType::Instance)
        {
            CScriptObject *pObj = IndexObject(rkIndex);
            const CScriptNode *pNode = mpScene->NodeForInstance(pObj);
            if (pNode->MarkedVisible())
                return Visible;
            return Invisible;
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

    const uint32 RowIndex = ((rkIndex.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
    return mpArea->ScriptLayer(RowIndex);
}

CScriptTemplate* CInstancesModel::IndexTemplate(const QModelIndex& rkIndex) const
{
    if ((mModelType != EInstanceModelType::Types) || (IndexNodeType(rkIndex) != ENodeType::Script) || (IndexType(rkIndex) != EIndexType::ObjectType))
        return nullptr;

    const uint32 RowIndex = ((rkIndex.internalId() & TYPES_ROW_INDEX_MASK) >> TYPES_ROW_INDEX_SHIFT);
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
    if (mModelType != EInstanceModelType::Types)
        return;

    if (pProj != nullptr)
        mpCurrentGame = NGameList::GetGameTemplate( pProj->Game() );
    else
        mpCurrentGame = nullptr;

    GenerateList();
}

void CInstancesModel::OnMapChange()
{
    if (mModelType == EInstanceModelType::Types)
    {
        GenerateList();
    }
    else
    {
        beginResetModel();
        mpArea = mpEditor->ActiveArea();
        endResetModel();
    }
}

void CInstancesModel::NodeAboutToBeCreated()
{
    if (mChangingLayout)
        return;

    emit layoutAboutToBeChanged();
    mChangingLayout = true;
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
                const QModelIndex ScriptRootIdx = index(0, 0, QModelIndex());
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
    if (pNode->NodeType() != ENodeType::Script)
        return;

    if (mModelType == EInstanceModelType::Types)
    {
        const auto *pScript = static_cast<CScriptNode*>(pNode);
        const CScriptObject *pObj = pScript->Instance();

        if (pObj->Template()->NumObjects() <= 1)
        {
            const QModelIndex ScriptRootIdx = index(0, 0, QModelIndex());
            const int TempIdx = mTemplateList.indexOf(pObj->Template());
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

void CInstancesModel::NodeDeleted()
{
    if (!mChangingLayout)
        return;

    emit layoutChanged();
    mChangingLayout = false;
}

void CInstancesModel::PropertyModified(IProperty *pProp, CScriptObject *pInst)
{
    if (pProp->Name() != "Name")
        return;

    const QModelIndex ScriptRoot = index(0, 0, QModelIndex());

    if (mModelType == EInstanceModelType::Layers)
    {
        const uint32 Index = pInst->Layer()->AreaIndex();
        const QModelIndex LayerIndex = index(Index, 0, ScriptRoot);
        const QModelIndex InstIndex = index(pInst->LayerIndex(), 0, LayerIndex);
        emit dataChanged(InstIndex, InstIndex);
    }
    else
    {
        const uint32 Index = mTemplateList.indexOf(pInst->Template());
        const QModelIndex TempIndex = index(Index, 0, ScriptRoot);

        const QList<CScriptObject*> InstList = QList<CScriptObject*>::fromStdList(pInst->Template()->ObjectList());
        const uint32 InstIdx = InstList.indexOf(pInst);
        const QModelIndex InstIndex = index(InstIdx, 0, TempIndex);
        emit dataChanged(InstIndex, InstIndex);
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
    InstanceList.reserve(rkInstanceList.size());
    for (CScriptNode *pNode : rkInstanceList)
        InstanceList.push_back(pNode->Instance());

    const QModelIndex ScriptIdx = index(0, 0, QModelIndex());

    // For types, just find the instances that have changed layers and emit dataChanged for column 1.
    if (mModelType == EInstanceModelType::Types)
    {
        for (int iType = 0; iType < rowCount(ScriptIdx); iType++)
        {
            const QModelIndex TypeIdx = index(iType, 0, ScriptIdx);

            for (int iInst = 0; iInst < rowCount(TypeIdx); iInst++)
            {
                const QModelIndex InstIdx = index(iInst, 1, TypeIdx);
                CScriptObject *pInst = IndexObject(InstIdx);

                if (InstanceList.contains(pInst))
                    emit dataChanged(InstIdx, InstIdx);
            }
        }
    }
    else // For layers we just need to emit layoutChanged() and done
    {
        emit layoutChanged();
    }
}

// ************ STATIC ************
CInstancesModel::EIndexType CInstancesModel::IndexType(const QModelIndex& rkIndex)
{
    if (!rkIndex.isValid())
        return EIndexType::Root;

    if (rkIndex.internalId() == 0)
        return EIndexType::NodeType;

    if (((rkIndex.internalId() & TYPES_ITEM_TYPE_MASK) >> TYPES_ITEM_TYPE_SHIFT) == 1)
        return EIndexType::ObjectType;

    return EIndexType::Instance;
}

ENodeType CInstancesModel::IndexNodeType(const QModelIndex& rkIndex)
{
    const EIndexType type = IndexType(rkIndex);
    const std::array kTypes{ENodeType::Script, ENodeType::Light};

    switch (type)
    {
    case EIndexType::Root:       return ENodeType::None;
    case EIndexType::NodeType:   return kTypes[rkIndex.row()];
    case EIndexType::ObjectType: return kTypes[rkIndex.parent().row()];
    case EIndexType::Instance:   return kTypes[rkIndex.parent().parent().row()];
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
        const uint32 NumTemplates = mpCurrentGame->NumScriptTemplates();

        for (uint32 iTemp = 0; iTemp < NumTemplates; iTemp++)
        {
            CScriptTemplate *pTemp = mpCurrentGame->TemplateByIndex(iTemp);

            if (pTemp->NumObjects() > 0)
                mTemplateList.push_back(pTemp);
        }

        std::sort(mTemplateList.begin(), mTemplateList.end(), [](const CScriptTemplate *pLeft, const CScriptTemplate *pRight) {
            return pLeft->Name() < pRight->Name();
        });
    }

    endResetModel();
}
