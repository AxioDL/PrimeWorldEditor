#include "IPropertyNew.h"
#include "Property/CAssetProperty.h"
#include "Property/CArrayProperty.h"
#include "Property/CEnumProperty.h"
#include "Property/CFlagsProperty.h"
#include "Property/CPointerProperty.h"

#include "Core/Resource/Script/CMasterTemplate.h"
#include "Core/Resource/Script/CScriptTemplate.h"

/** IPropertyNew */
IPropertyNew::IPropertyNew()
    : mpParent( nullptr )
    , mpPointerParent( nullptr )
    , mpArchetype( nullptr )
    , mOffset( -1 )
    , mID( -1 )
    , mCookPreference( ECookPreferenceNew::Default )
    , mMinVersion( 0.0f )
    , mMaxVersion( FLT_MAX )
{}

void IPropertyNew::_CalcOffset()
{
    // For standard properties, append to the end of the parent.
    bool IsRootArrayArchetype = (IsArrayArchetype() && TPropCast<CArrayProperty>(mpParent) != nullptr);

    if (mpParent && !IsRootArrayArchetype)
    {
        // When we have a parent, our data is usually located inside the parent's property data. So we want to
        // position ourself at the end of the parent's existing children so we don't overlap any other properties.
        IPropertyNew* pLastChild = (mpParent->mChildren.empty() ? nullptr : mpParent->mChildren.back());

        if (pLastChild)
        {
            mOffset = pLastChild->mOffset + pLastChild->DataSize();
        }
        else if (mpParent != mpPointerParent)
        {
            mOffset = mpParent->mOffset;
        }
        else
        {
            mOffset = 0;
        }

        mOffset = ALIGN(mOffset, DataAlignment());
    }
    // Array archetypes are accessed differently because they have no way of knowing
    // which array index is meant to be accessed. So the offset is 0 and the caller
    // is responsible for passing in a pointer to the correct array item.
    else
    {
        mOffset = 0;
    }
}

void IPropertyNew::_ClearChildren()
{
    for (int ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
        delete mChildren[ChildIdx];

    mChildren.clear();
}

IPropertyNew::~IPropertyNew()
{
    // Remove from archetype
    if( mpArchetype != nullptr )
    {
        NBasics::VectorRemoveOne(mpArchetype->mSubInstances, this);
    }

    // If this is an archetype, all our sub-instances should have destructed first.
    if( IsArchetype() )
    {
        ASSERT(mSubInstances.empty());
    }

    // Delete children
    _ClearChildren();
}

const char* IPropertyNew::HashableTypeName() const
{
    return PropEnumToHashableTypeName( Type() );
}

void* IPropertyNew::GetChildDataPointer(void* pPropertyData) const
{
    return pPropertyData;
}

void IPropertyNew::Serialize(IArchive& rArc)
{
    if (rArc.Game() <= ePrime && !IsArchetype())
    {
        rArc << SerialParameter("Name", mName);
    }

    rArc << SerialParameter("ID", mID, SH_HexDisplay | SH_Attribute | SH_Optional, (u32) 0xFFFFFFFF)
         << SerialParameter("Description", mDescription, SH_Optional)
         << SerialParameter("CookPreference", mCookPreference, SH_Optional, ECookPreferenceNew::Default)
         << SerialParameter("MinVersion", mMinVersion, SH_Optional, 0.f)
         << SerialParameter("MaxVersion", mMaxVersion, SH_Optional, FLT_MAX);

    // Children don't get serialized for most property types
}

void IPropertyNew::InitFromArchetype(IPropertyNew* pOther)
{
    //@todo maybe somehow use Serialize for this instead?
    mpArchetype = pOther;
    mFlags = pOther->mFlags & EPropertyFlag::ArchetypeCopyFlags;
    mID = pOther->mID;
    mName = pOther->mName;
    mDescription = pOther->mDescription;
    mSuffix = pOther->mSuffix;
    mCookPreference = pOther->mCookPreference;
    mMinVersion = pOther->mMinVersion;
    mMaxVersion = pOther->mMaxVersion;

    // Copy children
    _ClearChildren();

    for (u32 ChildIdx = 0; ChildIdx < pOther->mChildren.size(); ChildIdx++)
    {
        CreateCopy( pOther->mChildren[ChildIdx], this );
    }
}

TString IPropertyNew::GetTemplateFileName()
{
    if (mpScriptTemplate)
    {
        return mpScriptTemplate->SourceFile();
    }
    else if (IsArchetype())
    {
        IPropertyNew* pRootParent = RootParent();
        ASSERT(pRootParent != this);
        return pRootParent->GetTemplateFileName();
    }
    else
    {
        return mpArchetype->GetTemplateFileName();
    }
}

void* IPropertyNew::RawValuePtr(void* pData) const
{
    // For array archetypes, the caller needs to provide the pointer to the correct array item
    // Array archetypes can't store their index in the array so it's impossible to determine the correct pointer.
    void* pBasePtr = (mpPointerParent && !IsArrayArchetype() ? mpPointerParent->GetChildDataPointer(pData) : pData);
    void* pValuePtr = ((char*)pBasePtr + mOffset);
    return pValuePtr;
}

IPropertyNew* IPropertyNew::ChildByID(u32 ID) const
{
    for (u32 ChildIdx = 0; ChildIdx < mChildren.size(); ChildIdx++)
    {
        if (mChildren[ChildIdx]->mID == ID)
            return mChildren[ChildIdx];
    }

    return nullptr;
}

IPropertyNew* IPropertyNew::ChildByIDString(const TIDString& rkIdString)
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

    IPropertyNew* pNextChild = ChildByID(NextChildID);

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

bool IPropertyNew::ShouldCook(void*pPropertyData) const
{
    switch (mCookPreference)
    {
    case ECookPreferenceNew::Always:
        return true;

    case ECookPreferenceNew::Never:
        return false;

    default:
        return (Game() < eReturns ? true : !MatchesDefault(pPropertyData));
    }
}

void IPropertyNew::SetName(const TString& rkNewName)
{
    mName = rkNewName;
    mFlags.ClearFlag(EPropertyFlag::HasCachedNameCheck);
}

void IPropertyNew::SetDescription(const TString& rkNewDescription)
{
    mDescription = rkNewDescription;
}

void IPropertyNew::SetSuffix(const TString& rkNewSuffix)
{
    mSuffix = rkNewSuffix;
}

bool IPropertyNew::HasAccurateName()
{
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

/** IPropertyNew Accessors */
EGame IPropertyNew::Game() const
{
    return mGame;
}

IPropertyNew* IPropertyNew::Create(EPropertyTypeNew Type,
                                   IPropertyNew* pParent,
                                   EGame Game,
                                   CScriptTemplate* pScript,
                                   bool CallPostInit /*= true*/)
{
    IPropertyNew* pOut = nullptr;

    switch (Type)
    {
    case EPropertyTypeNew::Bool:            pOut = new CBoolProperty; break;
    case EPropertyTypeNew::Byte:            pOut = new CByteProperty; break;
    case EPropertyTypeNew::Short:           pOut = new CShortProperty; break;
    case EPropertyTypeNew::Int:             pOut = new CIntProperty; break;
    case EPropertyTypeNew::Float:           pOut = new CFloatProperty; break;
    case EPropertyTypeNew::Choice:          pOut = new CChoiceProperty; break;
    case EPropertyTypeNew::Enum:            pOut = new CEnumProperty; break;
    case EPropertyTypeNew::Flags:           pOut = new CFlagsProperty; break;
    case EPropertyTypeNew::String:          pOut = new CStringProperty; break;
    case EPropertyTypeNew::Vector:          pOut = new CVectorProperty; break;
    case EPropertyTypeNew::Color:           pOut = new CColorProperty; break;
    case EPropertyTypeNew::Asset:           pOut = new CAssetProperty; break;
    case EPropertyTypeNew::Sound:           pOut = new CSoundProperty; break;
    case EPropertyTypeNew::Animation:       pOut = new CAnimationProperty; break;
    case EPropertyTypeNew::AnimationSet:    pOut = new CAnimationSetProperty; break;
    case EPropertyTypeNew::Sequence:        pOut = new CSequenceProperty; break;
    case EPropertyTypeNew::Spline:          pOut = new CSplineProperty; break;
    case EPropertyTypeNew::Guid:            pOut = new CGuidProperty; break;
    case EPropertyTypeNew::Pointer:         pOut = new CPointerProperty; break;
    case EPropertyTypeNew::Struct:          pOut = new CStructPropertyNew; break;
    case EPropertyTypeNew::Array:           pOut = new CArrayProperty; break;
    }

    if (!pOut)
    {
        // this shouldn't be possible! unhandled type! someone fucked up!
        ASSERT(false);
        return nullptr;
    }

    // Set parent and offset
    pOut->mpParent = pParent;

    if (pParent)
    {
        pOut->mFlags = pParent->mFlags & EPropertyFlag::InheritableFlags;

        if (pParent->IsPointerType())
        {
            pOut->mpPointerParent = pParent;
        }
        else
        {
            pOut->mpPointerParent = pParent->mpPointerParent;
        }
    }

    // Set other metadata
    pOut->mGame = Game;
    pOut->mpScriptTemplate = pScript;
    pOut->_CalcOffset();

    // Add to the parent's array. This needs to be done -after- we calculate offset, as adding a child to
    // the parent property will change the offset that gets calculated.
    if (pParent)
    {
        pParent->mChildren.push_back(pOut);
    }

    if (CallPostInit)
    {
        pOut->PostInitialize();
    }

    return pOut;
}

IPropertyNew* IPropertyNew::CreateCopy(IPropertyNew* pArchetype,
                                       IPropertyNew* pParent)
{
    // Note this is mainly going to be used to create copies from struct/enum/flag archetype properties.
    // Properties that have archetypes will never be the root property of a script template, and there
    // is no case where we will be creating archetypes outside this context. As such, pParent should
    // always be valid.
    ASSERT(pParent != nullptr);

    IPropertyNew* pOut = Create(pArchetype->Type(), pParent, pParent->mGame, pParent->mpScriptTemplate, false);
    pOut->InitFromArchetype(pArchetype);
    pArchetype->mSubInstances.push_back(pOut);
    return pOut;
}

IPropertyNew* IPropertyNew::CreateIntrinsic(EPropertyTypeNew Type,
                                            IPropertyNew* pParent,
                                            u32 Offset,
                                            const TString& rkName)
{
    IPropertyNew* pOut = Create(Type, pParent, pParent ? pParent->mGame : eUnknownGame, nullptr, false);
    pOut->mOffset = Offset;
    pOut->SetName(rkName);
    pOut->PostInitialize();
    return pOut;
}

IPropertyNew* IPropertyNew::ArchiveConstructor(EPropertyTypeNew Type,
                                               const IArchive& Arc)
{
    IPropertyNew* pParent = Arc.FindParentObject<IPropertyNew>();
    CScriptTemplate* pTemplate = (pParent ? pParent->ScriptTemplate() : Arc.FindParentObject<CScriptTemplate>());
    EGame Game = Arc.Game();
    return Create(Type, pParent, Game, pTemplate);
}
