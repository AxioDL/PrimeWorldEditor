#ifndef CRESOURCE_H
#define CRESOURCE_H

#include "Core/GameProject/CDependencyTree.h"
#include "Core/GameProject/CResourceEntry.h"
#include "Core/Resource/CResTypeInfo.h"
#include "Core/Resource/EResType.h"
#include <Common/CAssetID.h>
#include <Common/EGame.h>
#include <Common/TString.h>
#include <memory>

class IArchive;

// This macro creates functions that allow us to easily identify this resource type.
// Must be included on every CResource subclass.
#define DECLARE_RESOURCE_TYPE(ResourceTypeEnum) \
public: \
    [[nodiscard]] static constexpr EResourceType StaticType() \
    { \
        return EResourceType::ResourceTypeEnum; \
    } \
    \
    [[nodiscard]] static CResTypeInfo* StaticTypeInfo() \
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
    virtual ~CResource() = default;

    [[nodiscard]] virtual std::unique_ptr<CDependencyTree> BuildDependencyTree() { return std::make_unique<CDependencyTree>(); }
    virtual void Serialize(IArchive& /*rArc*/) {}
    virtual void InitializeNewResource()       {}

    [[nodiscard]] CResourceEntry* Entry() const  { return mpEntry; }
    [[nodiscard]] CResTypeInfo* TypeInfo() const { return mpEntry->TypeInfo(); }
    [[nodiscard]] EResourceType Type() const     { return mpEntry->TypeInfo()->Type(); }
    [[nodiscard]] TString Source() const         { return mpEntry ? mpEntry->CookedAssetPath(true).GetFileName() : ""; }
    [[nodiscard]] TString FullSource() const     { return mpEntry ? mpEntry->CookedAssetPath(true) : ""; }
    [[nodiscard]] const CAssetID& ID() const     { return mpEntry ? mpEntry->ID() : CAssetID::skInvalidID64; }
    [[nodiscard]] EGame Game() const             { return mpEntry ? mpEntry->Game() : EGame::Invalid; }
    [[nodiscard]] bool IsReferenced() const      { return mRefCount > 0; }

    void Lock()    { mRefCount++; }
    void Release() { mRefCount--; }
};

#endif // CRESOURCE_H
