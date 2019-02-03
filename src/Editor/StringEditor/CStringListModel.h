#ifndef CSTRINGLISTMODEL_H
#define CSTRINGLISTMODEL_H

#include <QAbstractListModel>
#include <Core/Resource/TResPtr.h>
#include <Core/Resource/StringTable/CStringTable.h>

class CStringEditor;

/** Model for listing available strings in a string table */
class CStringListModel : public QAbstractListModel
{
    /** String editor that owns the model */
    CStringEditor* mpEditor;

    /** Asset to pull the strings from */
    TResPtr<CStringTable> mpStringTable;

    /** Language to use for the string preview for modes that support it */
    ELanguage mStringPreviewLanguage;

public:
    CStringListModel(CStringEditor* pInEditor);

    /** Change the preview language display */
    void SetPreviewLanguage(ELanguage InLanguage);

    /** QAbstractListModel interface */
    virtual int rowCount(const QModelIndex& kParent) const override;
    virtual QVariant data(const QModelIndex& kIndex, int Role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& kIndex) const override;

    /** Drag & Drop support */
    virtual Qt::DropActions supportedDragActions() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual QMimeData* mimeData(const QModelIndexList& kIndexes) const override;
    virtual bool canDropMimeData(const QMimeData* pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& kParent) const override;
    virtual bool dropMimeData(const QMimeData* pkData, Qt::DropAction Action, int Row, int Column, const QModelIndex& kParent) override;
};

#endif // CSTRINGLISTMODEL_H
