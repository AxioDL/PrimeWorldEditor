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

EResType CScan::Type()
{
    return eScan;
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
