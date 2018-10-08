#include "NPropertyMap.h"
#include <Common/NBasics.h>
#include <Common/Serialization/XML.h>

/** NPropertyMap: Namespace for property ID -> name mappings */
namespace NPropertyMap
{

/** Path to the property map file */
const char* gpkLegacyMapPath = "../templates/PropertyMapLegacy.xml";
const char* gpkMapPath = "../templates/PropertyMap.xml";

/** Whether to do name lookups from the legacy map */
const bool gkUseLegacyMapForNameLookups = false;

/** Whether to update names in the legacy map */
const bool gkUseLegacyMapForUpdates = false;

/** Whether the map is dirty (has unsaved changes */
bool gMapIsDirty = false;

/** Whether the map has been loaded */
bool gMapIsLoaded = false;

/** Mapping of typename hashes back to the original string */
std::unordered_map<u32, TString> gHashToTypeName;

/** Register a hash -> name mapping */
inline void RegisterTypeName(u32 TypeHash, TString TypeName)
{
    ASSERT( !TypeName.IsEmpty() );
    ASSERT( TypeName != "Unknown" );
    gHashToTypeName.emplace( std::make_pair<u32, TString>(std::move(TypeHash), std::move(TypeName)) );
}

/** Key structure for name map lookups */
struct SNameKey
{
    union
    {
        struct {
            u32 TypeHash;
            u32 ID;
        };
        struct {
            u64 Key;
        };
    };

    SNameKey()
        : TypeHash(-1), ID(-1)
    {}

    SNameKey(u32 InTypeHash, u32 InID)
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
    inline size_t operator()(const SNameKey& kKey) const
    {
        return std::hash<u64>()(kKey.Key);
    }
};

/** Value structure for name map lookups */
struct SNameValue
{
    /** Name of the property */
    TString Name;

    /** List of all properties using this ID */
    std::list<IProperty*> PropertyList;

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
std::map<u32, TString> gLegacyNameMap;

/** Internal: Creates a name key for the given property. */
SNameKey CreateKey(IProperty* pProperty)
{
    SNameKey Key;
    Key.ID = pProperty->ID();
    Key.TypeHash = CCRC32::StaticHashString( pProperty->HashableTypeName() );
    return Key;
}

/** Loads property names into memory */
void LoadMap()
{
    ASSERT( !gMapIsLoaded );

    if ( gkUseLegacyMapForNameLookups )
    {
        CXMLReader Reader(gpkLegacyMapPath);
        ASSERT(Reader.IsValid());
        Reader << SerialParameter("PropertyMap", gLegacyNameMap, SH_HexDisplay);
    }
    else
    {
        CXMLReader Reader(gpkMapPath);
        ASSERT(Reader.IsValid());
        Reader << SerialParameter("PropertyMap", gNameMap, SH_HexDisplay);
    }

    gMapIsLoaded = true;
}

inline void ConditionalLoadMap()
{
    if( !gMapIsLoaded )
    {
        LoadMap();
    }
}

/** Saves property names back out to the template file */
void SaveMap(bool Force /*= false*/)
{
    ASSERT( gMapIsLoaded );

    if( gMapIsDirty || Force )
    {
        if( gkUseLegacyMapForUpdates )
        {
            CXMLWriter Writer(gpkLegacyMapPath, "PropertyMap");
            ASSERT(Writer.IsValid());
            Writer << SerialParameter("PropertyMap", gLegacyNameMap, SH_HexDisplay);
        }
        else
        {
            CXMLWriter Writer(gpkMapPath, "PropertyMap");
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

    if (gkUseLegacyMapForNameLookups)
    {
        auto MapFind = gLegacyNameMap.find( pInProperty->ID() );
        return (MapFind == gLegacyNameMap.end() ? "Unknown" : *MapFind->second);
    }
    else
    {
        SNameKey Key = CreateKey(pInProperty);
        auto MapFind = gNameMap.find(Key);
        return (MapFind == gNameMap.end() ? "Unknown" : *MapFind->second.Name);
    }
}

/** Given a property name and type, returns the name of the property.
 *  This requires you to provide the exact type string used in the hash.
 */
const char* GetPropertyName(u32 ID, const char* pkTypeName)
{
    // Does not support legacy map
    ConditionalLoadMap();

    SNameKey Key( CCRC32::StaticHashString(pkTypeName), ID );
    auto MapFind = gNameMap.find(Key);
    return MapFind == gNameMap.end() ? "Unknown" : *MapFind->second.Name;
}

/** Updates the name of a given property in the map */
void SetPropertyName(IProperty* pProperty, const char* pkNewName)
{
    if( gkUseLegacyMapForUpdates )
    {
        gLegacyNameMap[pProperty->ID()] = pkNewName;
    }
    else
    {
        SNameKey Key = CreateKey(pProperty);
        auto MapFind = gNameMap.find(Key);

        if (MapFind != gNameMap.end())
        {
            SNameValue& Value = MapFind->second;

            if (Value.Name != pkNewName)
            {
                TString OldName = Value.Name;
                Value.Name = pkNewName;

                // Update all properties with this ID with the new name
                for (auto Iter = Value.PropertyList.begin(); Iter != Value.PropertyList.end(); Iter++)
                {
                    // If the property overrides the name, then don't change it.
                    IProperty* pIterProperty = *Iter;

                    if (pIterProperty->Name() == OldName)
                    {
                        pIterProperty->SetName(Value.Name);
                    }
                }
            }
        }
    }

    gMapIsDirty = true;
}

/** Registers a property in the name map. Should be called on all properties that use the map */
void RegisterProperty(IProperty* pProperty)
{
    ConditionalLoadMap();

    // Sanity checks to make sure we don't accidentally add non-hash property IDs to the map.
    ASSERT( pProperty->UsesNameMap() );
    ASSERT( pProperty->ID() > 0xFF && pProperty->ID() != 0xFFFFFFFF );

    // Just need to register the property in the list.
    SNameKey Key = CreateKey(pProperty);
    auto MapFind = gNameMap.find(Key);

    if( gkUseLegacyMapForNameLookups )
    {
        // If we are using the legacy map, gNameMap may be empty. We need to retrieve the name
        // from the legacy map, and create an entry in gNameMap with it.

        //@todo this prob isn't the most efficient way to do this
        if (MapFind == gNameMap.end())
        {
            auto LegacyMapFind = gLegacyNameMap.find( pProperty->ID() );
            ASSERT( LegacyMapFind != gLegacyNameMap.end() );

            SNameValue Value;
            Value.Name = LegacyMapFind->second;
            pProperty->SetName(Value.Name);

            gNameMap[Key] = Value;
            MapFind = gNameMap.find(Key);

            RegisterTypeName(Key.TypeHash, pProperty->HashableTypeName());
        }
    }
    else
    {
        pProperty->SetName( MapFind->second.Name );
    }

    ASSERT(MapFind != gNameMap.end());
    MapFind->second.PropertyList.push_back(pProperty);

    // Update the property's Name field to match the mapped name.
    pProperty->SetName( MapFind->second.Name );
}

/** Unregisters a property from the name map. Should be called on all properties that use the map on destruction. */
void UnregisterProperty(IProperty* pProperty)
{
    SNameKey Key = CreateKey(pProperty);
    auto Iter = gNameMap.find(Key);

    if (Iter != gNameMap.end())
    {
        // Found the value, now remove the element from the list.
        SNameValue& Value = Iter->second;
        NBasics::ListRemoveOne(Value.PropertyList, pProperty);
    }
}

}
