#ifndef CRESOURCEPROXYMODEL_H
#define CRESOURCEPROXYMODEL_H

#include <Common/TString.h>

#include <QSortFilterProxyModel>

class CResourceTableModel;
class CResTypeInfo;

class CResourceProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum class ESortMode
    {
        ByName, BySize
    };

    explicit CResourceProxyModel(QObject* parent = nullptr);
    ~CResourceProxyModel() override;

    void setSourceModel(QAbstractItemModel* pSourceModel) override;
    bool lessThan(const QModelIndex& rkLeft, const QModelIndex& rkRight) const override;
    bool filterAcceptsRow(int SourceRow, const QModelIndex& rkSourceParent) const override;

    void SetTypeFilter(const CResTypeInfo *pInfo, bool Allow);
    void ClearTypeFilter() { mTypeFilter.clear(); }
    bool HasTypeFilter() const { return !mTypeFilter.isEmpty(); }
    bool IsTypeAccepted(const CResTypeInfo* pTypeInfo) const;

    void SetSortMode(ESortMode Mode);

public slots:
    void SetSearchString(const TString& rkString);

private:
    CResourceTableModel *mpModel = nullptr;
    TString mSearchString;
    ESortMode mSortMode{};
    QSet<const CResTypeInfo*> mTypeFilter;

    uint64_t mCompareID = 0;
    uint64_t mCompareMask = 0;
    uint32_t mCompareBitLength = 0;
};

#endif // CRESOURCEPROXYMODEL_H

