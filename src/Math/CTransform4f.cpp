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

CTransform4f::CTransform4f(const CMatrix4f& rkMtx)
{
    for (int iRow = 0; iRow < 3; iRow++)
        for (int iCol = 0; iCol < 4; iCol++)
            m[iRow][iCol] = rkMtx[iRow][iCol];

    SetupRow4();
}

CTransform4f::CTransform4f(IInputStream& rInput)
{
    for (int iVal = 0; iVal < 12; iVal++)
        _m[iVal] = rInput.ReadFloat();

    SetupRow4();
}

CTransform4f::CTransform4f(float Diagonal)
{
    *this = skZero;
    m[0][0] = Diagonal;
    m[1][1] = Diagonal;
    m[2][2] = Diagonal;
    m[3][3] = 1.f;
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
    SetupRow4();
}

CTransform4f::CTransform4f(CVector3f Position, CQuaternion Rotation, CVector3f Scale)
{
    *this = skIdentity;
    this->Scale(Scale);
    Rotate(Rotation);
    Translate(Position);
}

CTransform4f::CTransform4f(CVector3f Position, CVector3f Rotation, CVector3f Scale)
{
    *this = skIdentity;
    this->Scale(Scale);
    Rotate(Rotation);
    Translate(Position);
}

void CTransform4f::Serialize(IArchive& rOut)
{
    rOut << SerialParameter("Row0Col0", m[0][0]) << SerialParameter("Row0Col1", m[0][1]) << SerialParameter("Row0Col2", m[0][2]) << SerialParameter("Row0Col3", m[0][3])
         << SerialParameter("Row1Col0", m[1][0]) << SerialParameter("Row1Col1", m[1][1]) << SerialParameter("Row1Col2", m[1][2]) << SerialParameter("Row1Col3", m[1][3])
         << SerialParameter("Row2Col0", m[2][0]) << SerialParameter("Row2Col1", m[2][1]) << SerialParameter("Row2Col2", m[2][2]) << SerialParameter("Row2Col3", m[2][3]);
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

void CTransform4f::SetIdentity()
{
    *this = skIdentity;
}

void CTransform4f::ZeroTranslation()
{
    m[0][3] = 0.f;
    m[1][3] = 0.f;
    m[2][3] = 0.f;
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

CTransform4f CTransform4f::QuickInverse() const
{
    CTransform4f Out;
    Out[0][0] = m[0][0];
    Out[0][1] = m[1][0];
    Out[0][2] = m[2][0];
    Out[0][3] = -((m[0][0] * m[0][3]) + (m[1][0] * m[1][3]) + (m[2][0] * m[2][3]));
    Out[1][0] = m[0][1];
    Out[1][1] = m[1][1];
    Out[1][2] = m[2][1];
    Out[1][3] = -((m[0][1] * m[0][3]) + (m[1][1] * m[1][3]) + (m[2][1] * m[2][3]));
    Out[2][0] = m[0][2];
    Out[2][1] = m[1][2];
    Out[2][2] = m[2][2];
    Out[2][3] = -((m[0][2] * m[0][3]) + (m[1][2] * m[1][3]) + (m[2][2] * m[2][3]));
    return Out;
}

CTransform4f CTransform4f::NoTranslation() const
{
    return CTransform4f(m[0][0], m[0][1], m[0][2], 0.f,
                        m[1][0], m[1][1], m[1][2], 0.f,
                        m[2][0], m[2][1], m[2][2], 0.f);
}

CTransform4f CTransform4f::TranslationOnly() const
{
    return CTransform4f(1.f, 0.f, 0.f, m[0][3],
                        0.f, 1.f, 0.f, m[1][3],
                        0.f, 0.f, 1.f, m[2][3]);
}

CTransform4f CTransform4f::RotationOnly() const
{
    return Inverse().Transpose();
}

CVector3f CTransform4f::ExtractTranslation() const
{
    return CVector3f(m[0][3], m[1][3], m[2][3]);
}

CQuaternion CTransform4f::ExtractRotation() const
{
    // todo: there's probably a faster way to do this...
    return CQuaternion::FromRotationMatrix(Inverse().Transpose());
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
    CVector3f Out;
    Out.X = (m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + (m[0][3]);
    Out.Y = (m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + (m[1][3]);
    Out.Z = (m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + (m[2][3]);
    return Out;
}

CVector4f CTransform4f::operator*(const CVector4f& rkVec) const
{
    CVector4f Out;
    Out.X = (m[0][0] * rkVec.X) + (m[0][1] * rkVec.Y) + (m[0][2] * rkVec.Z) + (m[0][3] * rkVec.W);
    Out.Y = (m[1][0] * rkVec.X) + (m[1][1] * rkVec.Y) + (m[1][2] * rkVec.Z) + (m[1][3] * rkVec.W);
    Out.Z = (m[2][0] * rkVec.X) + (m[2][1] * rkVec.Y) + (m[2][2] * rkVec.Z) + (m[2][3] * rkVec.W);
    Out.W = rkVec.W;
    return Out;
}

CQuaternion CTransform4f::operator*(const CQuaternion& rkQuat) const
{
    return ExtractRotation() * rkQuat;
}

CTransform4f CTransform4f::operator*(const CTransform4f& rkMtx) const
{
    CTransform4f Out;
    Out[0][0] = (m[0][0] * rkMtx[0][0]) + (m[0][1] * rkMtx[1][0]) + (m[0][2] * rkMtx[2][0]);
    Out[0][1] = (m[0][0] * rkMtx[0][1]) + (m[0][1] * rkMtx[1][1]) + (m[0][2] * rkMtx[2][1]);
    Out[0][2] = (m[0][0] * rkMtx[0][2]) + (m[0][1] * rkMtx[1][2]) + (m[0][2] * rkMtx[2][2]);
    Out[0][3] = (m[0][0] * rkMtx[0][3]) + (m[0][1] * rkMtx[1][3]) + (m[0][2] * rkMtx[2][3]) + m[0][3];
    Out[1][0] = (m[1][0] * rkMtx[0][0]) + (m[1][1] * rkMtx[1][0]) + (m[1][2] * rkMtx[2][0]);
    Out[1][1] = (m[1][0] * rkMtx[0][1]) + (m[1][1] * rkMtx[1][1]) + (m[1][2] * rkMtx[2][1]);
    Out[1][2] = (m[1][0] * rkMtx[0][2]) + (m[1][1] * rkMtx[1][2]) + (m[1][2] * rkMtx[2][2]);
    Out[1][3] = (m[1][0] * rkMtx[0][3]) + (m[1][1] * rkMtx[1][3]) + (m[1][2] * rkMtx[2][3]) + m[1][3];
    Out[2][0] = (m[2][0] * rkMtx[0][0]) + (m[2][1] * rkMtx[1][0]) + (m[2][2] * rkMtx[2][0]);
    Out[2][1] = (m[2][0] * rkMtx[0][1]) + (m[2][1] * rkMtx[1][1]) + (m[2][2] * rkMtx[2][1]);
    Out[2][2] = (m[2][0] * rkMtx[0][2]) + (m[2][1] * rkMtx[1][2]) + (m[2][2] * rkMtx[2][2]);
    Out[2][3] = (m[2][0] * rkMtx[0][3]) + (m[2][1] * rkMtx[1][3]) + (m[2][2] * rkMtx[2][3]) + m[2][3];
    return Out;
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

// ************ STATIC ************
CTransform4f CTransform4f::TranslationMatrix(CVector3f Translation)
{
    CTransform4f Out = skIdentity;
    Out[0][3] = Translation.X;
    Out[1][3] = Translation.Y;
    Out[2][3] = Translation.Z;
    return Out;
}

CTransform4f CTransform4f::RotationMatrix(CQuaternion Rotation)
{
    CTransform4f Out = skIdentity;
    float X = Rotation.X;
    float Y = Rotation.Y;
    float Z = Rotation.Z;
    float W = Rotation.W;
    float X2 = X * X;
    float Y2 = Y * Y;
    float Z2 = Z * Z;

    Out[0][0] = 1.0f - (2 * Y2) - (2 * Z2);
    Out[0][1] = (2 * X * Y) - (2 * Z * W);
    Out[0][2] = (2 * X * Z) + (2 * Y * W);
    Out[1][0] = (2 * X * Y) + (2 * Z * W);
    Out[1][1] = 1.0f - (2 * X2) - (2 * Z2);
    Out[1][2] = (2 * Y * Z) - (2 * X * W);
    Out[2][0] = (2 * X * Z) - (2 * Y * W);
    Out[2][1] = (2 * Y * Z) + (2 * X * W);
    Out[2][2] = 1.0f - (2 * X2) - (2 * Y2);
    return Out;
}

CTransform4f CTransform4f::ScaleMatrix(CVector3f Scale)
{
    CTransform4f Out = skIdentity;
    Out[0][0] = Scale.X;
    Out[1][1] = Scale.Y;
    Out[2][2] = Scale.Z;
    return Out;
}

// ************ CONSTANTS ************
const CTransform4f CTransform4f::skIdentity(1.0f, 0.0f, 0.0f, 0.0f,
                                            0.0f, 1.0f, 0.0f, 0.0f,
                                            0.0f, 0.0f, 1.0f, 0.0f);

const CTransform4f CTransform4f::skZero(0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f);
