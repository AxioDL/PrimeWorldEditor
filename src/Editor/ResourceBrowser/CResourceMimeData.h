#ifndef CRESOURCEMIMEDATA_H
#define CRESOURCEMIMEDATA_H

#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CVirtualDirectory.h>
#include <QMimeData>

class CResourceMimeData : public QMimeData
{
    Q_OBJECT
    QList<CResourceEntry*> mEntries;
    QList<CVirtualDirectory*> mDirectories;

public:
    CResourceMimeData(const QList<CResourceEntry*>& rkEntries, const QList<CVirtualDirectory*>& rkDirectories)
        : QMimeData()
        , mEntries(rkEntries)
        , mDirectories(rkDirectories)
    {}

    CResourceMimeData(CResourceEntry *pEntry)
        : QMimeData()
    {
        mEntries << pEntry;
    }

    CResourceMimeData(CVirtualDirectory *pDir)
        : QMimeData()
    {
        mDirectories << pDir;
    }

    const QList<CResourceEntry*>& Resources() const         { return mEntries; }
    const QList<CVirtualDirectory*>& Directories() const    { return mDirectories; }
};

#endif // CRESOURCEMIMEDATA_H
