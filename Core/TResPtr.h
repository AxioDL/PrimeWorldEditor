#ifndef TRESPTR_H
#define TRESPTR_H

#include <Resource/CResource.h>

template <typename ResType>
class TResPtr
{
    ResType *mpRes;

public:
    TResPtr()
        : mpRes(nullptr) {}

    TResPtr(void *pPtr)
        : mpRes(nullptr)
    {
        *this = pPtr;
    }

    TResPtr(const TResPtr<ResType>& rkSource)
        : mpRes(nullptr)
    {
        *this = rkSource;
    }

    ~TResPtr()
    {
        if (mpRes)
            mpRes->Release();
    }

    inline void Delete()
    {
        // use with caution! this function exists because not all resources are cached currently
        delete mpRes;
        mpRes = nullptr;
    }

    inline ResType* RawPointer() const
    {
        return mpRes;
    }

    inline operator ResType*() const
    {
        return mpRes;
    }

    inline ResType& operator*() const
    {
        return *mpRes;
    }

    inline ResType* operator->() const
    {
        return mpRes;
    }

    TResPtr<ResType>& operator=(void *pPtr)
    {
        // todo: this probably crashes if you try to pass a non-CResource-derived pointer. is there a safer way?
        if (mpRes)
            mpRes->Release();

        CResource *pRes = (CResource*) pPtr;

        if (ResType::StaticType() == pRes->Type())
        {
            mpRes = (ResType*) pRes;
            mpRes->Lock();
        }

        else
            mpRes = nullptr;

        return *this;
    }

    TResPtr<ResType>& operator=(const TResPtr<ResType>& rkRight)
    {
        if (mpRes)
            mpRes->Release();

        mpRes = rkRight.mpRes;

        if (mpRes)
            mpRes->Lock();

        return *this;
    }
};

#endif // TRESPTR_H
