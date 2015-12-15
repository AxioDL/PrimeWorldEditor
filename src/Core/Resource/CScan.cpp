#include "CScan.h"

CScan::CScan()
{
    mpFrame = nullptr;
    mpStringTable = nullptr;
    mIsSlow = false;
    mIsImportant = false;
    mCategory = eNone;
}

CScan::~CScan()
{
}

EGame CScan::Version()
{
    return mVersion;
}

CStringTable* CScan::ScanText()
{
    return mpStringTable;
}

bool CScan::IsImportant()
{
    return mIsImportant;
}

bool CScan::IsSlow()
{
    return mIsSlow;
}

CScan::ELogbookCategory CScan::LogbookCategory()
{
    return mCategory;
}
