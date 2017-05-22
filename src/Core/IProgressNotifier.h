#ifndef IPROGRESSNOTIFIER_H
#define IPROGRESSNOTIFIER_H

#include <Common/Common.h>

class IProgressNotifier
{
    TString mTaskName;
    int mTaskIndex;
    int mTaskCount;

public:
    IProgressNotifier()
        : mTaskIndex(0)
        , mTaskCount(1)
    {}

    void SetNumTasks(int NumTasks)
    {
        mTaskName = "";
        mTaskIndex = 0;
        mTaskCount = NumTasks;
    }

    void SetTask(int TaskIndex, TString TaskName)
    {
        mTaskName = TaskName;
        mTaskIndex = TaskIndex;
    }

    void Report(int StepIndex, int StepCount, const TString& rkStepDesc)
    {
        ASSERT(mTaskCount >= 1);

        // Calculate percentage
        float TaskPercent = 1.f / (float) mTaskCount;
        float StepPercent = (StepCount >= 0 ? (float) StepIndex / (float) StepCount : 0.f);
        float ProgressPercent = (TaskPercent * mTaskIndex) + (TaskPercent * StepPercent);
        UpdateProgress(mTaskName, rkStepDesc, ProgressPercent);
    }

    virtual bool ShouldCancel() const = 0;

protected:
    virtual void UpdateProgress(const TString& rkTaskName, const TString& rkStepDesc, float ProgressPercent) = 0;
};

#endif // IPROGRESSNOTIFIER_H
