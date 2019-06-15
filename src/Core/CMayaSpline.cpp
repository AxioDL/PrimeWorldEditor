#include "CMayaSpline.h"
#include <Math/MathUtil.h>

void ValidateTangent(CVector2f& rTangent)
{
    if (rTangent.X < 0.f) rTangent.X = 0.f;
    rTangent = rTangent.Normalized();

    if (rTangent.X == 0.f && rTangent.Y != 0.f)
    {
        float Mul = (rTangent.Y >= 0.f ? 1.f : -1.f);
        rTangent.X = 0.0001f;
        rTangent.Y = 5729578 * rTangent.X * Mul; // not sure where that number comes from!
    }
}

void CMayaSplineKnot::GetTangents(const CMayaSplineKnot *pkPrev, const CMayaSplineKnot *pkNext, CVector2f& rOutTangentA, CVector2f& rOutTangentB) const
{
    if (Flags & 0x8000)
        CalculateTangents(pkPrev, pkNext);

    rOutTangentA = CachedTangentA;
    rOutTangentB = CachedTangentB;
}

void CMayaSplineKnot::CalculateTangents(const CMayaSplineKnot *pkPrev, const CMayaSplineKnot *pkNext) const
{
    // todo: this function is incomplete
    Flags &= ~0x8000;

    if ((Flags >> 24) == 4 && pkPrev)
    {
        float PrevAmpDiff = Math::Abs(pkPrev->Amplitude - Amplitude);
        float NextAmpDiff = (pkNext ? Math::Abs(pkNext->Amplitude - Amplitude) : PrevAmpDiff);

        if (PrevAmpDiff > 0.05f && NextAmpDiff <= 0.05f)
        {
            Flags &= 0x00FFFFFF;
            Flags |= 0x01000000;
        }
    }

    u32 TopFlagByte = (Flags >> 24) & 0xFF;

    if (TopFlagByte == 0)
    {
    }
}

u32 CMayaSpline::GetKnotCount() const
{
    return mKnots.size();
}

const std::vector<CMayaSplineKnot>& CMayaSpline::GetKnots() const
{
    return mKnots;
}

float CMayaSpline::GetMinTime() const
{
    return (mKnots.empty() ? 0.f : mKnots.front().Time);
}

float CMayaSpline::GetMaxTime() const
{
    return (mKnots.empty() ? 0.f : mKnots.back().Time);
}

float CMayaSpline::GetDuration() const
{
    if (mKnots.empty()) return 0.f;
    else return GetMaxTime() - mKnots.front().Time;
}

float CMayaSpline::EvaluateAt(float Time) const
{
    float Amplitude = EvaluateAtUnclamped(Time);

    if (mClampMode == 0)
        return Amplitude;

    else if (mClampMode == 1)
    {
        if (Amplitude < mMinAmplitude)
            Amplitude = mMinAmplitude;
        else if (Amplitude > mMaxAmplitude)
            Amplitude = mMaxAmplitude;

        return Amplitude;
    }

    else if (mClampMode == 2)
    {
        if (mMaxAmplitude <= mMinAmplitude)
            return mMinAmplitude;

        // todo
        return 0.f;
    }

    else return 0.f;
}

float CMayaSpline::EvaluateAtUnclamped(float Time) const
{
    if (mKnots.size() == 0)
        return 0.f;

    // Check for infinity
    if (Time < mKnots.front().Time)
    {
        if (mPreInfinity == 0)
            return mKnots.front().Time;
        else
            return EvaluateInfinities(Time, true);
    }

    if (Time > mKnots.back().Time)
    {
        if (mPostInfinity == 0)
            return mKnots.back().Time;
        else
            return EvaluateInfinities(Time, false);
    }

    // Check for valid cached knot index
    int LastKnotIndex = mKnots.size() - 1;
    int KnotIndex = -1;
    bool ValidKnot = false;

    if (mCachedKnotIndex != -1)
    {
        // WTF second check? I've read the code like 10 times and I don't see any other way to interpret it, this is what the game does
        if ((mCachedKnotIndex < LastKnotIndex) && (Time > mKnots.back().Time))
        {
            // You will notice we already did that check earlier, so this code can't execute...
            float KnotTime = mKnots[mCachedKnotIndex + 1].Time;

            if (Time == KnotTime)
            {
                mCachedKnotIndex = LastKnotIndex;
                return mKnots.back().Amplitude;
            }

            else if (Time < KnotTime)
            {
                KnotIndex = mCachedKnotIndex + 1;
                ValidKnot = true;
            }
        }

        else if (mCachedKnotIndex > 0)
        {
            float KnotTime = mKnots[mCachedKnotIndex].Time;

            if (Time < KnotTime)
            {
                KnotTime = mKnots[mCachedKnotIndex - 1].Time;

                if (Time > KnotTime)
                {
                    KnotIndex = mCachedKnotIndex - 1;
                    ValidKnot = true;
                }

                if (Time == KnotTime)
                {
                    mCachedKnotIndex--;
                    return mKnots[mCachedKnotIndex].Amplitude;
                }
            }
        }
    }

    // Find new knot index if needed
    if (!ValidKnot)
    {
        bool ExactMatch = FindKnot(Time, KnotIndex);

        if (ExactMatch)
        {
            if (KnotIndex == 0)
            {
                mCachedKnotIndex = KnotIndex;
                return mKnots[KnotIndex].Amplitude;
            }

            else if (KnotIndex == mKnots.size())
            {
                mCachedKnotIndex = 0;
                return mKnots.back().Amplitude;
            }
        }
    }

    // Update Hermite coefficients
    int PrevKnot = KnotIndex - 1;

    if (mUnknown1 != PrevKnot)
    {
        mCachedKnotIndex = PrevKnot;
        mUnknown1 = PrevKnot;

        if ( ((mKnots[PrevKnot].Flags >> 16) & 0xFF) == 3)
        {
            mDirtyFlags |= 0x80;
        }

        else
        {
            mDirtyFlags &= ~0x80;

            std::vector<CVector2f> ControlPoints;
            FindControlPoints(PrevKnot, ControlPoints);
            CalculateHermiteCoefficients(ControlPoints, mCachedHermiteCoefficients);
            mCachedMinTime = ControlPoints.front().X;
        }
    }

    // Evaluate Hermite
    if (mDirtyFlags & 0x80)
        return mKnots[mCachedKnotIndex].Amplitude;
    else
        return EvaluateHermite(Time);
}

float CMayaSpline::EvaluateInfinities(float /*Time*/, bool Pre) const
{
    // todo - return Constant for now!
    if (Pre)
        return mKnots.front().Amplitude;
    else
        return mKnots.back().Amplitude;
}

float CMayaSpline::EvaluateHermite(float Time) const
{
    // todo: better organization and more descriptive variable names
    float f4 = Time - mCachedMinTime;
    float f3 = f4 * mCachedHermiteCoefficients[0];
    float f2 = mCachedHermiteCoefficients[1] + f3;
    f2 = f4 * f2;
    float f1 = mCachedHermiteCoefficients[2] + f2;
    f1 = f4 * f1;
    f1 = mCachedHermiteCoefficients[3] + f1;
    return f1;
}

bool CMayaSpline::FindKnot(float Time, int& rOutKnotIndex) const
{
    // Stores the index of the closest knot to Time (without going over).
    // Returns whether or not the knot found was an exact match.
    rOutKnotIndex = 0;

    if (mKnots.empty())
        return false;

    u32 Lower = 0;
    u32 Upper = mKnots.size();

    while (Lower < Upper)
    {
        u32 Index = (Lower + Upper) >> 1;

        if (mKnots[Index].Time > Time)
            Lower = Index + 1;
        else if (mKnots[Index].Time < Time)
            Upper = Index - 1;

        else
        {
            rOutKnotIndex = Index;
            return true;
        }
    }

    rOutKnotIndex = Lower;
    return false;
}

void CMayaSpline::FindControlPoints(int KnotIdx, std::vector<CVector2f>& rOut) const
{
        const CMayaSplineKnot *pkKnot = &mKnots[KnotIdx];
        CVector2f KnotPos(pkKnot->Time, pkKnot->Amplitude);
        rOut.push_back(KnotPos);

        CVector2f TangentA(0,0), TangentB(0,0);
        const CMayaSplineKnot *pkNext = (KnotIdx < (s32) mKnots.size() ? &mKnots[KnotIdx + 1] : nullptr);
        const CMayaSplineKnot *pkPrev = (KnotIdx > 0 ? &mKnots[KnotIdx - 1] : nullptr);
        pkKnot->GetTangents(pkPrev, pkNext, TangentA, TangentB);

        rOut.push_back(KnotPos + (TangentB * 0.333333f));

        // The game doesn't check whether the next knot exists before executing this code, not sure why...
        KnotIdx++;
        pkKnot = pkNext;
        KnotPos = CVector2f(pkNext->Time, pkNext->Amplitude);

        pkNext = (KnotIdx < (s32) mKnots.size() ? &mKnots[KnotIdx + 1] : nullptr);
        pkPrev = (KnotIdx > 0 ? &mKnots[KnotIdx - 1] : nullptr);
        pkKnot->GetTangents(pkPrev, pkNext, TangentA, TangentB);

        rOut.push_back(KnotPos - (TangentA * 0.333333f));
        rOut.push_back(KnotPos);
}

void CMayaSpline::CalculateHermiteCoefficients(const std::vector<CVector2f>& rkControlPoints, float *pOutCoefs) const
{
    // rkControlPoints should contain 4 elements.
    const CVector2f& rkKnotA = rkControlPoints[0];
    const CVector2f& rkTangentA = rkControlPoints[1];
    const CVector2f& rkTangentB = rkControlPoints[2];
    const CVector2f& rkKnotB = rkControlPoints[3];

    CVector2f Range = rkKnotB - rkKnotA;

    CVector2f KnotAToTangentA = rkTangentA - rkKnotA;
    float MulA = (KnotAToTangentA.X == 0 ? 5729578.f : KnotAToTangentA.Y / KnotAToTangentA.X);

    CVector2f KnotBToTangentB = rkKnotB - rkTangentB;
    float MulB = (KnotBToTangentB.X == 0 ? 5729578.f : KnotBToTangentB.Y / KnotBToTangentB.X);

    // todo: better organization and better variable names
    float f3 = Range.X * Range.X;
    float f1 = 1.0f;
    float f0 = Range.Y * 2;
    float f5 = MulA * Range.X;
    f3 = 1.f / f3;
    float f6 = MulB * Range.X;
    f0 = Range.Y + f0;
    f1 = f5 + f6;
    f0 -= f5;
    f1 -= f2;
    f0 -= f5;
    f1 -= f2;
    f0 -= f6;
    f1 *= f3;
    f0 *= f3;
    f1 /= Range.X;

    pOutCoefs[0] = f1;
    pOutCoefs[1] = f0;
    pOutCoefs[2] = MulA;
    pOutCoefs[3] = rkKnotA.Y;
}