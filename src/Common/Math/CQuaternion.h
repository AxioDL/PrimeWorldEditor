#ifndef CQUATERNION_H
#define CQUATERNION_H

#include "CVector3f.h"

class CQuaternion
{
public:
    float w, x, y, z;

    CQuaternion();
    CQuaternion(float _w, float _x, float _y, float _z);

    CVector3f XAxis();
    CVector3f YAxis();
    CVector3f ZAxis();
    CQuaternion Inverse();
    CVector3f ToEuler();

    // Operators
    CVector3f operator*(const CVector3f& vec) const;
    CQuaternion operator*(const CQuaternion& other) const;
    void operator *= (const CQuaternion& other);

    // Static
    static CQuaternion FromEuler(CVector3f euler);
    static CQuaternion FromAxisAngle(float angle, CVector3f axis);
    static CQuaternion FromRotationMatrix(const CMatrix4f& RotMtx);
    static CQuaternion FromAxes(const CVector3f& X, const CVector3f& Y, const CVector3f& Z);

    static CQuaternion skIdentity;
    static CQuaternion skZero;
};

#endif // CQUATERNION_H
