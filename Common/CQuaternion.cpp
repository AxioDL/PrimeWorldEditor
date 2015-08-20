    #include "CQuaternion.h"
#include <cmath>
#include <math.h>

CQuaternion::CQuaternion()
{
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}

CQuaternion::CQuaternion(float _x, float _y, float _z, float _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

CVector3f CQuaternion::XAxis()
{
    return (*this * CVector3f::skUnitX);
}

CVector3f CQuaternion::YAxis()
{
    return (*this * CVector3f::skUnitY);
}

CVector3f CQuaternion::ZAxis()
{
    return (*this * CVector3f::skUnitZ);
}

CQuaternion CQuaternion::Inverse()
{
    float fNorm = (w * w) + (x * x) + (y * y) + (z * z);

    if (fNorm > 0.f)
    {
        float fInvNorm = 1.f / fNorm;
        return CQuaternion(-x * fInvNorm, -y * fInvNorm, -z * fInvNorm, w * fInvNorm);
    }
    else
        return CQuaternion::skZero;
}

// ************ OPERATORS ************
CVector3f CQuaternion::operator*(const CVector3f& vec) const
{
    CVector3f uv, uuv;
    CVector3f qvec(x, y, z);
    uv = qvec.Cross(vec);
    uuv = qvec.Cross(uv);
    uv *= (2.0f * w);
    uuv *= 2.0f;

    return vec + uv + uuv;
}

CQuaternion CQuaternion::operator*(const CQuaternion& other) const
{
    CQuaternion out;
    out.x = ( x * other.w) + (y * other.z) - (z * other.y) + (w * other.x);
    out.y = (-x * other.z) + (y * other.w) + (z * other.x) + (w * other.y);
    out.z = ( x * other.y) - (y * other.x) + (z * other.w) + (w * other.z);
    out.w = (-x * other.x) - (y * other.y) - (z * other.z) + (w * other.w);
    return out;
}

void CQuaternion::operator *= (const CQuaternion& other)
{
    *this = *this * other;
}

// ************ STATIC ************
CQuaternion CQuaternion::FromEuler(CVector3f euler)
{
    /**
     * The commented-out code below might be faster but the conversion isn't completely correct
     * So in lieu of fixing it I'm using axis angles to convert from Eulers instead
     * I'm not sure what the difference is performance-wise but the result is 100% accurate
     */
    /*CQuaternion quat;

    // Convert from degrees to radians
    float pi = 3.14159265358979323846f;
    euler = euler * pi / 180;

    // Convert to quaternion
    float c1 = cos(euler.x / 2);
    float c2 = cos(euler.y / 2);
    float c3 = cos(euler.z / 2);
    float s1 = sin(euler.x / 2);
    float s2 = sin(euler.y / 2);
    float s3 = sin(euler.z / 2);

    quat.w =   (c1 * c2 * c3) - (s1 * s2 * s3);
    quat.x = -((s1 * c2 * c3) + (c1 * s2 * s3));
    quat.y =  ((c1 * s2 * c3) - (s1 * c2 * s3));
    quat.z =  ((s1 * s2 * c3) + (c1 * c2 * s3));*/

    CQuaternion x = CQuaternion::FromAxisAngle(euler.x, CVector3f(1,0,0));
    CQuaternion y = CQuaternion::FromAxisAngle(euler.y, CVector3f(0,1,0));
    CQuaternion z = CQuaternion::FromAxisAngle(euler.z, CVector3f(0,0,1));
    CQuaternion quat = z * y * x;

    return quat;
}

CQuaternion CQuaternion::FromAxisAngle(float angle, CVector3f axis)
{
    CQuaternion quat;
    axis = axis.Normalized();
    angle = angle * 3.14159265358979323846f / 180;

    float sa = sin(angle / 2);
    quat.x = axis.x * sa;
    quat.y = axis.y * sa;
    quat.z = axis.z * sa;
    quat.w = cos(angle / 2);
    return quat;

}

CQuaternion CQuaternion::skIdentity = CQuaternion(0.f, 0.f, 0.f, 1.f);
CQuaternion CQuaternion::skZero = CQuaternion(0.f, 0.f, 0.f, 0.f);
