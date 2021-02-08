#ifndef NPROPERTYMAP_H
#define NPROPERTYMAP_H

#include <Common/BasicTypes.h>
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
const char* GetPropertyName(uint32 ID, const char* pkTypeName);

/** Calculate the property ID of a given name/type. */
uint32 CalculatePropertyID(const char* pkName, const char* pkTypeName);

/**
 *  Returns whether the specified name is in the map.
 *  If the ID is valid and pOutIsValid is non-null, it will return whether the current name is correct.
 */
bool IsValidPropertyID(uint32 ID, const char* pkTypeName, bool* pOutIsValid = nullptr);

/** Retrieves a list of all properties that match the requested property ID. */
void RetrievePropertiesWithID(uint32 ID, const char* pkTypeName, std::vector<IProperty*>& OutList);

/** Retrieves a list of all XML templates that contain a given property ID. */
void RetrieveXMLsWithProperty(uint32 ID, const char* pkTypeName, std::set<TString>& OutSet);

/** Updates the name of a given property in the map */
void SetPropertyName(uint32 ID, const char* pkTypeName, const char* pkNewName);

/** Change a type name of a property. */
void ChangeTypeName(IProperty* pProperty, const char* pkOldTypeName, const char* pkNewTypeName);

/** Change a type name. */
void ChangeTypeNameGlobally(const char* pkOldTypeName, const char* pkNewTypeName);

/** Registers a property in the name map. Should be called on all properties that use the map */
void RegisterProperty(IProperty* pProperty);

/** Unregisters a property from the name map. Should be called on all properties that use the map on destruction. */
void UnregisterProperty(IProperty* pProperty);

/** Class that allows for iteration through the name map */
class CIterator
{
    /** Private implementation */
    class CIteratorImpl* mpImpl;

public:
    CIterator();
    ~CIterator();

    uint32 ID() const;
    const char* Name() const;
    const char* TypeName() const;

    explicit operator bool() const;
    void operator ++();
};

}

#endif // NPROPERTYMAP_H
