#ifndef NPROPERTYMAP_H
#define NPROPERTYMAP_H

#include <Common/TString.h>

#include <cstdint>
#include <set>
#include <string_view>
#include <vector>

class IProperty;

/** NPropertyMap: Namespace for property ID -> name mappings */
namespace NPropertyMap
{

/** Loads property names into memory */
void LoadMap();

/** Saves property names back out to the template file */
void SaveMap(bool Force = false);

/** Returns the name of the property */
const char* GetPropertyName(const IProperty* pProperty);

/** Given a property name and type, returns the name of the property.
 *  This requires you to provide the exact type string used in the hash.
 */
const char* GetPropertyName(uint32_t ID, std::string_view typeName);

/** Calculate the property ID of a given name/type. */
uint32_t CalculatePropertyID(std::string_view name, std::string_view typeName);

/**
 *  Returns whether the specified name is in the map.
 *  If the ID is valid and pOutIsValid is non-null, it will return whether the current name is correct.
 */
bool IsValidPropertyID(uint32_t ID, std::string_view typeName, bool* pOutIsValid = nullptr);

/** Retrieves a list of all properties that match the requested property ID. */
std::vector<IProperty*> RetrievePropertiesWithID(uint32_t ID, std::string_view typeName);

/** Retrieves a list of all XML templates that contain a given property ID. */
std::set<TString> RetrieveXMLsWithProperty(uint32_t ID, std::string_view typeName);

/** Updates the name of a given property in the map */
void SetPropertyName(uint32_t ID, std::string_view typeName, std::string_view newName);

/** Change a type name of a property. */
void ChangeTypeName(IProperty* pProperty, std::string_view oldTypeName, std::string_view newTypeName);

/** Change a type name. */
void ChangeTypeNameGlobally(std::string_view oldTypeName, std::string_view newTypeName);

/** Registers a property in the name map. Should be called on all properties that use the map */
void RegisterProperty(IProperty* pProperty);

/** Unregisters a property from the name map. Should be called on all properties that use the map on destruction. */
void UnregisterProperty(IProperty* pProperty);

} // namespace NPropertyMap

#endif // NPROPERTYMAP_H
