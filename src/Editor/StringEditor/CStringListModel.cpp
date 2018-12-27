#include "CStringListModel.h"
#include "Editor/UICommon.h"

CStringListModel::CStringListModel(CStringTable* pInStrings, QObject* pParent /*= 0*/)
    : QAbstractListModel(pParent)
    , mpStringTable(pInStrings)
    , mStringPreviewLanguage(ELanguage::English)
{
}

/** Change the preview language display */
void CStringListModel::SetPreviewLanguage(ELanguage InLanguage)
{
    if (mStringPreviewLanguage != InLanguage)
    {
        mStringPreviewLanguage = InLanguage;

        // Emit data changed for user role for the full range of strings
        int NumStrings = mpStringTable ? mpStringTable->NumStrings() : 0;

        if (NumStrings)
        {
            QVector<int> Roles;
            Roles << Qt::UserRole;
            emit dataChanged( index(0), index(NumStrings-1), Roles );
        }
    }
}

/** QAbstractListModel interface */
int CStringListModel::rowCount(const QModelIndex& kParent) const
{
    return mpStringTable ? mpStringTable->NumStrings() : 0;
}

QVariant CStringListModel::data(const QModelIndex& kIndex, int Role) const
{
    if (!kIndex.isValid() || !mpStringTable)
    {
        return QVariant::Invalid;
    }

    int StringIndex = kIndex.row();

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
