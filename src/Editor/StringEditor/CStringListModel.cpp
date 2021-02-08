#include "CStringListModel.h"
#include "CStringEditor.h"
#include "CStringMimeData.h"
#include "Editor/UICommon.h"

#include <QMimeData>

CStringListModel::CStringListModel(CStringEditor* pInEditor)
    : QAbstractListModel(pInEditor)
    , mpEditor(pInEditor)
    , mpStringTable(pInEditor->StringTable())
{
}

/** Change the preview language display */
void CStringListModel::SetPreviewLanguage(ELanguage InLanguage)
{
    if (mStringPreviewLanguage == InLanguage)
        return;

    mStringPreviewLanguage = InLanguage;

    // Emit data changed for user role for the full range of strings
    const int NumStrings = mpStringTable ? static_cast<int>(mpStringTable->NumStrings()) : 0;
    if (NumStrings == 0)
        return;

    const QVector<int> Roles{Qt::UserRole};
    emit dataChanged(index(0), index(NumStrings - 1), Roles);
}

/** QAbstractListModel interface */
int CStringListModel::rowCount(const QModelIndex& kParent) const
{
    return mpStringTable ? static_cast<int>(mpStringTable->NumStrings()) : 0;
}

QVariant CStringListModel::data(const QModelIndex& kIndex, int Role) const
{
    if (!kIndex.isValid() || !mpStringTable)
    {
        return QVariant::Invalid;
    }

    const auto StringIndex = static_cast<size_t>(kIndex.row());

    // display/tooltip role: return the string name
    if (Role == Qt::DisplayRole || Role == Qt::ToolTipRole)
    {
        TString StringName = mpStringTable->StringNameByIndex(StringIndex);

        if (StringName.IsEmpty())
        {
            StringName = TString::Format("<String #%d>", kIndex.row()+1);
        }

        return TO_QSTRING(StringName);
    }
    // user role: used for string preview, return the string contents
    else if (Role == Qt::UserRole)
    {
        TString StringData = mpStringTable->GetString(mStringPreviewLanguage, StringIndex);
        StringData.Replace("\n", "     ");
        return TO_QSTRING(StringData);
    }
    // other roles: invalid
    else
    {
        return QVariant::Invalid;
    }
}

Qt::ItemFlags CStringListModel::flags(const QModelIndex& kIndex) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

/** Drag & Drop support */
Qt::DropActions CStringListModel::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions CStringListModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* CStringListModel::mimeData(const QModelIndexList& kIndexes) const
{
    // We don't support drag&drop on multiple strings at once
    ASSERT( kIndexes.size() == 1 );
    QModelIndex Index = kIndexes.front();
    return new CStringMimeData(mpStringTable->ID(), Index.row());
}

bool CStringListModel::canDropMimeData(const QMimeData* pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& kParent) const
{
    // Only allow dropping string mime data that originated from our string table
    const CStringMimeData* pkStringMimeData = qobject_cast<const CStringMimeData*>(pkData);
    return Action == Qt::MoveAction && pkStringMimeData != nullptr && pkStringMimeData->AssetID() == mpStringTable->ID();
}

bool CStringListModel::dropMimeData(const QMimeData* pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& kParent)
{
    debugf("Dropped onto row %d column %d", Row, Column);

    if (Action == Qt::MoveAction)
    {
        const CStringMimeData* pkStringMimeData = qobject_cast<const CStringMimeData*>(pkData);

        if (pkStringMimeData && pkStringMimeData->AssetID() == mpStringTable->ID())
        {
            // Determine new row index. If the string was dropped in between items, we can place
            // it in between those two items. Otherwise, if it was dropped on top of an item, we
            // want to bump it in place of that item (so use the item's index).
            if (Row == -1)
            {
                Row = kParent.row();

                // In some cases Row can still be -1 at this point if the parent is invalid
                // It seems like this can happen when trying to place at the top of the list
                // So to account for it, in this case, reset Row back to 0
                if (Row == -1)
                {
                    Row = 0;
                }
            }
            // If the user placed the string at the end of the list, then the index we receive
            // will be out of range, so cap it to a valid index.
            else if (Row >= static_cast<int>(mpStringTable->NumStrings()))
            {
                Row = static_cast<int>(mpStringTable->NumStrings()) - 1;
            }
            // If the string is being moved further down the list, then account for the fact that
            // the rest of the strings below it will be bumped up.
            else if (Row > static_cast<int>(pkStringMimeData->StringIndex()))
            {
                Row--;
            }

            mpEditor->OnMoveString(pkStringMimeData->StringIndex(), Row);
            return true;
        }
    }
    return false;
}
