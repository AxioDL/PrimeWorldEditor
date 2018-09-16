#ifndef NBASICS_H
#define NBASICS_H

#include "types.h"
#include <vector>

namespace NBasics
{

/** Remove an element from a vector */
template<typename T>
bool VectorRemoveOne(std::vector<T>& Vector, const T& kElement)
{
    for (auto Iter = Vector.begin(); Iter != Vector.end(); Iter++)
    {
        if (*Iter == kElement)
        {
            Vector.erase(Iter);
            return true;
        }
    }
    return false;
}

/** Remove all occurrences of an element from a vector. Returns the number of elements that were removed. */
template<typename T>
int VectorRemoveAll(std::vector<T>& Vector, const T& kElement)
{
    int NumRemoved = 0;

    for (auto Iter = Vector.begin(); Iter != Vector.end(); Iter++)
    {
        if (*Iter == kElement)
        {
            Iter = Vector.erase(Iter);
            NumRemoved++;
        }
    }

    return NumRemoved;
}

/** Returns whether the vector contains the given element */
template<typename T>
bool VectorContains(std::vector<T>& Vector, const T& kElement)
{
    for (auto Iter = Vector.begin(); Iter != Vector.end(); Iter++)
    {
        if (*Iter == kElement)
        {
            return true;
        }
    }

    return false;
}

/** Adds an element to a vector only if it is not already present */
template<typename T>
bool VectorAddUnique(std::vector<T>& Vector, const T& kElement)
{
    if (!VectorContainsElement(Vector, kElement))
    {
        Vector.push_back(kElement);
        return true;
    }

    return false;
}

}

#endif // NBASICS_H
