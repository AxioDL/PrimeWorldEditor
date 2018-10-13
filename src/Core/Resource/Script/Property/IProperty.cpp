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

/** IProperty */
IProperty::IProperty(EGame Game)
    : mpParent( nullptr )
    , mpPointerParent( nullptr )
    , mpArchetype( nullptr )
    , mGame( Game )
    , mpScriptTemplate( nullptr )
    , mOffset( -1 )
    , mID( -1 )
    , mCookPreference( ECookPreference::Default )
    , mMinVersion( 0.0f )
    , mMaxVersion( FLT_MAX )
{}

void IProperty::_ClearChildren()
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        // Unregister children from the name map. This has to be done before actually deleting them.
        if (mChildren[ChildIdx]->UsesNameMap())
        {
            NPropertyMap::UnregisterProperty(mChildren[ChildIdx]);
        }

        delete mChildren[ChildIdx];
    }

    mChildren.clear();
}

IProperty::~IProperty()
{
    // Remove from archetype
    if( mpArchetype != nullptr )
    {
        // If you crash here, it most likely means this property was not added to the archetype's sub-instances list.
        NBasics::VectorRemoveOne(mpArchetype->mSubInstances, this);
    }

    // If this is an archetype, make sure no sub-instances have a reference to us.
    if( IsArchetype() )
    {
        for( int SubIdx = 0; SubIdx < mSubInstances.size(); SubIdx++ )
        {
            mSubInstances[SubIdx]->mpArchetype = nullptr;
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
    rArc << SerialParameter("ID", mID, SH_HexDisplay | SH_Attribute | SH_Optional, (u32) 0xFFFFFFFF);

    // Now we can serialize the archetype reference and initialize if needed
    if ( ((mpArchetype && mpArchetype->IsRootParent()) || rArc.IsReader()) && rArc.CanSkipParameters() )
    {
        TString ArchetypeName = (mpArchetype ? mpArchetype->Name() : "");
        rArc << SerialParameter("Archetype", ArchetypeName, SH_Attribute);

        if (rArc.IsReader() && !ArchetypeName.IsEmpty())
        {
            CGameTemplate* pGame = NGameList::GetGameTemplate( Game() );
            IProperty* pArchetype = pGame->FindPropertyArchetype(ArchetypeName);

            // The archetype must exist, or else the template file is malformed.
            ASSERT(pArchetype != nullptr);

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

void IProperty::Initialize(IProperty* pInParent, CScriptTemplate* pInTemplate, u32 InOffset)
{
    // Make sure we only get initialized once.
    ASSERT( (mFlags & EPropertyFlag::IsInitialized) == 0 );
    mFlags |= EPropertyFlag::IsInitialized;

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
    u32 ChildOffset = mOffset;

    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        IProperty* pChild = mChildren[ChildIdx];

        // update offset and round up to the child's alignment
        if (ChildIdx > 0)
        {
            ChildOffset += mChildren[ChildIdx-1]->DataSize();
        }
        ChildOffset = ALIGN(ChildOffset, pChild->DataAlignment());

        // Don't call Initialize on intrinsic children as they have already been initialized.
        if (!pChild->IsIntrinsic())
        {
            pChild->Initialize(this, pInTemplate, ChildOffset);
        }
    }
}

void* IProperty::RawValuePtr(void* pData) const
{
    // For array archetypes, the caller needs to provide the pointer to the correct array item
    // Array archetypes can't store their index in the array so it's impossible to determine the correct pointer.
    void* pBasePtr = (mpPointerParent && !IsArrayArchetype() ? mpPointerParent->GetChildDataPointer(pData) : pData);
    void* pValuePtr = ((char*)pBasePtr + mOffset);
    return pValuePtr;
}

IProperty* IProperty::ChildByID(u32 ID) const
{
    for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (mChildren[ChildIdx]->mID == ID)
            return mChildren[ChildIdx];
    }

    return nullptr;
}

IProperty* IProperty::ChildByIDString(const TIDString& rkIdString)
{
    // String must contain at least one ID!
    // some ID strings are formatted with 8 characters and some with 2 (plus the beginning "0x")
    ASSERT(rkIdString.Size() >= 4);

    u32 IDEndPos = rkIdString.IndexOf(':');
    u32 NextChildID = -1;

    if (IDEndPos == -1)
        NextChildID = rkIdString.ToInt32();
    else
        NextChildID = rkIdString.SubString(2, IDEndPos - 2).ToInt32();

    if (NextChildID == 0xFFFFFFFF)
    {
        return nullptr;
    }

    IProperty* pNextChild = ChildByID(NextChildID);

    // Check if we need to recurse
    if (IDEndPos != -1)
    {
        return pNextChild->ChildByIDString(rkIdString.ChopFront(IDEndPos + 1));
    }
    else
    {
        return pNextChild;
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
    static const u32 kChopAmount = strlen("../templates/");

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
    switch (mCookPreference)
    {
    case ECookPreference::Always:
        return true;

    case ECookPreference::Never:
        return false;

    default:
        return (Game() < EGame::DKCReturns ? true : !MatchesDefault(pPropertyData));
    }
}

void IProperty::SetName(const TString& rkNewName)
{
    if (mName != rkNewName)
    {
        mName = rkNewName;
        mFlags.ClearFlag(EPropertyFlag::HasCachedNameCheck);

        // in Echoes and on, since property names are referenced by ID, renaming a property
        // doesn't directly affect the serialized data, so it doesn't need to be flagged dirty
        if (mGame <= EGame::Prime)
        {
            MarkDirty();
        }
    }
}

void IProperty::SetDescription(const TString& rkNewDescription)
{
    if (mDescription != rkNewDescription)
    {
        mDescription = rkNewDescription;
        MarkDirty();
    }
}

void IProperty::SetSuffix(const TString& rkNewSuffix)
{
    if (mSuffix != rkNewSuffix)
    {
        mSuffix = rkNewSuffix;
        MarkDirty();
    }
}

void IProperty::MarkDirty()
{
    RootParent()->mFlags |= EPropertyFlag::IsDirty;
}

void IProperty::ClearDirtyFlag()
{
    if (!mpScriptTemplate)
    {
        RootParent()->mFlags &= ~EPropertyFlag::IsDirty;
    }
}

bool IProperty::UsesNameMap()
{
    return Game() >= EGame::EchoesDemo &&
            !IsRootParent() &&
            !IsIntrinsic() &&
            !mpParent->IsAtomic() && // Atomic properties can use the name map, but their children shouldn't
            !IsArrayArchetype();
}

bool IProperty::HasAccurateName()
{
    // Exceptions for the three hardcoded 4CC property IDs
    if (mID == FOURCC('XFRM') ||
        mID == FOURCC('INAM') ||
        mID == FOURCC('ACTV'))
    {
        return true;
    }

    // Children of atomic properties defer to parents. Intrinsic properties and array archetypes also defer to parents.
    if ( (mpParent && mpParent->IsAtomic()) || IsIntrinsic() || IsArrayArchetype() )
    {
        if (mpParent)
            return mpParent->HasAccurateName();
        else
            return true;
    }

    // For everything else, hash the property name and check if it is a match for the property ID
    if (!mFlags.HasFlag(EPropertyFlag::HasCachedNameCheck))
    {
        CCRC32 Hash;
        Hash.Hash(*mName);
        Hash.Hash(HashableTypeName());
        u32 GeneratedID = Hash.Digest();

        if (GeneratedID == mID)
            mFlags.SetFlag( EPropertyFlag::HasCorrectPropertyName );
        else
            mFlags.ClearFlag( EPropertyFlag::HasCorrectPropertyName );

        mFlags.SetFlag(EPropertyFlag::HasCachedNameCheck);
    }

    return mFlags.HasFlag( EPropertyFlag::HasCorrectPropertyName );
}

void IProperty::RecacheName()
{
    mFlags.ClearFlag( EPropertyFlag::HasCachedNameCheck | EPropertyFlag::HasCorrectPropertyName );
}

/** IPropertyNew Accessors */
EGame IProperty::Game() const
{
    return mGame;
}

IProperty* IProperty::Create(EPropertyType Type,
                                   EGame Game)
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
    case EPropertyType::Struct:          pOut = new CStructProperty(Game);        break;
    case EPropertyType::Array:           pOut = new CArrayProperty(Game);            break;
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
                                            u32 Offset,
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
                                            u32 Offset,
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
