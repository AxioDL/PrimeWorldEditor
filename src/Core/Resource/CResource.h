#ifndef CRESOURCE_H
#define CRESOURCE_H

#include "EResType.h"
#include "Core/GameProject/CDependencyTree.h"
#include "Core/GameProject/CResourceEntry.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/types.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>

// This macro creates functions that allow us to easily identify this resource type.
// Must be included on every CResource subclass.
#define DECLARE_RESOURCE_TYPE(ResTypeEnum) \
public: \
    virtual EResType Type() const \
    { \
        return ResTypeEnum; \
    } \
    \
    static EResType StaticType() \
    { \
        return ResTypeEnum; \
    } \
    \
private: \

class CResource
{
    DECLARE_RESOURCE_TYPE(eResource)

    CResourceEntry *mpEntry;
    int mRefCount;

public:
    CResource(CResourceEntry *pEntry = 0)
        : mpEntry(pEntry), mRefCount(0)
    {
    }

    virtual ~CResource() {}
    virtual CDependencyTree* BuildDependencyTree() const    { return new CDependencyTree(ID()); }
    virtual void Serialize(IArchive& /*rArc*/)              {}
    
    inline CResourceEntry* Entry() const    { return mpEntry; }
    inline TString Source() const           { return mpEntry ? mpEntry->CookedAssetPath(true).GetFileName() : ""; }
    inline TString FullSource() const       { return mpEntry ? mpEntry->CookedAssetPath(true) : ""; }
    inline CAssetID ID() const              { return mpEntry ? mpEntry->ID() : CAssetID::skInvalidID64; }
    inline EGame Game() const               { return mpEntry ? mpEntry->Game() : eUnknownGame; }
    inline bool IsReferenced() const        { return mRefCount > 0; }
    inline void SetGame(EGame Game)         { if (mpEntry) mpEntry->SetGame(Game); }
    inline void Lock()                      { mRefCount++; }
    inline void Release()                   { mRefCount--; }

    static EResType ResTypeForExtension(CFourCC Extension);
};

// Global Functions
bool ResourceSupportsSerialization(EResType Type);
TString GetResourceTypeName(EResType Type);
TString GetResourceSerialName(EResType Type);
TString GetResourceRawExtension(EResType Type, EGame Game);
TString GetResourceCookedExtension(EResType Type, EGame Game);

#endif // CRESOURCE_H
