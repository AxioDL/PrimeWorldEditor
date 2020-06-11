#ifndef CMAYASPLINE_H
#define CMAYASPLINE_H

#include <Common/Common.h>
#include <Common/Math/CVector2f.h>
#include <vector>

// These classes based off Metroid Prime 2's CMayaSpline implementation
class CMayaSplineKnot
{
public:
    float Time;
    float Amplitude;

    mutable uint32 Flags;
    mutable CVector2f CachedTangentA;
    mutable CVector2f CachedTangentB;

    void GetTangents(const CMayaSplineKnot* pkPrev, const CMayaSplineKnot* pkNext, CVector2f& OutTangentA, CVector2f& OutTangentB) const;
    void CalculateTangents(const CMayaSplineKnot* pkPrev, const CMayaSplineKnot* pkNext) const;
};

class CMayaSpline
{
    uint mPreInfinity = 0; // 0x00
    uint mPostInfinity = 0; // 0x04
    std::vector<CMayaSplineKnot> mKnots; // 0x08, 0x0C, 0x10
    uint mClampMode = 0; // 0x14 - clamp mode
    float mMinAmplitude = 0.0f; // 0x18
    float mMaxAmplitude = 0.0f; // 0x1C

    mutable int mCachedKnotIndex = 0; // 0x20
    mutable int mUnknown1 = 0; // 0x24
    mutable uint8 mDirtyFlags = 0; // 0x28
    mutable float mCachedMinTime = 0.0f; // 0x2C
    mutable float mCachedHermiteCoefficients[4] = {}; // 0x30, 0x34, 0x38, 0x3C

public:
    CMayaSpline() = default;
    uint GetKnotCount() const;
    const std::vector<CMayaSplineKnot>& GetKnots() const;
    float GetMinTime() const;
    float GetMaxTime() const;
    float GetDuration() const;

    float EvaluateAt(float Time) const;
    float EvaluateAtUnclamped(float Time) const;
    float EvaluateInfinities(float Time, bool Pre) const;
    float EvaluateHermite(float Time) const;
    bool FindKnot(float Time, int& OutKnotIndex) const;
    void FindControlPoints(int KnotIndex, std::vector<CVector2f>& Out) const;
    void CalculateHermiteCoefficients(const std::vector<CVector2f>& kControlPoints, float* pOutCoefs) const;
};

#endif // CMAYASPLINE_H
