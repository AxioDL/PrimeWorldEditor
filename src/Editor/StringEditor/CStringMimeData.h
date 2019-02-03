#ifndef CSTRINGMIMEDATA_H
#define CSTRINGMIMEDATA_H

#include <QMimeData>
#include <Common/Common.h>

/** Mime data encoding a reference to a string for drag&drop support in string editor */
class CStringMimeData : public QMimeData
{
    Q_OBJECT
    CAssetID mAssetID;
    uint mStringIndex;

public:
    CStringMimeData(CAssetID AssetID, uint StringIndex)
        : mAssetID(AssetID), mStringIndex(StringIndex)
    {}

    virtual bool hasFormat(const QString& kMimeType) const override { return true; }

    inline CAssetID AssetID() const { return mAssetID; }
    inline uint StringIndex() const { return mStringIndex; }
};

#endif // CSTRINGMIMEDATA_H
