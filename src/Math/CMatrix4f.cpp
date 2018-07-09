#include "CMatrix4f.h"
#include "CQuaternion.h"
#include "CVector3f.h"
#include "CVector4f.h"
#include "CTransform4f.h"

CMatrix4f::CMatrix4f()
{
}

CMatrix4f::CMatrix4f(float Diagonal)
{
    *this = skZero;
    m[0][0] = Diagonal;
    m[1][1] = Diagonal;
    m[2][2] = Diagonal;
    m[3][3] = Diagonal;
}

CMatrix4f::CMatrix4f(float m00, float m01, float m02, float m03,
                     float m10, float m11, float m12, float m13,
                     float m20, float m21, float m22, float m23,
                     float m30, float m31, float m32, float m33)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
}

// ************ MATH ************
CMatrix4f CMatrix4f::Transpose() const
{
    return CMatrix4f(m[0][0], m[1][0], m[2][0], m[3][0],
                     m[0][1], m[1][1], m[2][1], m[3][1],
                     m[0][2], m[1][2], m[2][2], m[3][2],
                     m[0][3], m[1][3], m[2][3], m[3][3]);
}

CMatrix4f CMatrix4f::Inverse() const
{
    // Copied from Ogre.
    // todo after developing a better understanding of the math - rewrite
    float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
    float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
    float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
    float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    float d00 = t00 * invDet;
    float d10 = t10 * invDet;
    float d20 = t20 * invDet;
    float d30 = t30 * invDet;

    float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    return CMatrix4f(
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33);
}

float CMatrix4f::Determinant() const
{
    float AA = m[1][1] * ((m[2][2] * m[3][3]) - (m[2][3] * m[3][2]));
    float AB = m[1][2] * ((m[2][1] * m[3][3]) - (m[2][3] * m[3][1]));
    float AC = m[1][3] * ((m[2][1] * m[3][2]) - (m[2][2] * m[3][1]));
    float A = m[0][0] * (AA - AB + AC);

    float BA = m[1][0] * ((m[2][2] * m[3][3]) - (m[2][3] * m[3][2]));
    float BB = m[1][2] * ((m[2][0] * m[3][3]) - (m[2][3] * m[3][0]));
    float BC = m[1][3] * ((m[2][0] * m[3][2]) - (m[2][2] * m[3][0]));
    float B = m[0][1] * (BA - BB + BC);

    float CA = m[1][0] * ((m[2][1] * m[3][3]) - (m[2][3] * m[3][1]));
    float CB = m[1][1] * ((m[2][0] * m[3][3]) - (m[2][3] * m[3][0]));
    float CC = m[1][3] * ((m[2][0] * m[3][1]) - (m[2][1] * m[3][0]));
    float C = m[0][2] * (CA - CB + CC);

    float DA = m[1][0] * ((m[2][1] * m[3][2]) - (m[2][2] * m[3][1]));
    float DB = m[1][1] * ((m[2][0] * m[3][2]) - (m[2][2] * m[3][0]));
    float DC = m[1][2] * ((m[2][0] * m[3][1]) - (m[2][1] * m[3][0]));
    float D = m[0][3] * (DA - DB + DC);

    return (A - B + C - D);
}

// ************ OPERATORS ************
CVector3f CMatrix4f::operator*(const CVector3f& rkVec) const
{
    // For vec3 multiplication, the vector w component is considered to be 1.0
    CVector3f out;
    float w = (m[3][0] * rkVec.X) + (m[3][1] * rkVec.Y) + (m[3][2] * rkVec.Z) + m[3][3];
    out.X = ((m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + m[0][3]) / w;
    out.Y = ((m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + m[1][3]) / w;
    out.Z = ((m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + m[1][3]) / w;
    return out;
}

CVector4f CMatrix4f::operator*(const CVector4f& rkVec) const
{
    CVector4f out;
    out.X = (m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + (m[0][3] * rkVec.W);
    out.Y = (m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + (m[1][3] * rkVec.W);
    out.Z = (m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + (m[2][3] * rkVec.W);
    out.W = (m[3][0] * rkVec.X) + (m[3][1] * rkVec.Y) + (m[3][2] * rkVec.Z) + (m[3][3] * rkVec.W);
    return out;
}

CMatrix4f CMatrix4f::operator*(const CTransform4f& rkMtx) const
{
    // CTransform4f is a 3x4 matrix with an implicit fourth row of {0, 0, 0, 1}
    CMatrix4f out;
    out[0][0] = (m[0][0] * rkMtx[0][0]) + (m[0][1] * rkMtx[1][0]) + (m[0][2] * rkMtx[2][0]);
    out[0][1] = (m[0][0] * rkMtx[0][1]) + (m[0][1] * rkMtx[1][1]) + (m[0][2] * rkMtx[2][1]);
    out[0][2] = (m[0][0] * rkMtx[0][2]) + (m[0][1] * rkMtx[1][2]) + (m[0][2] * rkMtx[2][2]);
    out[0][3] = (m[0][0] * rkMtx[0][3]) + (m[0][1] * rkMtx[1][3]) + (m[0][2] * rkMtx[2][3]) + m[0][3];
    out[1][0] = (m[1][0] * rkMtx[0][0]) + (m[1][1] * rkMtx[1][0]) + (m[1][2] * rkMtx[2][0]);
    out[1][1] = (m[1][0] * rkMtx[0][1]) + (m[1][1] * rkMtx[1][1]) + (m[1][2] * rkMtx[2][1]);
    out[1][2] = (m[1][0] * rkMtx[0][2]) + (m[1][1] * rkMtx[1][2]) + (m[1][2] * rkMtx[2][2]);
    out[1][3] = (m[1][0] * rkMtx[0][3]) + (m[1][1] * rkMtx[1][3]) + (m[1][2] * rkMtx[2][3]) + m[1][3];
    out[2][0] = (m[2][0] * rkMtx[0][0]) + (m[2][1] * rkMtx[1][0]) + (m[2][2] * rkMtx[2][0]);
    out[2][1] = (m[2][0] * rkMtx[0][1]) + (m[2][1] * rkMtx[1][1]) + (m[2][2] * rkMtx[2][1]);
    out[2][2] = (m[2][0] * rkMtx[0][2]) + (m[2][1] * rkMtx[1][2]) + (m[2][2] * rkMtx[2][2]);
    out[2][3] = (m[2][0] * rkMtx[0][3]) + (m[2][1] * rkMtx[1][3]) + (m[2][2] * rkMtx[2][3]) + m[2][3];
    out[3][0] = (m[3][0] * rkMtx[0][0]) + (m[3][1] * rkMtx[1][0]) + (m[3][2] * rkMtx[2][0]);
    out[3][1] = (m[3][0] * rkMtx[0][1]) + (m[3][1] * rkMtx[1][1]) + (m[3][2] * rkMtx[2][1]);
    out[3][2] = (m[3][0] * rkMtx[0][2]) + (m[3][1] * rkMtx[1][2]) + (m[3][2] * rkMtx[2][2]);
    out[3][3] = (m[3][0] * rkMtx[0][3]) + (m[3][1] * rkMtx[1][3]) + (m[3][2] * rkMtx[2][3]) + m[3][3];
    return out;
}

CMatrix4f CMatrix4f::operator*(const CMatrix4f& rkMtx) const
{
    CMatrix4f out;
    out[0][0] = (m[0][0] * rkMtx[0][0]) + (m[0][1] * rkMtx[1][0]) + (m[0][2] * rkMtx[2][0]) + (m[0][3] * rkMtx[3][0]);
    out[0][1] = (m[0][0] * rkMtx[0][1]) + (m[0][1] * rkMtx[1][1]) + (m[0][2] * rkMtx[2][1]) + (m[0][3] * rkMtx[3][1]);
    out[0][2] = (m[0][0] * rkMtx[0][2]) + (m[0][1] * rkMtx[1][2]) + (m[0][2] * rkMtx[2][2]) + (m[0][3] * rkMtx[3][2]);
    out[0][3] = (m[0][0] * rkMtx[0][3]) + (m[0][1] * rkMtx[1][3]) + (m[0][2] * rkMtx[2][3]) + (m[0][3] * rkMtx[3][3]);
    out[1][0] = (m[1][0] * rkMtx[0][0]) + (m[1][1] * rkMtx[1][0]) + (m[1][2] * rkMtx[2][0]) + (m[1][3] * rkMtx[3][0]);
    out[1][1] = (m[1][0] * rkMtx[0][1]) + (m[1][1] * rkMtx[1][1]) + (m[1][2] * rkMtx[2][1]) + (m[1][3] * rkMtx[3][1]);
    out[1][2] = (m[1][0] * rkMtx[0][2]) + (m[1][1] * rkMtx[1][2]) + (m[1][2] * rkMtx[2][2]) + (m[1][3] * rkMtx[3][2]);
    out[1][3] = (m[1][0] * rkMtx[0][3]) + (m[1][1] * rkMtx[1][3]) + (m[1][2] * rkMtx[2][3]) + (m[1][3] * rkMtx[3][3]);
    out[2][0] = (m[2][0] * rkMtx[0][0]) + (m[2][1] * rkMtx[1][0]) + (m[2][2] * rkMtx[2][0]) + (m[2][3] * rkMtx[3][0]);
    out[2][1] = (m[2][0] * rkMtx[0][1]) + (m[2][1] * rkMtx[1][1]) + (m[2][2] * rkMtx[2][1]) + (m[2][3] * rkMtx[3][1]);
    out[2][2] = (m[2][0] * rkMtx[0][2]) + (m[2][1] * rkMtx[1][2]) + (m[2][2] * rkMtx[2][2]) + (m[2][3] * rkMtx[3][2]);
    out[2][3] = (m[2][0] * rkMtx[0][3]) + (m[2][1] * rkMtx[1][3]) + (m[2][2] * rkMtx[2][3]) + (m[2][3] * rkMtx[3][3]);
    out[3][0] = (m[3][0] * rkMtx[0][0]) + (m[3][1] * rkMtx[1][0]) + (m[3][2] * rkMtx[2][0]) + (m[3][3] * rkMtx[3][0]);
    out[3][1] = (m[3][0] * rkMtx[0][1]) + (m[3][1] * rkMtx[1][1]) + (m[3][2] * rkMtx[2][1]) + (m[3][3] * rkMtx[3][1]);
    out[3][2] = (m[3][0] * rkMtx[0][2]) + (m[3][1] * rkMtx[1][2]) + (m[3][2] * rkMtx[2][2]) + (m[3][3] * rkMtx[3][2]);
    out[3][3] = (m[3][0] * rkMtx[0][3]) + (m[3][1] * rkMtx[1][3]) + (m[3][2] * rkMtx[2][3]) + (m[3][3] * rkMtx[3][3]);
    return out;
}

// ************ CONSTANT ************
const CMatrix4f CMatrix4f::skZero(0.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f);

const CMatrix4f CMatrix4f::skIdentity(1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      0.0f, 0.0f, 1.0f, 0.0f,
                                      0.0f, 0.0f, 0.0f, 1.0f);
