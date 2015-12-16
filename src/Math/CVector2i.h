#ifndef CVECTOR2I_H
#define CVECTOR2I_H

#include <FileIO/IInputStream.h>
#include <FileIO/IOutputStream.h>

class CVector2i
{
public:
    int x, y;
    CVector2i();
    CVector2i(int xy);
    CVector2i(int _x, int _y);

    CVector2i operator+(const CVector2i& other) const;
    CVector2i operator-(const CVector2i& other) const;
    CVector2i operator*(const CVector2i& other) const;
    CVector2i operator/(const CVector2i& other) const;
    void operator+=(const CVector2i& other);
    void operator-=(const CVector2i& other);
    void operator*=(const CVector2i& other);
    void operator/=(const CVector2i& other);
    CVector2i operator+(const int other) const;
    CVector2i operator-(const int other) const;
    CVector2i operator*(const int other) const;
    CVector2i operator/(const int other) const;
    void operator+=(const int other);
    void operator-=(const int other);
    void operator*=(const int other);
    void operator/=(const int other);
    bool operator==(const CVector2i& other) const;
    bool operator!=(const CVector2i& other) const;
    int& operator[](int index);

    // Static Members
    static const CVector2i skZero;
};

#endif // CVECTOR2I_H
