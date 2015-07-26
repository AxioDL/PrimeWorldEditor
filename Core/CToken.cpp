#include "CToken.h"

CToken::CToken()
{
    mpRes = nullptr;
    mLocked = false;
}

CToken::CToken(CResource *pRes)
{
    mpRes = pRes;
    mLocked = false;
    Lock();
}

CToken::CToken(const CToken& Source)
{
    mLocked = false;
    *this = Source;
}

CToken::~CToken()
{
    Unlock();
}

void CToken::Lock()
{
    if (!mLocked)
    {
        if (mpRes)
        {
            mpRes->Lock();
            mLocked = true;
        }
    }
}

void CToken::Unlock()
{
    if (mLocked)
    {
        mpRes->Release();
        mLocked = false;
    }
}

CToken& CToken::operator=(const CToken& Source)
{
    if (mLocked) Unlock();

    mpRes = Source.mpRes;
    Lock();
    return *this;
}
