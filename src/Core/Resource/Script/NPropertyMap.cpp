#include "NPropertyMap.h"
#include "NGameList.h"
#include <Common/NBasics.h>
#include <Common/Serialization/XML.h>

/** NPropertyMap: Namespace for property ID -> name mappings */
namespace NPropertyMap
{
namespace
{
/** Path to the property map file */
constexpr char gpkLegacyMapPath[] = "templates/PropertyMapLegacy.xml";
constexpr char gpkMapPath[] = "templates/PropertyMap.xml";

/** Whether to do name lookups from the legacy map */
constexpr bool gkUseLegacyMapForNameLookups = false;

/** Whether to update names in the legacy map */
constexpr bool gkUseLegacyMapForUpdates = false;

/** Whether the map is dirty (has unsaved changes */
bool gMapIsDirty = false;

/** Whether the map has been loaded */
bool gMapIsLoaded = false;

/** Mapping of typename hashes back to the original string */
std::unordered_map<uint32, TString> gHashToTypeName;

/** Register a hash -> name mapping */
void RegisterTypeName(uint32 TypeHash, TString TypeName)
{
    ASSERT(!TypeName.IsEmpty());
    ASSERT(TypeName != "Unknown");
    gHashToTypeName.emplace( std::make_pair<uint32, TString>(std::move(TypeHash), std::move(TypeName)) );
}

/** Key structure for name map lookups */
struct SNameKey
{
    union
    {
        struct {
            uint32 TypeHash;
            uint32 ID;
        };
        struct {
            uint64 Key;
        };
    };

    SNameKey()
        : TypeHash(UINT32_MAX), ID(UINT32_MAX)
    {}

    SNameKey(uint32 InTypeHash, uint32 InID)
        : TypeHash(InTypeHash), ID(InID)
    {}

    void Serialize(IArchive& Arc)
    {
        TString TypeName;

        if (Arc.IsWriter())
        {
            TypeName = gHashToTypeName[TypeHash];
            ASSERT(!TypeName.IsEmpty());
        }

        Arc << SerialParameter("ID", ID, SH_Attribute | SH_HexDisplay)
            << SerialParameter("Type", TypeName, SH_Attribute);

        if (Arc.IsReader())
        {
            TypeHash = TypeName.Hash32();
            RegisterTypeName(TypeHash, TypeName);
        }
    }

    friend bool operator==(const SNameKey& kLHS, const SNameKey& kRHS)
    {
        return kLHS.Key == kRHS.Key;
    }

    friend bool operator<(const SNameKey& kLHS, const SNameKey& kRHS)
    {
        return kLHS.Key < kRHS.Key;
    }
};

/** Hasher for name keys for use in std::unordered_map */
struct KeyHash
{
    size_t operator()(const SNameKey& kKey) const noexcept
    {
        return std::hash<uint64>()(kKey.Key);
    }
};

/** Value structure for name map lookups */
struct SNameValue
{
    /** Name of the property */
    TString Name;

    /** Whether this name is valid */
    bool IsValid;

    /** List of all properties using this ID */
    /** @todo - make this an intrusively linked list */
    std::set<IProperty*> PropertyList;

    void Serialize(IArchive& Arc)
    {
        Arc << SerialParameter("Name", Name, SH_Attribute);
    }

    friend bool operator==(const SNameValue& kLHS, const SNameValue& kRHS)
    {
        return kLHS.Name == kRHS.Name;
    }
};

/** Mapping of property IDs to names. In the key, the upper 32 bits
 *  are the type, and the lower 32 bits are the ID.
 */
std::map<SNameKey, SNameValue> gNameMap;

/** Legacy map that only includes the ID in the key */
std::map<uint32, TString> gLegacyNameMap;

/** Internal: Creates a name key for the given property. */
SNameKey CreateKey(IProperty* pProperty)
{
    SNameKey Key;
    Key.ID = pProperty->ID();
    Key.TypeHash = CCRC32::StaticHashString(pProperty->HashableTypeName());
    return Key;
}

SNameKey CreateKey(uint32 ID, const char* pkTypeName)
{
    return SNameKey(CCRC32::StaticHashString(pkTypeName), ID);
}
} // Anonymous namespace

/** Loads property names into memory */
void LoadMap()
{
    ASSERT(!gMapIsLoaded);
    debugf("Loading property map");

    if constexpr (gkUseLegacyMapForNameLookups)
    {
        CXMLReader Reader(gDataDir + gpkLegacyMapPath);
        ASSERT(Reader.IsValid());
        Reader << SerialParameter("PropertyMap", gLegacyNameMap, SH_HexDisplay);
    }
    else
    {
        CXMLReader Reader(gDataDir + gpkMapPath);
        ASSERT(Reader.IsValid());
        Reader << SerialParameter("PropertyMap", gNameMap, SH_HexDisplay);

        // Iterate over the map and set up the valid flags
        for (auto& [key, value] : gNameMap)
        {
            value.IsValid = CalculatePropertyID(*value.Name, *gHashToTypeName[key.TypeHash]) == key.ID;
        }
    }

    gMapIsLoaded = true;
}

static void ConditionalLoadMap()
{
    if (!gMapIsLoaded)
    {
        LoadMap();
    }
}

/** Saves property names back out to the template file */
void SaveMap(bool Force)
{
    if (!gMapIsLoaded)
    {
        if (Force)
        {
            LoadMap();
        }
        else
        {
            return;
        }
    }

    debugf("Saving property map");

    if (gMapIsDirty || Force)
    {
        if constexpr (gkUseLegacyMapForUpdates)
        {
            CXMLWriter Writer(gDataDir + gpkLegacyMapPath, "PropertyMap");
            ASSERT(Writer.IsValid());
            Writer << SerialParameter("PropertyMap", gLegacyNameMap, SH_HexDisplay);
        }
        else
        {
            // Make sure all game templates are loaded and clear out ID-type pairs that aren't used
            // This mostly occurs when type names are changed - unneeded pairings with the old type can be left in the map
            NGameList::LoadAllGameTemplates();

            for (auto Iter = gNameMap.begin(); Iter != gNameMap.end(); ++Iter)
            {
                SNameValue& Value = Iter->second;

                if (Value.PropertyList.empty())
                {
                    Iter = gNameMap.erase(Iter);
                }
            }

            // Perform the actual save
            CXMLWriter Writer(gDataDir + gpkMapPath, "PropertyMap");
            ASSERT(Writer.IsValid());
            Writer << SerialParameter("PropertyMap", gNameMap, SH_HexDisplay);
        }
        gMapIsDirty = false;
    }
}

/** Given a property ID and type, returns the name of the property */
const char* GetPropertyName(IProperty* pInProperty)
{
    ConditionalLoadMap();

    if constexpr (gkUseLegacyMapForNameLookups)
    {
        const auto MapFind = gLegacyNameMap.find(pInProperty->ID());
        return MapFind == gLegacyNameMap.cend() ? "Unknown" : *MapFind->second;
    }
    else
    {
        const SNameKey Key = CreateKey(pInProperty);
        const auto MapFind = gNameMap.find(Key);
        return MapFind == gNameMap.end() ? "Unknown" : *MapFind->second.Name;
    }
}

/** Given a property name and type, returns the name of the property.
 *  This requires you to provide the exact type string used in the hash.
 */
const char* GetPropertyName(uint32 ID, const char* pkTypeName)
{
    // Does not support legacy map
    ConditionalLoadMap();

    const SNameKey Key = CreateKey(ID, pkTypeName);
    const auto MapFind = gNameMap.find(Key);
    return MapFind == gNameMap.cend() ? "Unknown" : *MapFind->second.Name;
}

/** Calculate the property ID of a given name/type. */
uint32 CalculatePropertyID(const char* pkName, const char* pkTypeName)
{
    CCRC32 CRC;
    CRC.Hash(pkName);
    CRC.Hash(pkTypeName);
    return CRC.Digest();
}

/** Returns whether the specified ID is in the map. */
bool IsValidPropertyID(uint32 ID, const char* pkTypeName, bool* pOutIsValid /*= nullptr*/)
{
    const SNameKey Key = CreateKey(ID, pkTypeName);
    const auto MapFind = gNameMap.find(Key);

    if (MapFind != gNameMap.cend())
    {
        if (pOutIsValid != nullptr)
        {
            SNameValue& Value = MapFind->second;
            *pOutIsValid = Value.IsValid;
        }
        return true;
    }

    return false;
}

/** Retrieves a list of all properties that match the requested property ID. */
void RetrievePropertiesWithID(uint32 ID, const char* pkTypeName, std::vector<IProperty*>& OutList)
{
    const SNameKey Key = CreateKey(ID, pkTypeName);
    const auto MapFind = gNameMap.find(Key);

    if (MapFind == gNameMap.cend())
        return;

    SNameValue& Value = MapFind->second;
    OutList.reserve(Value.PropertyList.size());

    for (auto* property : Value.PropertyList)
    {
        OutList.push_back(property);
    }
}

/** Retrieves a list of all XML templates that contain a given property ID. */
void RetrieveXMLsWithProperty(uint32 ID, const char* pkTypeName, std::set<TString>& OutSet)
{
    const SNameKey Key = CreateKey(ID, pkTypeName);
    const auto MapFind = gNameMap.find(Key);

    if (MapFind == gNameMap.cend())
        return;

    SNameValue& NameValue = MapFind->second;

    for (auto* pProperty : NameValue.PropertyList)
    {
        OutSet.insert(pProperty->GetTemplateFileName());
    }
}

/** Updates the name of a given property in the map */
void SetPropertyName(uint32 ID, const char* pkTypeName, const char* pkNewName)
{
    if constexpr (gkUseLegacyMapForUpdates)
    {
        auto Iter = gLegacyNameMap.find(ID);

        if (Iter == gLegacyNameMap.end() || Iter->second != pkNewName)
        {
            Iter->second = pkNewName;
            gMapIsDirty = true;
        }
    }
    else
    {
        const SNameKey Key = CreateKey(ID, pkTypeName);
        auto MapFind = gNameMap.find(Key);

        if (MapFind == gNameMap.cend())
            return;

        SNameValue& Value = MapFind->second;

        if (Value.Name == pkNewName)
            return;

        const TString OldName = std::move(Value.Name);
        Value.Name = pkNewName;
        gMapIsDirty = true;

        // Update all properties with this ID with the new name
        for (IProperty* pIterProperty : Value.PropertyList)
        {
            // If the property overrides the name, then don't change it.
            if (pIterProperty->Name() == OldName)
            {
                pIterProperty->SetName(Value.Name);
            }
        }
    }
}

/** Change a type name of a property. */
void ChangeTypeName(IProperty* pProperty, const char* pkOldTypeName, const char* pkNewTypeName)
{
    const uint32 OldTypeHash = CCRC32::StaticHashString(pkOldTypeName);
    const uint32 NewTypeHash = CCRC32::StaticHashString(pkNewTypeName);

    if (OldTypeHash == NewTypeHash)
    {
        return;
    }

    // Start off with a ist of all properties in the same inheritance chain as this one.
    std::list<IProperty*> Properties;
    IProperty* pArchetype = pProperty->RootArchetype();
    pArchetype->GatherAllSubInstances(Properties, true);

    for (auto* property : Properties)
    {
        pProperty = property;

        if (pProperty->UsesNameMap())
        {
            const SNameKey OldKey(OldTypeHash, pProperty->ID());
            const SNameKey NewKey(NewTypeHash, pProperty->ID());

            // Disassociate this property from the old mapping.
            bool WasRegistered = false;
            auto Find = gNameMap.find(OldKey);

            if (Find != gNameMap.cend())
            {
                SNameValue& Value = Find->second;
                WasRegistered = Value.PropertyList.find(pProperty) != Value.PropertyList.cend();
            }

            // Create a key for the new property and add it to the list.
            Find = gNameMap.find(NewKey);

            if (Find == gNameMap.cend())
            {
                SNameValue Value;
                Value.Name = pProperty->Name();
                Value.IsValid = CalculatePropertyID(*Value.Name, pkNewTypeName) == pProperty->ID();
                Find = gNameMap.insert_or_assign(NewKey, std::move(Value)).first;
            }
            ASSERT(Find != gNameMap.cend());

            if (WasRegistered)
            {
                Find->second.PropertyList.insert(pProperty);
            }

            gMapIsDirty = true;
        }
    }

    RegisterTypeName(NewTypeHash, pkNewTypeName);
}

/** Change a type name. */
void ChangeTypeNameGlobally(const char* pkOldTypeName, const char* pkNewTypeName)
{
    const uint32 OldTypeHash = CCRC32::StaticHashString(pkOldTypeName);
    const uint32 NewTypeHash = CCRC32::StaticHashString(pkNewTypeName);

    if (OldTypeHash == NewTypeHash)
    {
        return;
    }

    // The process here is basically to find all properties with a matching typename
    // hash and update the hashes to the new type. Not 100% sure if this is the best
    // way to go about doing it. From what I understand, insert() does not invalidate
    // iterators, and extract() only invalidates the iterator being extracted. So this
    // implementation should work correctly.
    for (auto MapIter = gNameMap.begin(); MapIter != gNameMap.end(); ++MapIter)
    {
        if (MapIter->first.TypeHash == OldTypeHash)
        {
            auto PrevIter = MapIter;
            --PrevIter;

            auto MapNode = gNameMap.extract(MapIter);
            MapIter = PrevIter;

            SNameKey& Key = MapNode.key();
            Key.TypeHash = NewTypeHash;
            gNameMap.insert(std::move(MapNode));
            gMapIsDirty = true;
        }
    }

    RegisterTypeName(NewTypeHash, pkNewTypeName);
    gHashToTypeName[NewTypeHash] = pkNewTypeName;
}

/** Registers a property in the name map. Should be called on all properties that use the map */
void RegisterProperty(IProperty* pProperty)
{
    ConditionalLoadMap();

    // Sanity checks to make sure we don't accidentally add non-hash property IDs to the map.
    ASSERT(pProperty->UsesNameMap());
    ASSERT(pProperty->ID() > 0xFF && pProperty->ID() != 0xFFFFFFFF);

    // Just need to register the property in the list.
    SNameKey Key = CreateKey(pProperty);
    auto MapFind = gNameMap.find(Key);

    if constexpr (gkUseLegacyMapForNameLookups)
    {
        // If we are using the legacy map, gNameMap may be empty. We need to retrieve the name
        // from the legacy map, and create an entry in gNameMap with it.

        //@todo this prob isn't the most efficient way to do this
        if (MapFind == gNameMap.end())
        {
            const auto LegacyMapFind = gLegacyNameMap.find(pProperty->ID());
            ASSERT(LegacyMapFind != gLegacyNameMap.cend());

            SNameValue Value;
            Value.Name = LegacyMapFind->second;
            Value.IsValid = (CalculatePropertyID(*Value.Name, pProperty->HashableTypeName()) == pProperty->ID());
            pProperty->SetName(Value.Name);

            MapFind = gNameMap.insert_or_assign(Key, Value).first;
            ASSERT(MapFind != gNameMap.cend());

            RegisterTypeName(Key.TypeHash, pProperty->HashableTypeName());
        }
    }
    else
    {
        // If we didn't find the property name, check for int<->choice conversions
        if (MapFind == gNameMap.cend())
        {
            if (pProperty->Type() == EPropertyType::Int)
            {
                const uint32 ChoiceHash = CCRC32::StaticHashString("choice");
                const SNameKey ChoiceKey(ChoiceHash, pProperty->ID());
                MapFind = gNameMap.find(ChoiceKey);
            }
            else if (pProperty->Type() == EPropertyType::Choice)
            {
                const uint32 IntHash = CCRC32::StaticHashString("int");
                const SNameKey IntKey(IntHash, pProperty->ID());
                MapFind = gNameMap.find(IntKey);
            }
        }

        // If we still didn't find it, register the property name in the map
        if (MapFind == gNameMap.cend())
        {
            SNameValue Value{"Unknown", false, {}};
            MapFind = gNameMap.insert_or_assign(Key, std::move(Value)).first;
            RegisterTypeName(Key.TypeHash, pProperty->HashableTypeName());
        }

        // We should have a valid iterator at this point no matter what.
        ASSERT(MapFind != gNameMap.cend());
        pProperty->SetName(MapFind->second.Name);
    }

    MapFind->second.PropertyList.insert(pProperty);

    // Update the property's Name field to match the mapped name.
    pProperty->SetName(MapFind->second.Name);
}

/** Unregisters a property from the name map. Should be called on all properties that use the map on destruction. */
void UnregisterProperty(IProperty* pProperty)
{
    const SNameKey Key = CreateKey(pProperty);
    const auto Iter = gNameMap.find(Key);

    if (Iter == gNameMap.cend())
        return;

    // Found the value, now remove the element from the list.
    SNameValue& Value = Iter->second;
    Value.PropertyList.erase(pProperty);
}

/** Class for iterating through the map */
class CIteratorImpl
{
public:
    std::map<SNameKey, SNameValue>::const_iterator mIter;
};

CIterator::CIterator()
{
    mpImpl = new CIteratorImpl;
    mpImpl->mIter = gNameMap.begin();
}

CIterator::~CIterator()
{
    delete mpImpl;
}

uint32 CIterator::ID() const
{
    return mpImpl->mIter->first.ID;
}

const char* CIterator::Name() const
{
    return *mpImpl->mIter->second.Name;
}

const char* CIterator::TypeName() const
{
    uint32 TypeHash = mpImpl->mIter->first.TypeHash;
    auto Find = gHashToTypeName.find(TypeHash);
    ASSERT(Find != gHashToTypeName.end());
    return *Find->second;
}

CIterator::operator bool() const
{
    return mpImpl->mIter != gNameMap.end();
}

void CIterator::operator++()
{
    mpImpl->mIter++;
}

}
