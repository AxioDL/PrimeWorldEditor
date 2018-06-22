#ifndef NBASICS_H
#define NBASICS_H

#include "types.h"
#include <vector>

namespace NBasics
{

/** Remove an element from a vector */
template<typename T>
bool VectorRemoveOne(std::vector<T>& rVector, const T& rkElement)
{
    for (auto Iter = rVector.begin(); Iter != rVector.end(); Iter++)
    {
        if (*Iter == rkElement)
        {
            rVector.erase(Iter);
            return true;
        }
    }
    return false;
}

/** Remove all occurrences of an element from a vector. Returns the number of elements that were removed. */
template<typename T>
int VectorRemoveAll(std::vector<T>& rVector, const T& rkElement)
{
    int NumRemoved = 0;

    for (auto Iter = rVector.begin(); Iter != rVector.end(); Iter++)
    {
        if (*Iter == rkElement)
        {
            Iter = rVector.erase(Iter);
            NumRemoved++;
        }
    }

    return NumRemoved;
}

}

#endif // NBASICS_H
