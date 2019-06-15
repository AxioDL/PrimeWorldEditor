#ifndef CMAYASPLINE_H
#define CMAYASPLINE_H

#include <Common/types.h>
#include <Math/CVector2f.h>
#include <vector>

// These classes based off Metroid Prime 2's CMayaSpline implementation
class CMayaSplineKnot
{
public:
    float Time;
    float Amplitude;

    mutable u32 Flags;
    mutable CVector2f CachedTangentA;
    mutable CVector2f CachedTangentB;

    void GetTangents(const CMayaSplineKnot *pkPrev, const CMayaSplineKnot *pkNext, CVector2f& rOutTangentA, CVector2f& rOutTangentB) const;
    void CalculateTangents(const CMayaSplineKnot *pkPrev, const CMayaSplineKnot *pkNext) const;
};

class CMayaSpline
{
    u32 mPreInfinity; // 0x00
    u32 mPostInfinity; // 0x04
    std::vector<CMayaSplineKnot> mKnots; // 0x08, 0x0C, 0x10
    u32 mClampMode; // 0x14 - clamp mode
    float mMinAmplitude; // 0x18
    float mMaxAmplitude; // 0x1C

    mutable s32 mCachedKnotIndex; // 0x20
    mutable s32 mUnknown1; // 0x24
    mutable u8 mDirtyFlags; // 0x28
    mutable float mCachedMinTime; // 0x2C
    mutable float mCachedHermiteCoefficients[4]; // 0x30, 0x34, 0x38, 0x3C

public:
    CMayaSpline() {}
    inline u32 GetKnotCount() const;
    inline const std::vector<CMayaSplineKnot>& GetKnots() const;
    float GetMinTime() const;
    float GetMaxTime() const;
    float GetDuration() const;

    float EvaluateAt(float Time) const;
    float EvaluateAtUnclamped(float Time) const;
    float EvaluateInfinities(float Time, bool Pre) const;
    float EvaluateHermite(float Time) const;
    bool FindKnot(float Time, int& rOutKnotIndex) const;
    void FindControlPoints(int KnotIndex, std::vector<CVector2f>& rOut) const;
    void CalculateHermiteCoefficients(const std::vector<CVector2f>& rkControlPoints, float *pOutCoefs) const;
};

#endif // CMAYASPLINE_H
