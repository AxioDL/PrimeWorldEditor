#ifndef CTRANSFORM4F_H
#define CTRANSFORM4F_H

#include <Common/FileIO.h>
#include <Common/Serialization/IArchive.h>
#include "CMatrix4f.h"

class CVector3f;
class CVector4f;
class CQuaternion;

class CTransform4f : public CMatrix4f
{
public:
    CTransform4f();
    CTransform4f(const CMatrix4f& rkMtx);
    CTransform4f(IInputStream& rInput);
    CTransform4f(float Diagonal);
    CTransform4f(float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23);
    CTransform4f(CVector3f Position, CQuaternion Rotation, CVector3f Scale);
    CTransform4f(CVector3f Position, CVector3f Rotation, CVector3f Scale);
    void Serialize(IArchive& rOut);
    void Write(IOutputStream& rOut);

    // Math
    void Translate(CVector3f Translation);
    void Translate(float XTrans, float YTrans, float ZTrans);
    void Rotate(CQuaternion Rotation);
    void Rotate(CVector3f Rotation);
    void Rotate(float XRot, float YRot, float ZRot);
    void Scale(CVector3f Scale);
    void Scale(float XScale, float YScale, float ZScale);
    void SetIdentity();
    void ZeroTranslation();
    CTransform4f MultiplyIgnoreTranslation(const CTransform4f& rkMtx) const;
    CTransform4f QuickInverse() const;
    CTransform4f NoTranslation() const;
    CTransform4f TranslationOnly() const;
    CTransform4f RotationOnly() const;

    CVector3f ExtractTranslation() const;
    CQuaternion ExtractRotation() const;

    // Static
    static CTransform4f TranslationMatrix(CVector3f Translation);
    static CTransform4f RotationMatrix(CQuaternion Rotation);
    static CTransform4f ScaleMatrix(CVector3f Scale);

    // Operators
    float* operator[](long Index);
    const float* operator[](long Index) const;
    CVector3f operator*(const CVector3f& rkVec) const;
    CVector4f operator*(const CVector4f& rkVec) const;
    CQuaternion operator*(const CQuaternion& rkQuat) const;
    CTransform4f operator*(const CTransform4f& rkMtx) const;
    void operator*=(const CTransform4f& rkMtx);
    bool operator==(const CTransform4f& rkMtx) const;
    bool operator!=(const CTransform4f& rkMtx) const;

    // Constant
    static const CTransform4f skIdentity;
    static const CTransform4f skZero;

    // Protected Utility
protected:
    inline void SetupRow4()
    {
        m[3][0] = 0.f;
        m[3][1] = 0.f;
        m[3][2] = 0.f;
        m[3][3] = 1.f;
    }
};

#endif // CTRANSFORM4F_H
