#include "IProperty.h"
#include "CAssetProperty.h"
#include "CArrayProperty.h"
#include "CEnumProperty.h"
#include "CFlagsProperty.h"
#include "CPointerProperty.h"

#include "Core/Resource/Script/CGameTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"
#include "Core/Resource/Script/NGameList.h"
#include "Core/Resource/Script/NPropertyMap.h"

#include <algorithm>
#include <cfloat>

/** IProperty */
IProperty::IProperty(EGame Game)
    : mGame(Game)
{}

void IProperty::_ClearChildren()
{
    for (auto* child : mChildren)
    {
        // Unregister children from the name map. This has to be done before actually deleting them.
        if (child->UsesNameMap())
        {
            NPropertyMap::UnregisterProperty(child);
        }

        delete child;
    }

    mChildren.clear();
}

IProperty::~IProperty()
{
    // Remove from archetype
    if (mpArchetype != nullptr)
    {
        // If you crash here, it most likely means this property was not added to the archetype's sub-instances list.
        NBasics::VectorRemoveOne(mpArchetype->mSubInstances, this);
    }

    // If this is an archetype, make sure no sub-instances have a reference to us.
    if (IsArchetype())
    {
        for (auto* subInstance : mSubInstances)
        {
            subInstance->mpArchetype = nullptr;
        }
    }

    // Delete children
    _ClearChildren();
}

const char* IProperty::HashableTypeName() const
{
    return PropEnumToHashableTypeName( Type() );
}

void* IProperty::GetChildDataPointer(void* pPropertyData) const
{
    return pPropertyData;
}

void IProperty::Serialize(IArchive& rArc)
{
    // Always serialize ID first! ID is always required (except for root properties, which have an ID of 0xFFFFFFFF)
    // because they are needed to look up the correct property to apply parameter overrides to.
    rArc << SerialParameter("ID", mID, SH_HexDisplay | SH_Attribute | SH_Optional, 0xFFFFFFFFU);

    // Now we can serialize the archetype reference and initialize if needed
    if (((mpArchetype && mpArchetype->IsRootParent()) || rArc.IsReader()) && rArc.CanSkipParameters())
    {
        TString ArchetypeName = (mpArchetype ? mpArchetype->Name() : "");
        rArc << SerialParameter("Archetype", ArchetypeName, SH_Attribute);

        if (rArc.IsReader() && !ArchetypeName.IsEmpty())
        {
            CGameTemplate* pGame = NGameList::GetGameTemplate(Game());
            IProperty* pArchetype = pGame->FindPropertyArchetype(ArchetypeName);

            // The archetype must exist, or else the template file is malformed.
            ASSERT(pArchetype != nullptr);
            ASSERT(pArchetype->Type() == Type());

            InitFromArchetype(pArchetype);
        }
    }

    // In MP1, the game data does not use property IDs, so we serialize the name directly.
    // In MP2 and on, property names are looked up based on the property ID via the property name map.
    // Exceptions: Properties that are not in the name map still need to serialize their names.
    // This includes root-level properties, and properties of atomic structs.
    //
    // We can't currently tell if this property is atomic, as the flag hasn't been serialized and the parent
    // hasn't been set, but atomic sub-properties don't use hash IDs, so we can do a pseudo-check against the ID.
    if (rArc.Game() <= EGame::Prime || IsRootParent() || IsArrayArchetype() || mID <= 0xFF)
    {
        rArc << SerialParameter("Name", mName, mpArchetype ? SH_Optional : 0, mpArchetype ? mpArchetype->mName : "");
    }

    rArc << SerialParameter("Description",      mDescription,       SH_Optional, mpArchetype ? mpArchetype->mDescription : "")
         << SerialParameter("CookPreference",   mCookPreference,    SH_Optional, mpArchetype ? mpArchetype->mCookPreference : ECookPreference::Default)
         << SerialParameter("MinVersion",       mMinVersion,        SH_Optional, mpArchetype ? mpArchetype->mMinVersion : 0.f)
         << SerialParameter("MaxVersion",       mMaxVersion,        SH_Optional, mpArchetype ? mpArchetype->mMaxVersion : FLT_MAX)
         << SerialParameter("Suffix",           mSuffix,            SH_Optional, mpArchetype ? mpArchetype->mSuffix : "");

    // Children don't get serialized for most property types
}

void IProperty::InitFromArchetype(IProperty* pOther)
{
    //@todo maybe somehow use Serialize for this instead?
    mpArchetype = pOther;
    mpArchetype->mSubInstances.push_back(this);

    mFlags = pOther->mFlags & EPropertyFlag::ArchetypeCopyFlags;
    mName = pOther->mName;
    mDescription = pOther->mDescription;
    mSuffix = pOther->mSuffix;
    mCookPreference = pOther->mCookPreference;
    mMinVersion = pOther->mMinVersion;
    mMaxVersion = pOther->mMaxVersion;

    // Copy ID only if our existing ID is not valid.
    if (mID == 0xFFFFFFFF)
    {
        mID = pOther->mID;
    }
}

bool IProperty::ShouldSerialize() const
{
    return mpArchetype == nullptr ||
           mName != mpArchetype->mName ||
           mDescription != mpArchetype->mDescription ||
           mSuffix != mpArchetype->mSuffix ||
           mCookPreference != mpArchetype->mCookPreference ||
           mMinVersion != mpArchetype->mMinVersion ||
           mMaxVersion != mpArchetype->mMaxVersion;
}

void IProperty::Initialize(IProperty* pInParent, CScriptTemplate* pInTemplate, uint32 InOffset)
{
    // Make sure we only get initialized once.
    ASSERT( (mFlags & EPropertyFlag::IsInitialized) == 0 );

    mpParent = pInParent;
    mOffset = InOffset;
    mpScriptTemplate = pInTemplate;

    // Set any fields dependent on the parent...
    if (mpParent)
    {
        mFlags |= mpParent->mFlags & EPropertyFlag::InheritableFlags;

        if (mpParent->IsPointerType())
        {
            mpPointerParent = mpParent;
        }
        else
        {
            mpPointerParent = mpParent->mpPointerParent;
        }

        if (mpParent->Type() == EPropertyType::Array)
        {
            mFlags |= EPropertyFlag::IsArrayArchetype;
        }

        // MP1 has some weirdness we need to account for, most likely due to incorrect templates
        // The templates we have right now have non-atomic structs inside atomic structs...
        if (Game() <= EGame::Prime && mpParent->IsAtomic() && mpArchetype && !mpArchetype->IsAtomic())
        {
            mFlags.ClearFlag(EPropertyFlag::IsAtomic);
        }
    }
    else if (!mpScriptTemplate)
    {
        mFlags |= EPropertyFlag::IsArchetype;
    }

    // Register the property if needed.
    if (UsesNameMap())
    {
        NPropertyMap::RegisterProperty(this);
    }

    // Allow subclasses to handle any initialization tasks
    PostInitialize();

    // Now, route initialization to any child properties...
    uint32 ChildOffset = mOffset;

    for (size_t ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        IProperty* pChild = mChildren[ChildIdx];

        // update offset and round up to the child's alignment
        if (ChildIdx > 0)
        {
            ChildOffset += mChildren[ChildIdx - 1]->DataSize();
        }
        ChildOffset = VAL_ALIGN(ChildOffset, pChild->DataAlignment());

        // Don't call Initialize on intrinsic children as they have already been initialized.
        if (!pChild->IsIntrinsic())
        {
            pChild->Initialize(this, pInTemplate, ChildOffset);
        }
    }

    mFlags |= EPropertyFlag::IsInitialized;
}

void* IProperty::RawValuePtr(void* pData) const
{
    // For array archetypes, the caller needs to provide the pointer to the correct array item
    // Array archetypes can't store their index in the array so it's impossible to determine the correct pointer.
    void* pBasePtr = (mpPointerParent && !IsArrayArchetype() ? mpPointerParent->GetChildDataPointer(pData) : pData);
    void* pValuePtr = ((char*)pBasePtr + mOffset);
    return pValuePtr;
}

IProperty* IProperty::ChildByID(uint32 ID) const
{
    const auto iter = std::find_if(mChildren.begin(), mChildren.end(),
                                   [ID](const auto* element) { return element->mID == ID; });

    if (iter == mChildren.cend())
    {
        return nullptr;
    }

    return *iter;
}

IProperty* IProperty::ChildByIDString(const TIDString& rkIdString)
{
    // String must contain at least one ID!
    // some ID strings are formatted with 8 characters and some with 2 (plus the beginning "0x")
    ASSERT(rkIdString.Size() >= 4);

    const uint32 IDEndPos = rkIdString.IndexOf(':');
    uint32 NextChildID = UINT32_MAX;

    if (IDEndPos == UINT32_MAX)
        NextChildID = rkIdString.ToInt32(16);
    else
        NextChildID = rkIdString.SubString(2, IDEndPos - 2).ToInt32(16);

    if (NextChildID == UINT32_MAX)
    {
        return nullptr;
    }

    IProperty* pNextChild = ChildByID(NextChildID);

    // Check if we need to recurse
    if (IDEndPos != UINT32_MAX)
    {
        return pNextChild->ChildByIDString(rkIdString.ChopFront(IDEndPos + 1));
    }
    else
    {
        return pNextChild;
    }
}

void IProperty::GatherAllSubInstances(std::list<IProperty*>& OutList, bool Recursive)
{
    OutList.push_back(this);

    for (auto* subInstance : mSubInstances)
    {
        if (Recursive)
            subInstance->GatherAllSubInstances(OutList, true);
        else
            OutList.push_back(subInstance);
    }
}

TString IProperty::GetTemplateFileName()
{
    // We want to return the path to the XML file that this property originally belongs to.
    // So, for example, if this is a property of a script template, we want to return that script template.
    // However, if this property was copied from a property archetype... If we are a direct instance of an
    // archetype property (for instance a DamageInfo struct instance), then we want to return the template
    // that contains the instance. However, if we are a sub-property of an archetype, then we want to return
    // the path to that archetype instead. Hopefully that makes sense!
    IProperty* pTemplateRoot = this;

    // If our archetype has a parent, then our archetype is a sub-property of the main archetype, and we
    // need to go deeper to find the original source XML file.
    //
    // If our archetype doesn't have a parent, then we are an instance of the main archetype, and we stop here.
    while (pTemplateRoot->Archetype() && pTemplateRoot->Archetype()->Parent())
    {
        pTemplateRoot = pTemplateRoot->Archetype();
    }
    pTemplateRoot = pTemplateRoot->RootParent();

    // Now that we have the base property of our template, we can return the file path.
    static const size_t kChopAmount = strlen(*(gDataDir + "templates/"));

    if (pTemplateRoot->ScriptTemplate())
    {
        return pTemplateRoot->ScriptTemplate()->SourceFile().ChopFront(kChopAmount);
    }
    else
    {
        CGameTemplate* pGameTemplate = NGameList::GetGameTemplate(Game());
        return pGameTemplate->GetPropertyArchetypeFilePath( pTemplateRoot->Name() ).ChopFront(kChopAmount);
    }
}

bool IProperty::ShouldCook(void* pPropertyData) const
{
    ECookPreference Preference = mCookPreference;

    // Determine the real cook preference to use.
    if (Preference == ECookPreference::Default)
    {
        if (Game() == EGame::DKCReturns)
        {
            // DKCR properties usually don't write unless they have been modified.
            Preference = ECookPreference::OnlyIfModified;
        }
        else
        {
            // MP2 and MP3 properties usually always write no matter what.
            Preference = ECookPreference::Always;
        }
    }
    else if (Preference == ECookPreference::OnlyIfModified && Game() <= EGame::Prime)
    {
        // OnlyIfModified not supported for MP1.
        Preference = ECookPreference::Always;
    }

    switch (Preference)
    {
    case ECookPreference::Always:
        return true;

    case ECookPreference::Never:
        return false;

    case ECookPreference::OnlyIfModified:
        return !MatchesDefault(pPropertyData);

    default:
        // Unhandled case
        ASSERT(false);
        return true;
    }
}

void IProperty::SetName(const TString& rkNewName)
{
    if (mName == rkNewName)
        return;

    mName = rkNewName;
    mFlags.ClearFlag(EPropertyFlag::HasCachedNameCheck);
    MarkDirty();
}

void IProperty::SetDescription(const TString& rkNewDescription)
{
    if (mDescription == rkNewDescription)
        return;

    mDescription = rkNewDescription;
    MarkDirty();
}

void IProperty::SetSuffix(const TString& rkNewSuffix)
{
    if (mSuffix == rkNewSuffix)
        return;

    mSuffix = rkNewSuffix;
    MarkDirty();
}

void IProperty::MarkDirty()
{
    // Don't allow properties to be marked dirty before they are fully initialized.
    if (!IsInitialized())
        return;

    // Mark the root parent as dirty so the template file will get resaved
    RootParent()->mFlags |= EPropertyFlag::IsDirty;

    // Clear property name cache in case something has been modified that affects the hash
    mFlags &= ~(EPropertyFlag::HasCachedNameCheck | EPropertyFlag::HasCorrectPropertyName);

    // Mark sub-instances as dirty since they may need to resave as well
    for (auto& subInstance : mSubInstances)
    {
        subInstance->MarkDirty();
    }
}

void IProperty::ClearDirtyFlag()
{
    RootParent()->mFlags &= ~EPropertyFlag::IsDirty;
}

bool IProperty::ConvertType(EPropertyType NewType, IProperty* pNewArchetype)
{
    if (mpArchetype && !pNewArchetype)
    {
        // We need to start from the root archetype and cascade down sub-instances.
        // The archetype will re-call this function with a valid pNewArchetype pointer.
        return mpArchetype->ConvertType(NewType, nullptr);
    }

    IProperty* pNewProperty = Create(NewType, Game());

    // We can only replace properties with types that have the same size and alignment
    if (pNewProperty->DataSize() != DataSize() || pNewProperty->DataAlignment() != DataAlignment())
    {
        delete pNewProperty;
        return false;
    }

    // Use InitFromArchetype to copy most parameters over from the original property.
    // Note we do *not* want to call the virtual version, because the new property isn't
    // actually the same type, so the virtual overrides will likely crash.
    pNewProperty->IProperty::InitFromArchetype(this);
    pNewProperty->mpArchetype = pNewArchetype;
    NBasics::VectorRemoveOne(mSubInstances, pNewProperty);

    if (pNewArchetype)
    {
        pNewArchetype->mSubInstances.push_back(pNewProperty);
    }

    // We use CopyDefaultValueTo to ensure that the default value is preserved (as the default value
    // is important in most games, and necessary to cook correctly in DKCR). However, note that
    // other type-specific parameters (such as min/max values) are lost in the conversion.
    CopyDefaultValueTo(pNewProperty);

    // Since we are about to delete this property, we need to unregister it and all its sub-instances
    // from the name map, and change the type name. The reason we need to do it this way is because
    // after we change the type name in the map, we won't be able to unregister the original properties
    // because their type name won't match what's in the map. However, the change has to be done before
    // initializing any new properties, or else they won't be able to initialize correctly, as the
    // name won't be tracked in the map under the new type name. So we need to manually unregister
    // everything to clear the original properties from the map, then change the type name, and then
    // we're free to start creating and initializing new properties.
    if (IsRootArchetype() && mGame >= EGame::EchoesDemo)
    {
        std::list<IProperty*> SubInstances;
        GatherAllSubInstances(SubInstances, true);

        for (auto* property : SubInstances)
        {
            if (property->UsesNameMap())
            {
                NPropertyMap::UnregisterProperty(property);
            }
        }

        NPropertyMap::ChangeTypeName(this, HashableTypeName(), pNewProperty->HashableTypeName());
    }

    // Swap out our parent's reference to us to point to the new property.
    if (mpParent != nullptr)
    {
        for (auto& sibling : mpParent->mChildren)
        {
            if (sibling == this)
            {
                sibling = pNewProperty;
                break;
            }
        }
    }

    // Change all our child properties to be parented under the new property. (Is this adoption?)
    for (auto* child : mChildren)
    {
        child->mpParent = pNewProperty;
        pNewProperty->mChildren.push_back(child);
    }
    ASSERT(pNewProperty->mChildren.size() == mChildren.size());
    mChildren.clear();

    // Create new versions of all sub-instances that inherit from the new property.
    // Note that when the sub-instances complete their conversion, they delete themselves.
    // The IProperty destructor removes the property from the archetype's sub-instance list.
    // So we shouldn't use a for loop, instead we should just wait until the array is empty
    [[maybe_unused]] const size_t SubCount = mSubInstances.size();

    while (!mSubInstances.empty())
    {
        [[maybe_unused]] const bool SubSuccess = mSubInstances[0]->ConvertType(NewType, pNewProperty);
        ASSERT(SubSuccess);
    }
    ASSERT(pNewProperty->mSubInstances.size() == SubCount);

    // Conversion is complete! Initialize the new property and flag it dirty.
    pNewProperty->Initialize(mpParent, mpScriptTemplate, mOffset);
    pNewProperty->MarkDirty();

    // Finally, if we are done converting this property and all its instances, resave the templates.
    if (IsRootArchetype())
    {
        NGameList::SaveTemplates();

        if (mGame >= EGame::EchoesDemo)
        {
            NPropertyMap::SaveMap(true);
        }
    }

    // We're done!
    delete this;
    return true;
}

bool IProperty::UsesNameMap() const
{
    return Game() >= EGame::EchoesDemo &&
            !IsRootParent() &&
            !IsIntrinsic() &&
            !mpParent->IsAtomic() && // Atomic properties can use the name map, but their children shouldn't
            !IsArrayArchetype();
}

bool IProperty::HasAccurateName()
{
    // Exceptions for the three hardcoded 4CC property IDs, and for 0xFFFFFFFF (root properties)
    if (mID == FOURCC('XFRM') ||
        mID == FOURCC('INAM') ||
        mID == FOURCC('ACTV') ||
        mID == 0xFFFFFFFF)
    {
        return true;
    }

    // Children of atomic properties defer to parents. Intrinsic properties and array archetypes also defer to parents.
    if ((mpParent && mpParent->IsAtomic()) || IsIntrinsic() || IsArrayArchetype())
    {
        if (mpParent)
            return mpParent->HasAccurateName();

        return true;
    }

    // For everything else, hash the property name and check if it is a match for the property ID
    if (!mFlags.HasFlag(EPropertyFlag::HasCachedNameCheck))
    {
        CCRC32 Hash;
        Hash.Hash(*mName);
        Hash.Hash(HashableTypeName());
        uint32 GeneratedID = Hash.Digest();

        // Some choice properties are incorrectly declared as ints, so account for
        // this and allow matching ints against choice typenames as well.
        if (GeneratedID != mID && Type() == EPropertyType::Int)
        {
            Hash = CCRC32();
            Hash.Hash(*mName);
            Hash.Hash("choice");
            GeneratedID = Hash.Digest();
        }

        if (GeneratedID == mID)
        {
            mFlags.SetFlag(EPropertyFlag::HasCorrectPropertyName);
        }
        else
        {
            mFlags.ClearFlag(EPropertyFlag::HasCorrectPropertyName);
        }

        mFlags.SetFlag(EPropertyFlag::HasCachedNameCheck);
    }

    return mFlags.HasFlag(EPropertyFlag::HasCorrectPropertyName);
}

/** IProperty Accessors */
EGame IProperty::Game() const
{
    return mGame;
}

IProperty* IProperty::Create(EPropertyType Type, EGame Game)
{
    IProperty* pOut = nullptr;

    switch (Type)
    {
    case EPropertyType::Bool:            pOut = new CBoolProperty(Game);             break;
    case EPropertyType::Byte:            pOut = new CByteProperty(Game);             break;
    case EPropertyType::Short:           pOut = new CShortProperty(Game);            break;
    case EPropertyType::Int:             pOut = new CIntProperty(Game);              break;
    case EPropertyType::Float:           pOut = new CFloatProperty(Game);            break;
    case EPropertyType::Choice:          pOut = new CChoiceProperty(Game);           break;
    case EPropertyType::Enum:            pOut = new CEnumProperty(Game);             break;
    case EPropertyType::Flags:           pOut = new CFlagsProperty(Game);            break;
    case EPropertyType::String:          pOut = new CStringProperty(Game);           break;
    case EPropertyType::Vector:          pOut = new CVectorProperty(Game);           break;
    case EPropertyType::Color:           pOut = new CColorProperty(Game);            break;
    case EPropertyType::Asset:           pOut = new CAssetProperty(Game);            break;
    case EPropertyType::Sound:           pOut = new CSoundProperty(Game);            break;
    case EPropertyType::Animation:       pOut = new CAnimationProperty(Game);        break;
    case EPropertyType::AnimationSet:    pOut = new CAnimationSetProperty(Game);     break;
    case EPropertyType::Sequence:        pOut = new CSequenceProperty(Game);         break;
    case EPropertyType::Spline:          pOut = new CSplineProperty(Game);           break;
    case EPropertyType::Guid:            pOut = new CGuidProperty(Game);             break;
    case EPropertyType::Pointer:         pOut = new CPointerProperty(Game);          break;
    case EPropertyType::Struct:          pOut = new CStructProperty(Game);           break;
    case EPropertyType::Array:           pOut = new CArrayProperty(Game);            break;
    default: break;
    }

    // If this assertion fails, then there is an unhandled type!
    ASSERT(pOut != nullptr);
    return pOut;
}

IProperty* IProperty::CreateCopy(IProperty* pArchetype)
{
    IProperty* pOut = Create(pArchetype->Type(), pArchetype->mGame);
    pOut->InitFromArchetype(pArchetype);
    return pOut;
}

IProperty* IProperty::CreateIntrinsic(EPropertyType Type,
                                      EGame Game,
                                      uint32 Offset,
                                      const TString& rkName)
{
    IProperty* pOut = Create(Type, Game);
    pOut->mFlags |= EPropertyFlag::IsIntrinsic;
    pOut->SetName(rkName);
    pOut->Initialize(nullptr, nullptr, Offset);
    return pOut;
}

IProperty* IProperty::CreateIntrinsic(EPropertyType Type,
                                      IProperty* pParent,
                                      uint32 Offset,
                                      const TString& rkName)
{
    // pParent should always be valid.
    // If you are creating a root property, call the other overload takes an EGame instead of a parent.
    ASSERT(pParent != nullptr);

    IProperty* pOut = Create(Type, pParent->mGame);
    pOut->mFlags |= EPropertyFlag::IsIntrinsic;
    pOut->SetName(rkName);
    pOut->Initialize(pParent, nullptr, Offset);
    pParent->mChildren.push_back(pOut);
    return pOut;
}

IProperty* IProperty::ArchiveConstructor(EPropertyType Type,
                                         const IArchive& Arc)
{
    return Create(Type, Arc.Game());
}