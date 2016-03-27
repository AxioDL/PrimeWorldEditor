#ifndef CTIMER_H
#define CTIMER_H

class CTimer
{
    double mStartTime;
    double mPauseStartTime;
    double mTotalPauseTime;
    double mStopTime;
    bool mStarted;
    bool mPaused;

public:
    CTimer();
    void Start();
    void Start(double StartTime);
    void Restart();
    double Stop();
    double Pause();
    bool IsPaused();
    void Resume();
    double Time();

    // Static
    static double GlobalTime();
    static float SecondsMod900();
};

#endif // CTIMER_H
