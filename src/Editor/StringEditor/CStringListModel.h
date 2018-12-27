#ifndef CSTRINGLISTMODEL_H
#define CSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <Core/Resource/TResPtr.h>
#include <Core/Resource/StringTable/CStringTable.h>

/** Model for listing available strings in a string table */
class CStringListModel : public QAbstractListModel
{
    /** Asset to pull the strings from */
    TResPtr<CStringTable> mpStringTable;

    /** Language to use for the string preview for modes that support it */
    ELanguage mStringPreviewLanguage;

public:
    CStringListModel(CStringTable* pInStrings, QObject* pParent = 0);

    /** Change the preview language display */
    void SetPreviewLanguage(ELanguage InLanguage);

    /** QAbstractListModel interface */
    virtual int rowCount(const QModelIndex& kParent) const override;
    virtual QVariant data(const QModelIndex& kIndex, int Role) const override;
};

#endif // CSTRINGLISTMODEL_H
