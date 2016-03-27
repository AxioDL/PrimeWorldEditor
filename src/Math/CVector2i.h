#ifndef CVECTOR2I_H
#define CVECTOR2I_H

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CVector2i
{
public:
    int X, Y;
    CVector2i();
    CVector2i(int XY);
    CVector2i(int _X, int _Y);

    CVector2i operator+(const CVector2i& rkOther) const;
    CVector2i operator-(const CVector2i& rkOther) const;
    CVector2i operator*(const CVector2i& rkOther) const;
    CVector2i operator/(const CVector2i& rkOther) const;
    void operator+=(const CVector2i& rkOther);
    void operator-=(const CVector2i& rkOther);
    void operator*=(const CVector2i& rkOther);
    void operator/=(const CVector2i& rkOther);
    CVector2i operator+(const int Other) const;
    CVector2i operator-(const int Other) const;
    CVector2i operator*(const int Other) const;
    CVector2i operator/(const int Other) const;
    void operator+=(const int Other);
    void operator-=(const int Other);
    void operator*=(const int Other);
    void operator/=(const int Other);
    bool operator==(const CVector2i& rkOther) const;
    bool operator!=(const CVector2i& rkOther) const;
    int& operator[](int Index);

    // Static Members
    static const CVector2i skZero;
};

#endif // CVECTOR2I_H
