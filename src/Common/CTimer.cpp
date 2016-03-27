#include "CTimer.h"
#include <cmath>
#include <ctime>

CTimer::CTimer()
    : mStartTime(0)
    , mStopTime(0)
    , mStarted(false)
    , mPaused(false)
{
}

void CTimer::Start()
{
    if (!mStarted)
    {
        mStartTime = GlobalTime();
        mStarted = true;
        mPaused = false;
        mPauseStartTime = 0;
        mTotalPauseTime = 0;
        mStopTime = 0;
    }
}

void CTimer::Start(double StartTime)
{
    if (!mStarted)
    {
        mStartTime = GlobalTime() - StartTime;
        mStarted = true;
        mPaused = false;
        mPauseStartTime = 0;
        mTotalPauseTime = 0;
        mStopTime = 0;
    }
}

void CTimer::Restart()
{
    mStarted = false;
    Start();
}

double CTimer::Stop()
{
    mStopTime = Time();
    mStarted = false;
    mPaused = false;
    return mStopTime;
}

double CTimer::Pause()
{
    mPauseStartTime = GlobalTime();
    mPaused = true;
    return Time();
}

bool CTimer::IsPaused()
{
    return mPaused;
}

void CTimer::Resume()
{
    if (mPaused)
    {
        mTotalPauseTime += GlobalTime() - mPauseStartTime;
        mPaused = false;
    }
}

double CTimer::Time()
{
    if (mStarted)
    {
        double CurrentPauseTime = 0;
        if (mPaused) CurrentPauseTime = GlobalTime() - mPauseStartTime;
        return GlobalTime() - mStartTime - mTotalPauseTime - CurrentPauseTime;
    }

    else
        return mStopTime;
}

// ************ STATIC ************
double CTimer::GlobalTime()
{
    return (double) clock() / CLOCKS_PER_SEC;
}

float CTimer::SecondsMod900()
{
    return fmodf((float) GlobalTime(), 900.f);
}
