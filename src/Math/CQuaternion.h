#ifndef CQUATERNION_H
#define CQUATERNION_H

#include "CVector3f.h"

class CQuaternion
{
public:
    float W, X, Y, Z;

    CQuaternion();
    CQuaternion(float _W, float _X, float _Y, float _Z);

    CVector3f XAxis() const;
    CVector3f YAxis() const;
    CVector3f ZAxis() const;
    CQuaternion Inverse() const;
    CVector3f ToEuler() const;

    // Operators
    CVector3f operator*(const CVector3f& rkVec) const;
    CQuaternion operator*(const CQuaternion& rkOther) const;
    void operator *= (const CQuaternion& rkOther);

    // Static
    static CQuaternion FromEuler(CVector3f Euler);
    static CQuaternion FromAxisAngle(float Angle, CVector3f Axis);
    static CQuaternion FromRotationMatrix(const CMatrix4f& rkRotMtx);
    static CQuaternion FromAxes(const CVector3f& rkX, const CVector3f& rkY, const CVector3f& rkZ);

    static CQuaternion skIdentity;
    static CQuaternion skZero;
};

#endif // CQUATERNION_H
