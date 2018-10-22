#ifndef CMATRIX4_H
#define CMATRIX4_H

class CQuaternion;
class CVector3f;
class CVector4f;
class CTransform4f;

class CMatrix4f
{
protected:
    union
    {
        float m[4][4];
        float _m[16];
    };

public:
    CMatrix4f();
    CMatrix4f(float Diagonal);
    CMatrix4f(float m00, float m01, float m02, float m03,
              float m10, float m11, float m12, float m13,
              float m20, float m21, float m22, float m23,
              float m30, float m31, float m32, float m33);

    // Math
    CMatrix4f Transpose() const;
    CMatrix4f Inverse() const;
    float Determinant() const;

    // Operators
    inline float* operator[](long Index);
    inline const float* operator[](long Index) const;
    CVector3f operator*(const CVector3f& rkVec) const;
    CVector4f operator*(const CVector4f& rkVec) const;
    CQuaternion operator*(const CQuaternion& rkQuat) const;
    CMatrix4f operator*(const CTransform4f& rkMtx) const;
    CMatrix4f operator*(const CMatrix4f& rkMtx) const;

    // Constants
    static const CMatrix4f skZero;
    static const CMatrix4f skIdentity;
};

inline float* CMatrix4f::operator[](long Index)
{
    return m[Index];
}

inline const float* CMatrix4f::operator[](long Index) const
{
    return m[Index];
}

#endif // CMATRIX4_H
