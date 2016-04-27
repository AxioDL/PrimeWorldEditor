#ifndef CSKINLOADER_H
#define CSKINLOADER_H

#include "Core/Resource/CSkin.h"
#include "Core/Resource/TResPtr.h"

class CSkinLoader
{
    CSkinLoader() {}
public:
    static CSkin* LoadCSKR(IInputStream& rCSKR);
};

#endif // CSKINLOADER_H
