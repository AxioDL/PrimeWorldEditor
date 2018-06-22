#ifndef TPROPERTYPROXY_H
#define TPROPERTYPROXY_H

#include <type_traits>

/**
 * Lightweight proxy class representing a property instance. Easy to read/modify
 * specific properties and efficient to pass around.
 */
template<class PropertyClass>
class TPropertyProxy
{
    typedef PropertyClass::ValueType ValueType;

    /** Property data buffer */
    void* mpDataPtr;

    /** Source property */
    PropertyClass* mpProperty;

public:
    TPropertyProxy()
        : mpDataPtr(nullptr)
        , mpProperty(nullptr)
    {}

    TPropertyProxy(void* pDataPtr, PropertyClass* pProperty)
        : mpDataPtr(pDataPtr)
        , mpProperty(pProperty)
    {}

    /** Returns whether this proxy points to a valid property instance */
    bool IsValid() const
    {
        return mpDataPtr != nullptr && mpProperty != nullptr;
    }
};

#endif // TPROPERTYPROXY_H
