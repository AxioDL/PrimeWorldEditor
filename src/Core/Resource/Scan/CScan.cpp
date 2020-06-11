#include "CScan.h"

CScan::CScan(CResourceEntry* pEntry /*= 0*/)
    : CResource(pEntry)
{
    CGameTemplate* pGameTemplate = NGameList::GetGameTemplate( Game() );
    mpTemplate = pGameTemplate->FindMiscTemplate("ScannableObjectInfo");
    ASSERT( mpTemplate != nullptr );

    CStructProperty* pProperties = mpTemplate->Properties();
    mPropertyData.resize( pProperties->DataSize() );
    pProperties->Construct( mPropertyData.data() );
}

CStructRef CScan::ScanData() const
{
    return CStructRef((void*) mPropertyData.data(), mpTemplate->Properties());
}

/** Convenience property accessors */
CAssetRef CScan::ScanStringPropertyRef() const
{
    const uint kStringIdMP1 = 0x1;
    const uint kStringIdMP2 = 0x2F5B6423;

    IProperty* pProperty = mpTemplate->Properties()->ChildByID(
        Game() <= EGame::Prime ? kStringIdMP1 : kStringIdMP2
    );

    return CAssetRef( (void*) mPropertyData.data(), pProperty );
}

CBoolRef CScan::IsCriticalPropertyRef() const
{
    const uint kIsCriticalIdMP1 = 0x4;
    const uint kIsCriticalIdMP2 = 0x7B714814;

    IProperty* pProperty = mpTemplate->Properties()->ChildByID(
        Game() <= EGame::Prime ? kIsCriticalIdMP1 : kIsCriticalIdMP2
    );

    return CBoolRef( (void*) mPropertyData.data(), pProperty );
}

/** CResource interface */
std::unique_ptr<CDependencyTree> CScan::BuildDependencyTree() const
{
    auto pTree = std::make_unique<CDependencyTree>();
    pTree->ParseProperties(Entry(), ScanData().Property(), ScanData().DataPointer());
    return pTree;
}
