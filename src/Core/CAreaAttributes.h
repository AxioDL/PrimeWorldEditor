#ifndef CAREAATTRIBUTES_H
#define CAREAATTRIBUTES_H

#include "Core/Resource/Script/CScriptObject.h"

class CAreaAttributes
{
    EGame mGame;
    CScriptObject *mpObj;

public:
    CAreaAttributes(CScriptObject *pObj);
    ~CAreaAttributes();
    void SetObject(CScriptObject *pObj);
    bool IsLayerEnabled() const;
    bool IsSkyEnabled() const;
    CModel* SkyModel() const;
};

#endif // CAREAATTRIBUTES_H
