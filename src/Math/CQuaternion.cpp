#include "CQuaternion.h"
#include "CMatrix4f.h"
#include "MathUtil.h"
#include <cmath>
#include <math.h>

CQuaternion::CQuaternion()
    : W(0.f)
    , X(0.f)
    , Y(0.f)
    , Z(0.f)
{
}

CQuaternion::CQuaternion(float _W, float _X, float _Y, float _Z)
    : W(_W)
    , X(_X)
    , Y(_Y)
    , Z(_Z)
{
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
    float Norm = (W * W) + (X * X) + (Y * Y) + (Z * Z);

    if (Norm > 0.f)
    {
        float InvNorm = 1.f / Norm;
        return CQuaternion( W * InvNorm, -X * InvNorm, -Y * InvNorm, -Z * InvNorm);
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

    float EulerX = atan2f(2 * (W*X + Y*Z), 1 - (2 * (Math::Pow(X,2) + Math::Pow(Y,2))));
    float EulerY = asinf(2 * (W*Y - Z*X));
    float EulerZ = atan2f(2 * (W*Z + X*Y), 1 - (2 * (Math::Pow(Y,2) + Math::Pow(Z,2))));
    return CVector3f(Math::RadiansToDegrees(EulerX), Math::RadiansToDegrees(EulerY), Math::RadiansToDegrees(EulerZ));
}

// ************ OPERATORS ************
CVector3f CQuaternion::operator*(const CVector3f& rkVec) const
{
    CVector3f uv, uuv;
    CVector3f qvec(X, Y, Z);
    uv = qvec.Cross(rkVec);
    uuv = qvec.Cross(uv);
    uv *= (2.0f * W);
    uuv *= 2.0f;

    return rkVec + uv + uuv;
}

CQuaternion CQuaternion::operator*(const CQuaternion& rkOther) const
{
    CQuaternion out;
    out.W = (-X * rkOther.X) - (Y * rkOther.Y) - (Z * rkOther.Z) + (W * rkOther.W);
    out.X = ( X * rkOther.W) + (Y * rkOther.Z) - (Z * rkOther.Y) + (W * rkOther.X);
    out.Y = (-X * rkOther.Z) + (Y * rkOther.W) + (Z * rkOther.X) + (W * rkOther.Y);
    out.Z = ( X * rkOther.Y) - (Y * rkOther.X) + (Z * rkOther.W) + (W * rkOther.Z);
    return out;
}

void CQuaternion::operator *= (const CQuaternion& rkOther)
{
    *this = *this * rkOther;
}

// ************ STATIC ************
CQuaternion CQuaternion::FromEuler(CVector3f Euler)
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

    CQuaternion X = CQuaternion::FromAxisAngle(Math::DegreesToRadians(Euler.X), CVector3f(1,0,0));
    CQuaternion Y = CQuaternion::FromAxisAngle(Math::DegreesToRadians(Euler.Y), CVector3f(0,1,0));
    CQuaternion Z = CQuaternion::FromAxisAngle(Math::DegreesToRadians(Euler.Z), CVector3f(0,0,1));
    CQuaternion Quat = Z * Y * X;

    return Quat;
}

CQuaternion CQuaternion::FromAxisAngle(float Angle, CVector3f Axis)
{
    CQuaternion Quat;
    Axis = Axis.Normalized();

    float sa = sinf(Angle / 2);
    Quat.W = cosf(Angle / 2);
    Quat.X = Axis.X * sa;
    Quat.Y = Axis.Y * sa;
    Quat.Z = Axis.Z * sa;
    return Quat;

}

CQuaternion CQuaternion::FromRotationMatrix(const CMatrix4f& rkRotMtx)
{
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    CQuaternion Out;
    float Trace = rkRotMtx[0][0] + rkRotMtx[1][1] + rkRotMtx[2][2];

    if (Trace > 0.f)
    {
      float s = Math::Sqrt(Trace + 1.f) * 2; // s = 4*w
      Out.W = 0.25f * s;
      Out.X = (rkRotMtx[2][1] - rkRotMtx[1][2]) / s;
      Out.Y = (rkRotMtx[0][2] - rkRotMtx[2][0]) / s;
      Out.Z = (rkRotMtx[1][0] - rkRotMtx[0][1]) / s;
    }

    else if ((rkRotMtx[0][0] > rkRotMtx[1][1]) && (rkRotMtx[0][0] > rkRotMtx[2][2]))
    {
      float s = Math::Sqrt(1.f + rkRotMtx[0][0] - rkRotMtx[1][1] - rkRotMtx[2][2]) * 2; // s = 4*x
      Out.W = (rkRotMtx[2][1] - rkRotMtx[1][2]) / s;
      Out.X = 0.25f * s;
      Out.Y = (rkRotMtx[0][1] + rkRotMtx[1][0]) / s;
      Out.Z = (rkRotMtx[0][2] + rkRotMtx[2][0]) / s;
    }

    else if (rkRotMtx[1][1] > rkRotMtx[2][2]) {
      float s = Math::Sqrt(1.f + rkRotMtx[1][1] - rkRotMtx[0][0] - rkRotMtx[2][2]) * 2; // s = 4*y
      Out.W = (rkRotMtx[0][2] - rkRotMtx[2][0]) / s;
      Out.X = (rkRotMtx[0][1] + rkRotMtx[1][0]) / s;
      Out.Y = 0.25f * s;
      Out.Z = (rkRotMtx[1][2] + rkRotMtx[2][1]) / s;
    }

    else {
      float s = Math::Sqrt(1.f + rkRotMtx[2][2] - rkRotMtx[0][0] - rkRotMtx[1][1]) * 2; // S=4*qz
      Out.W = (rkRotMtx[1][0] - rkRotMtx[0][1]) / s;
      Out.X = (rkRotMtx[0][2] + rkRotMtx[2][0]) / s;
      Out.Y = (rkRotMtx[1][2] + rkRotMtx[2][1]) / s;
      Out.Z = 0.25f * s;
    }

    return Out;
}

CQuaternion CQuaternion::FromAxes(const CVector3f& rkX, const CVector3f& rkY, const CVector3f& rkZ)
{
    CMatrix4f RotMtx(rkX.X, rkY.X, rkZ.X, 0.f,
                     rkX.Y, rkY.Y, rkZ.Y, 0.f,
                     rkX.Z, rkY.Z, rkZ.Z, 0.f,
                     0.f, 0.f, 0.f, 1.f);

    return CQuaternion::FromRotationMatrix(RotMtx);
}

CQuaternion CQuaternion::skIdentity = CQuaternion(1.f, 0.f, 0.f, 0.f);
CQuaternion CQuaternion::skZero = CQuaternion(0.f, 0.f, 0.f, 0.f);
