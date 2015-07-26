#ifndef CTOKEN_H
#define CTOKEN_H

#include <Resource/CResource.h>

class CToken
{
    CResource *mpRes;
    bool mLocked;

public:
    CToken();
    CToken(CResource *pRes);
    CToken(const CToken& Source);
    ~CToken();
    void Lock();
    void Unlock();
    CToken& operator=(const CToken& Source);
};

#endif // CTOKEN_H
