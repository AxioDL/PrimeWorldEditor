#ifndef CRESOURCE_H
#define CRESOURCE_H

#include "CResTypeInfo.h"
#include "EResType.h"
#include "Core/GameProject/CDependencyTree.h"
#include "Core/GameProject/CResourceEntry.h"
#include "Core/GameProject/CResourceStore.h"
#include <Common/CAssetID.h>
#include <Common/CFourCC.h>
#include <Common/TString.h>
#include <Common/Serialization/IArchive.h>

// This macro creates functions that allow us to easily identify this resource type.
// Must be included on every CResource subclass.
#define DECLARE_RESOURCE_TYPE(ResourceTypeEnum) \
public: \
    static EResourceType StaticType() \
    { \
        return EResourceType::ResourceTypeEnum; \
    } \
    \
    static CResTypeInfo* StaticTypeInfo() \
    { \
        return CResTypeInfo::FindTypeInfo(StaticType()); \
    } \
    \
private: \

class CResource
{
    DECLARE_RESOURCE_TYPE(Resource)

    CResourceEntry *mpEntry;
    int mRefCount;

public:
    CResource(CResourceEntry *pEntry = 0)
        : mpEntry(pEntry), mRefCount(0)
    {
    }

    virtual ~CResource() {}
    virtual CDependencyTree* BuildDependencyTree() const    { return new CDependencyTree(); }
    virtual void Serialize(IArchive& /*rArc*/)              {}
    virtual void InitializeNewResource()                    {}
    
    inline CResourceEntry* Entry() const    { return mpEntry; }
    inline CResTypeInfo* TypeInfo() const   { return mpEntry->TypeInfo(); }
    inline EResourceType Type() const       { return mpEntry->TypeInfo()->Type(); }
    inline TString Source() const           { return mpEntry ? mpEntry->CookedAssetPath(true).GetFileName() : ""; }
    inline TString FullSource() const       { return mpEntry ? mpEntry->CookedAssetPath(true) : ""; }
    inline CAssetID ID() const              { return mpEntry ? mpEntry->ID() : CAssetID::skInvalidID64; }
    inline EGame Game() const               { return mpEntry ? mpEntry->Game() : EGame::Invalid; }
    inline bool IsReferenced() const        { return mRefCount > 0; }
    inline void Lock()                      { mRefCount++; }
    inline void Release()                   { mRefCount--; }
};

#endif // CRESOURCE_H
