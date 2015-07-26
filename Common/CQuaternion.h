#ifndef CQUATERNION_H
#define CQUATERNION_H

#include "CVector3f.h"

class CQuaternion
{
public:
    float x, y, z, w;

    CQuaternion();
    CQuaternion(float _x, float _y, float _z, float _w);

    // Operators
    CQuaternion operator*(const CQuaternion& other) const;
    void operator *= (const CQuaternion& other);

    // Static
    static CQuaternion FromEuler(CVector3f euler);
    static CQuaternion FromAxisAngle(float angle, CVector3f axis);

    static CQuaternion skIdentity;
};

#endif // CQUATERNION_H
