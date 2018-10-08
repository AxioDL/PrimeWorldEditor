#ifndef NPROPERTYMAP_H
#define NPROPERTYMAP_H

#include <Common/types.h>
#include "Core/Resource/Script/Property/IProperty.h"

/** NPropertyMap: Namespace for property ID -> name mappings */
namespace NPropertyMap
{

/** Loads property names into memory */
void LoadMap();

/** Saves property names back out to the template file */
void SaveMap(bool Force = false);

/** Returns the name of the property */
const char* GetPropertyName(IProperty* pProperty);

/** Given a property name and type, returns the name of the property.
 *  This requires you to provide the exact type string used in the hash.
 */
const char* GetPropertyName(u32 ID, const char* pkTypeName);

/** Updates the name of a given property in the map */
void SetPropertyName(IProperty* pProperty, const char* pkNewName);

/** Registers a property in the name map. Should be called on all properties that use the map */
void RegisterProperty(IProperty* pProperty);

/** Unregisters a property from the name map. Should be called on all properties that use the map on destruction. */
void UnregisterProperty(IProperty* pProperty);

}

#endif // NPROPERTYMAP_H
