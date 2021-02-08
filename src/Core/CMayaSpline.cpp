#include "CMayaSpline.h"
#include <Common/Math/MathUtil.h>

void ValidateTangent(CVector2f& Tangent)
{
    if (Tangent.X < 0.f) Tangent.X = 0.f;
    Tangent = Tangent.Normalized();

    if (Tangent.X == 0.f && Tangent.Y != 0.f)
    {
        float Mul = (Tangent.Y >= 0.f ? 1.f : -1.f);
        Tangent.X = 0.0001f;
        Tangent.Y = 5729578 * Tangent.X * Mul; // not sure where that number comes from!
    }
}

void CMayaSplineKnot::GetTangents(const CMayaSplineKnot* pkPrev, const CMayaSplineKnot* pkNext, CVector2f& OutTangentA, CVector2f& OutTangentB) const
{
    if (Flags & 0x8000)
        CalculateTangents(pkPrev, pkNext);

    OutTangentA = CachedTangentA;
    OutTangentB = CachedTangentB;
}

void CMayaSplineKnot::CalculateTangents(const CMayaSplineKnot* pkPrev, const CMayaSplineKnot* pkNext) const
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

    uint32 TopFlagByte = (Flags >> 24) & 0xFF;

    if (TopFlagByte == 0)
    {
    }
}

uint CMayaSpline::GetKnotCount() const
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
    {
        return Amplitude;
    }
    else if (mClampMode == 1)
    {
        return Math::Clamp(mMinAmplitude, mMaxAmplitude, Amplitude);
    }
    else if (mClampMode == 2)
    {
        if (mMaxAmplitude <= mMinAmplitude)
            return mMinAmplitude;

        // todo
        return 0.f;
    }
    else
    {
        return 0.f;
    }
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

bool CMayaSpline::FindKnot(float Time, int& OutKnotIndex) const
{
    // Stores the index of the closest knot to Time (without going over).
    // Returns whether or not the knot found was an exact match.
    OutKnotIndex = 0;

    if (mKnots.empty())
        return false;

    uint Lower = 0;
    uint Upper = mKnots.size();

    while (Lower < Upper)
    {
        uint Index = (Lower + Upper) >> 1;

        if (mKnots[Index].Time > Time)
            Lower = Index + 1;
        else if (mKnots[Index].Time < Time)
            Upper = Index - 1;

        else
        {
            OutKnotIndex = Index;
            return true;
        }
    }

    OutKnotIndex = Lower;
    return false;
}

void CMayaSpline::FindControlPoints(int KnotIdx, std::vector<CVector2f>& Out) const
{
        const CMayaSplineKnot* pkKnot = &mKnots[KnotIdx];
        CVector2f KnotPos(pkKnot->Time, pkKnot->Amplitude);
        Out.push_back(KnotPos);

        CVector2f TangentA(0,0), TangentB(0,0);
        const CMayaSplineKnot* pkNext = (KnotIdx < mKnots.size() ? &mKnots[KnotIdx + 1] : nullptr);
        const CMayaSplineKnot* pkPrev = (KnotIdx > 0 ? &mKnots[KnotIdx - 1] : nullptr);
        pkKnot->GetTangents(pkPrev, pkNext, TangentA, TangentB);

        Out.push_back(KnotPos + (TangentB * 0.333333f));

        // The game doesn't check whether the next knot exists before executing this code, not sure why...
        KnotIdx++;
        pkKnot = pkNext;
        KnotPos = CVector2f(pkNext->Time, pkNext->Amplitude);

        pkNext = (KnotIdx < mKnots.size() ? &mKnots[KnotIdx + 1] : nullptr);
        pkPrev = (KnotIdx > 0 ? &mKnots[KnotIdx - 1] : nullptr);
        pkKnot->GetTangents(pkPrev, pkNext, TangentA, TangentB);

        Out.push_back(KnotPos - (TangentA * 0.333333f));
        Out.push_back(KnotPos);
}

void CMayaSpline::CalculateHermiteCoefficients(const std::vector<CVector2f>& kControlPoints, float* pOutCoefs) const
{
    // rkControlPoints should contain 4 elements.
    const CVector2f& kKnotA = kControlPoints[0];
    const CVector2f& kTangentA = kControlPoints[1];
    const CVector2f& kTangentB = kControlPoints[2];
    const CVector2f& kKnotB = kControlPoints[3];

    [[maybe_unused]] const CVector2f Range = kKnotB - kKnotA;

    const CVector2f KnotAToTangentA = kTangentA - kKnotA;
    const float MulA = (KnotAToTangentA.X == 0 ? 5729578.f : KnotAToTangentA.Y / KnotAToTangentA.X);

    const CVector2f KnotBToTangentB = kKnotB - kTangentB;
    [[maybe_unused]] const float MulB = (KnotBToTangentB.X == 0 ? 5729578.f : KnotBToTangentB.Y / KnotBToTangentB.X);

#if 0
    // todo: better organization and better variable names
    // also this doesn't actually compile, oops! will need to be re-reverse engineered
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
#else
    pOutCoefs[0] = 0.0f;
    pOutCoefs[1] = 0.0f;
#endif
    pOutCoefs[2] = MulA;
    pOutCoefs[3] = kKnotA.Y;
}
