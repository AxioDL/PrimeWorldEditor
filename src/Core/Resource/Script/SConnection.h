#ifndef SCONNECTION_H
#define SCONNECTION_H

#include <Common/types.h>

struct SLink
{
    u32 State;
    u32 Message;
    u32 ObjectID; // not a pointer because it can refer to objects outside the current area
};

#endif // SCONNECTION_H
