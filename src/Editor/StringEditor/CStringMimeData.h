#ifndef CSTRINGMIMEDATA_H
#define CSTRINGMIMEDATA_H

#include <QMimeData>
#include <Common/Common.h>

/** Mime data encoding a reference to a string for drag&drop support in string editor */
class CStringMimeData : public QMimeData
{
    Q_OBJECT
    CAssetID mAssetID;
    uint32 mStringIndex;

public:
    CStringMimeData(CAssetID AssetID, uint32 StringIndex)
        : mAssetID(AssetID), mStringIndex(StringIndex)
    {}

    bool hasFormat(const QString& kMimeType) const override { return true; }

    CAssetID AssetID() const { return mAssetID; }
    uint32 StringIndex() const { return mStringIndex; }
};

#endif // CSTRINGMIMEDATA_H
