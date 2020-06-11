#ifndef CSCAN_H
#define CSCAN_H

#include <Common/Common.h>
#include "Core/Resource/Animation/CAnimationParameters.h"
#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/NGameList.h"

/** Scannable object parameters from SCAN assets */
class CScan : public CResource
{
    DECLARE_RESOURCE_TYPE(Scan)
    friend class CScanLoader;
    friend class CScanCooker;

    /** Script template specifying scan data layout */
    CScriptTemplate* mpTemplate;

    /** Scan property data */
    std::vector<uint8> mPropertyData;

public:
    explicit CScan(CResourceEntry* pEntry = nullptr);
    CStructRef ScanData() const;

    /** Convenience property accessors */
    CAssetRef ScanStringPropertyRef() const;
    CBoolRef IsCriticalPropertyRef() const;

    /** CResource interface */
    std::unique_ptr<CDependencyTree> BuildDependencyTree() const override;
};

#endif // CSCAN_H
