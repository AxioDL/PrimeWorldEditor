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
#include <memory>

// This macro creates functions that allow us to easily identify this resource type.
// Must be included on every CResource subclass.
#define DECLARE_RESOURCE_TYPE(ResourceTypeEnum) \
public: \
    static constexpr EResourceType StaticType() \
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
    int mRefCount = 0;

public:
    explicit CResource(CResourceEntry *pEntry = nullptr)
        : mpEntry(pEntry)
    {
    }

    virtual ~CResource() {}
    virtual std::unique_ptr<CDependencyTree> BuildDependencyTree() const { return std::make_unique<CDependencyTree>(); }
    virtual void Serialize(IArchive& /*rArc*/) {}
    virtual void InitializeNewResource()       {}

    CResourceEntry* Entry() const    { return mpEntry; }
    CResTypeInfo* TypeInfo() const   { return mpEntry->TypeInfo(); }
    EResourceType Type() const       { return mpEntry->TypeInfo()->Type(); }
    TString Source() const           { return mpEntry ? mpEntry->CookedAssetPath(true).GetFileName() : ""; }
    TString FullSource() const       { return mpEntry ? mpEntry->CookedAssetPath(true) : ""; }
    CAssetID ID() const              { return mpEntry ? mpEntry->ID() : CAssetID::skInvalidID64; }
    EGame Game() const               { return mpEntry ? mpEntry->Game() : EGame::Invalid; }
    bool IsReferenced() const        { return mRefCount > 0; }
    void Lock()                      { mRefCount++; }
    void Release()                   { mRefCount--; }
};

#endif // CRESOURCE_H
