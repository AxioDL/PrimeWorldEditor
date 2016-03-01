#ifndef SLINK_H
#define SLINK_H

#include <Common/TString.h>
#include <Common/types.h>

struct SState
{
    u32 ID;
    TString Name;

    SState() {}
    SState(u32 _ID, const TString& rkName) : ID(_ID), Name(rkName) {}
};

struct SMessage
{
    u32 ID;
    TString Name;

    SMessage() {}
    SMessage(u32 _ID, const TString& rkName) : ID(_ID), Name(rkName) {}
};

struct SLink
{
    u32 State;
    u32 Message;
    u32 ObjectID; // not a pointer because it can refer to objects outside the current area
};

#endif // SLINK_H
