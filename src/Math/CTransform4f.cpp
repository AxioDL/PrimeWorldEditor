#include "CTransform4f.h"
#include "CVector3f.h"
#include "CVector4f.h"
#include "CQuaternion.h"
#include "CMatrix4f.h"

// ************ CONSTRUCTRS ************
CTransform4f::CTransform4f()
{
    *this = skIdentity;
}

CTransform4f::CTransform4f(IInputStream& rInput)
{
    for (int iVal = 0; iVal < 12; iVal++)
        _m[iVal] = rInput.ReadFloat();
}

CTransform4f::CTransform4f(float Diagonal)
{
    *this = skZero;
    m[0][0] = Diagonal;
    m[1][1] = Diagonal;
    m[2][2] = Diagonal;
}

CTransform4f::CTransform4f(float m00, float m01, float m02, float m03,
                           float m10, float m11, float m12, float m13,
                           float m20, float m21, float m22, float m23)
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
}

CTransform4f::CTransform4f(CVector3f Position, CQuaternion Rotation, CVector3f Scale)
{
    *this = skIdentity;
    Translate(Position);
    Rotate(Rotation);
    this->Scale(Scale);
}

CTransform4f::CTransform4f(CVector3f, CVector3f, CVector3f)
{
}

void CTransform4f::Write(IOutputStream& rOut)
{
    for (int iFlt = 0; iFlt < 12; iFlt++)
        rOut.WriteFloat(_m[iFlt]);
}

// ************ MATH ************
void CTransform4f::Translate(CVector3f Translation)
{
    CTransform4f TranslateMtx = CTransform4f::TranslationMatrix(Translation);
    *this = TranslateMtx * *this;
}

void CTransform4f::Translate(float XTrans, float YTrans, float ZTrans)
{
    Translate(CVector3f(XTrans, YTrans, ZTrans));
}

void CTransform4f::Rotate(CQuaternion Rotation)
{
    CTransform4f RotateMtx = CTransform4f::RotationMatrix(Rotation);
    *this = RotateMtx * *this;
}

void CTransform4f::Rotate(CVector3f Rotation)
{
    CQuaternion quat = CQuaternion::FromEuler(Rotation);
    Rotate(quat);
}

void CTransform4f::Rotate(float XRot, float YRot, float ZRot)
{
    Rotate(CVector3f(XRot, YRot, ZRot));
}

void CTransform4f::Scale(CVector3f Scale)
{
    CTransform4f ScaleMtx = CTransform4f::ScaleMatrix(Scale);
    *this = ScaleMtx * *this;
}

void CTransform4f::Scale(float XScale, float YScale, float ZScale)
{
    Scale(CVector3f(XScale, YScale, ZScale));
}

CTransform4f CTransform4f::MultiplyIgnoreTranslation(const CTransform4f& rkMtx) const
{
    CTransform4f Out;
    Out[0][0] = (m[0][0] * rkMtx[0][0]) + (m[0][1] * rkMtx[1][0]) + (m[0][2] * rkMtx[2][0]);
    Out[0][1] = (m[0][0] * rkMtx[0][1]) + (m[0][1] * rkMtx[1][1]) + (m[0][2] * rkMtx[2][1]);
    Out[0][2] = (m[0][0] * rkMtx[0][2]) + (m[0][1] * rkMtx[1][2]) + (m[0][2] * rkMtx[2][2]);
    Out[1][0] = (m[1][0] * rkMtx[0][0]) + (m[1][1] * rkMtx[1][0]) + (m[1][2] * rkMtx[2][0]);
    Out[1][1] = (m[1][0] * rkMtx[0][1]) + (m[1][1] * rkMtx[1][1]) + (m[1][2] * rkMtx[2][1]);
    Out[1][2] = (m[1][0] * rkMtx[0][2]) + (m[1][1] * rkMtx[1][2]) + (m[1][2] * rkMtx[2][2]);
    Out[2][0] = (m[2][0] * rkMtx[0][0]) + (m[2][1] * rkMtx[1][0]) + (m[2][2] * rkMtx[2][0]);
    Out[2][1] = (m[2][0] * rkMtx[0][1]) + (m[2][1] * rkMtx[1][1]) + (m[2][2] * rkMtx[2][1]);
    Out[2][2] = (m[2][0] * rkMtx[0][2]) + (m[2][1] * rkMtx[1][2]) + (m[2][2] * rkMtx[2][2]);
    Out[0][3] = 0.f;
    Out[1][3] = 0.f;
    Out[2][3] = 0.f;
    return Out;
}

CTransform4f CTransform4f::Inverse() const
{
    // This uses CMatrix4f because I suck at math
    // todo - rewrite this without using CMatrix4f
    CMatrix4f Mat4 = ToMatrix4f().Inverse();
    CTransform4f Out;
    memcpy(&Out[0][0], &Mat4[0][0], sizeof(CTransform4f));
    return Out;
}

CTransform4f CTransform4f::QuickInverse() const
{
    CTransform4f out;
    out[0][0] = m[0][0];
    out[0][1] = m[1][0];
    out[0][2] = m[2][0];
    out[0][3] = -((m[0][0] * m[0][3]) + (m[1][0] * m[1][3]) + (m[2][0] * m[2][3]));
    out[1][0] = m[0][1];
    out[1][1] = m[1][1];
    out[1][2] = m[2][1];
    out[1][3] = -((m[0][1] * m[0][3]) + (m[1][1] * m[1][3]) + (m[2][1] * m[2][3]));
    out[2][0] = m[0][2];
    out[2][1] = m[1][2];
    out[2][2] = m[2][2];
    out[2][3] = -((m[0][2] * m[0][3]) + (m[1][2] * m[1][3]) + (m[2][2] * m[2][3]));
    return out;
}

CTransform4f CTransform4f::NoTranslation() const
{
    return CTransform4f(m[0][0], m[0][1], m[0][2], 0.f,
                        m[1][0], m[1][1], m[1][2], 0.f,
                        m[2][0], m[2][1], m[2][2], 0.f);
}

CTransform4f CTransform4f::RotationOnly() const
{
    return CTransform4f::FromMatrix4f(Inverse().ToMatrix4f().Transpose());
}

// ************ OPERATORS ************
float* CTransform4f::operator[](long Index)
{
    return m[Index];
}

const float* CTransform4f::operator[](long Index) const
{
    return m[Index];
}

CVector3f CTransform4f::operator*(const CVector3f& rkVec) const
{
    CVector3f out;
    out.X = (m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + (m[0][3]);
    out.Y = (m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + (m[1][3]);
    out.Z = (m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + (m[2][3]);
    return out;
}

CVector4f CTransform4f::operator*(const CVector4f& rkVec) const
{
    CVector4f out;
    out.X = (m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + (m[0][3] * rkVec.W);
    out.Y = (m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + (m[1][3] * rkVec.W);
    out.Z = (m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + (m[2][3] * rkVec.W);
    out.W = rkVec.W;
    return out;
}

CTransform4f CTransform4f::operator*(const CTransform4f& rkMtx) const
{
    CTransform4f out;
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
    return out;
}

void CTransform4f::operator*=(const CTransform4f& rkMtx)
{
    *this = *this * rkMtx;
}

bool CTransform4f::operator==(const CTransform4f& rkMtx) const
{
    return ((m[0][0] == rkMtx[0][0]) &&
            (m[0][1] == rkMtx[0][1]) &&
            (m[0][2] == rkMtx[0][2]) &&
            (m[0][3] == rkMtx[0][3]) &&
            (m[1][0] == rkMtx[1][0]) &&
            (m[1][1] == rkMtx[1][1]) &&
            (m[1][2] == rkMtx[1][2]) &&
            (m[1][3] == rkMtx[1][3]) &&
            (m[2][0] == rkMtx[2][0]) &&
            (m[2][1] == rkMtx[2][1]) &&
            (m[2][2] == rkMtx[2][2]) &&
            (m[2][3] == rkMtx[2][3]));
}

bool CTransform4f::operator!=(const CTransform4f& rkMtx) const
{
    return (!(*this == rkMtx));
}

// ************ CONVERSION ************
CMatrix4f CTransform4f::ToMatrix4f() const
{
    return CMatrix4f(m[0][0], m[0][1], m[0][2], m[0][3],
                     m[1][0], m[1][1], m[1][2], m[1][3],
                     m[2][0], m[2][1], m[2][2], m[2][3],
                         0.f,     0.f,     0.f,     1.f);
}

// ************ STATIC ************
CTransform4f CTransform4f::TranslationMatrix(CVector3f Translation)
{
    CTransform4f out = skIdentity;
    out[0][3] = Translation.X;
    out[1][3] = Translation.Y;
    out[2][3] = Translation.Z;
    return out;
}

CTransform4f CTransform4f::RotationMatrix(CQuaternion Rotation)
{
    CTransform4f out = skIdentity;
    float x = Rotation.X;
    float y = Rotation.Y;
    float z = Rotation.Z;
    float w = Rotation.W;
    float x2 = x * x;
    float y2 = y * y;
    float z2 = z * z;

    out[0][0] = 1.0f - (2 * y2) - (2 * z2);
    out[0][1] = (2 * x * y) - (2 * z * w);
    out[0][2] = (2 * x * z) + (2 * y * w);
    out[1][0] = (2 * x * y) + (2 * z * w);
    out[1][1] = 1.0f - (2 * x2) - (2 * z2);
    out[1][2] = (2 * y * z) - (2 * x * w);
    out[2][0] = (2 * x * z) - (2 * y * w);
    out[2][1] = (2 * y * z) + (2 * x * w);
    out[2][2] = 1.0f - (2 * x2) - (2 * y2);
    return out;
}

    CTransform4f CTransform4f::ScaleMatrix(CVector3f Scale)
    {
        CTransform4f out = skIdentity;
        out[0][0] = Scale.X;
        out[1][1] = Scale.Y;
        out[2][2] = Scale.Z;
        return out;
    }

CTransform4f CTransform4f::FromMatrix4f(const CMatrix4f& rkMtx)
{
    CTransform4f Out;
    for (int iRow = 0; iRow < 3; iRow++)
        for (int iCol = 0; iCol < 4; iCol++)
            Out[iRow][iCol] = rkMtx[iRow][iCol];
    return Out;
}

CTransform4f CTransform4f::FromGlmMat4(const glm::mat4& rkMtx)
{
    CTransform4f Out;
    for (int iRow = 0; iRow < 3; iRow++)
        for (int iCol = 0; iCol < 4; iCol++)
            Out[iRow][iCol] = rkMtx[iRow][iCol];
    return Out;
}

static CTransform4f FromGlmMat4(const glm::mat4&)
{
}

// ************ CONSTANTS ************
const CTransform4f CTransform4f::skIdentity(1.0f, 0.0f, 0.0f, 0.0f,
                                            0.0f, 1.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 1.0f, 0.0f);

const CTransform4f CTransform4f::skZero(0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f);
