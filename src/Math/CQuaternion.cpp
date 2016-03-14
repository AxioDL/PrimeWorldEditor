#include "CQuaternion.h"
#include "CMatrix4f.h"
#include "MathUtil.h"
#include <cmath>
#include <math.h>

CQuaternion::CQuaternion()
{
    w = 0.f;
    x = 0.f;
    y = 0.f;
    z = 0.f;
}

CQuaternion::CQuaternion(float _w, float _x, float _y, float _z)
{
    w = _w;
    x = _x;
    y = _y;
    z = _z;
}

CVector3f CQuaternion::XAxis() const
{
    return (*this * CVector3f::skUnitX);
}

CVector3f CQuaternion::YAxis() const
{
    return (*this * CVector3f::skUnitY);
}

CVector3f CQuaternion::ZAxis() const
{
    return (*this * CVector3f::skUnitZ);
}

CQuaternion CQuaternion::Inverse() const
{
    float fNorm = (w * w) + (x * x) + (y * y) + (z * z);

    if (fNorm > 0.f)
    {
        float fInvNorm = 1.f / fNorm;
        return CQuaternion( w * fInvNorm, -x * fInvNorm, -y * fInvNorm, -z * fInvNorm);
    }
    else
        return CQuaternion::skZero;
}

CVector3f CQuaternion::ToEuler() const
{
    // There is more than one way to do this conversion, based on rotation order.
    // But since we only care about the rotation order used in Retro games, which is consistent,
    // we can just have a single conversion function. Handy!
    // https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles

    float ex = atan2f(2 * (w*x + y*z), 1 - (2 * (Math::Pow(x,2) + Math::Pow(y,2))));
    float ey = asinf(2 * (w*y - z*x));
    float ez = atan2f(2 * (w*z + x*y), 1 - (2 * (Math::Pow(y,2) + Math::Pow(z,2))));
    return CVector3f(Math::RadiansToDegrees(ex), Math::RadiansToDegrees(ey), Math::RadiansToDegrees(ez));
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
    out.w = (-x * other.x) - (y * other.y) - (z * other.z) + (w * other.w);
    out.x = ( x * other.w) + (y * other.z) - (z * other.y) + (w * other.x);
    out.y = (-x * other.z) + (y * other.w) + (z * other.x) + (w * other.y);
    out.z = ( x * other.y) - (y * other.x) + (z * other.w) + (w * other.z);
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

    CQuaternion x = CQuaternion::FromAxisAngle(Math::DegreesToRadians(euler.x), CVector3f(1,0,0));
    CQuaternion y = CQuaternion::FromAxisAngle(Math::DegreesToRadians(euler.y), CVector3f(0,1,0));
    CQuaternion z = CQuaternion::FromAxisAngle(Math::DegreesToRadians(euler.z), CVector3f(0,0,1));
    CQuaternion quat = z * y * x;

    return quat;
}

CQuaternion CQuaternion::FromAxisAngle(float angle, CVector3f axis)
{
    CQuaternion quat;
    axis = axis.Normalized();

    float sa = sinf(angle / 2);
    quat.w = cosf(angle / 2);
    quat.x = axis.x * sa;
    quat.y = axis.y * sa;
    quat.z = axis.z * sa;
    return quat;

}

CQuaternion CQuaternion::FromRotationMatrix(const CMatrix4f& RotMtx)
{
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    CQuaternion out;
    float trace = RotMtx[0][0] + RotMtx[1][1] + RotMtx[2][2];

    if (trace > 0.f)
    {
      float s = Math::Sqrt(trace + 1.f) * 2; // s = 4*w
      out.w = 0.25f * s;
      out.x = (RotMtx[2][1] - RotMtx[1][2]) / s;
      out.y = (RotMtx[0][2] - RotMtx[2][0]) / s;
      out.z = (RotMtx[1][0] - RotMtx[0][1]) / s;
    }

    else if ((RotMtx[0][0] > RotMtx[1][1]) && (RotMtx[0][0] > RotMtx[2][2]))
    {
      float s = Math::Sqrt(1.f + RotMtx[0][0] - RotMtx[1][1] - RotMtx[2][2]) * 2; // s = 4*x
      out.w = (RotMtx[2][1] - RotMtx[1][2]) / s;
      out.x = 0.25f * s;
      out.y = (RotMtx[0][1] + RotMtx[1][0]) / s;
      out.z = (RotMtx[0][2] + RotMtx[2][0]) / s;
    }

    else if (RotMtx[1][1] > RotMtx[2][2]) {
      float s = Math::Sqrt(1.f + RotMtx[1][1] - RotMtx[0][0] - RotMtx[2][2]) * 2; // s = 4*y
      out.w = (RotMtx[0][2] - RotMtx[2][0]) / s;
      out.x = (RotMtx[0][1] + RotMtx[1][0]) / s;
      out.y = 0.25f * s;
      out.z = (RotMtx[1][2] + RotMtx[2][1]) / s;
    }

    else {
      float s = Math::Sqrt(1.f + RotMtx[2][2] - RotMtx[0][0] - RotMtx[1][1]) * 2; // S=4*qz
      out.w = (RotMtx[1][0] - RotMtx[0][1]) / s;
      out.x = (RotMtx[0][2] + RotMtx[2][0]) / s;
      out.y = (RotMtx[1][2] + RotMtx[2][1]) / s;
      out.z = 0.25f * s;
    }

    return out;
}

CQuaternion CQuaternion::FromAxes(const CVector3f& X, const CVector3f& Y, const CVector3f& Z)
{
    CMatrix4f RotMtx(X.x, Y.x, Z.x, 0.f,
                     X.y, Y.y, Z.y, 0.f,
                     X.z, Y.z, Z.z, 0.f,
                     0.f, 0.f, 0.f, 1.f);

    return CQuaternion::FromRotationMatrix(RotMtx);
}

CQuaternion CQuaternion::skIdentity = CQuaternion(1.f, 0.f, 0.f, 0.f);
CQuaternion CQuaternion::skZero = CQuaternion(0.f, 0.f, 0.f, 0.f);
