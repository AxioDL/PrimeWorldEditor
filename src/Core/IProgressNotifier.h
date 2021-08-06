#ifndef IPROGRESSNOTIFIER_H
#define IPROGRESSNOTIFIER_H

#include <Common/Common.h>
#include <Common/Math/MathUtil.h>

class IProgressNotifier
{
    TString mTaskName;
    int mTaskIndex = 0;
    int mTaskCount = 1;

public:
    IProgressNotifier() = default;
    virtual ~IProgressNotifier() = default;

    void SetNumTasks(int NumTasks)
    {
        mTaskName = "";
        mTaskIndex = 0;
        mTaskCount = NumTasks;
    }

    void SetTask(int TaskIndex, TString TaskName)
    {
        mTaskName = std::move(TaskName);
        mTaskIndex = TaskIndex;
        mTaskCount = Math::Max(mTaskCount, TaskIndex + 1);
    }

    void Report(uint64 StepIndex, uint64 StepCount, const TString& rkStepDesc = "")
    {
        ASSERT(mTaskCount >= 1);

        // Make sure TaskCount and StepCount are at least 1 so we don't have divide-by-zero errors
        int TaskCount = Math::Max(mTaskCount, 1);
        StepCount = Math::Max<uint64>(StepCount, 1);

        // Calculate percentage
        double TaskPercent = 1.f / (double) TaskCount;
        double StepPercent = (StepCount >= 0 ? (double) StepIndex / (double) StepCount : 0.f);
        double ProgressPercent = (TaskPercent * mTaskIndex) + (TaskPercent * StepPercent);
        UpdateProgress(mTaskName, rkStepDesc, (float) ProgressPercent);
    }

    void Report(const TString& rkStepDesc)
    {
        Report(0, 0, rkStepDesc);
    }

    void SetOneShotTask(const TString& rkTaskDesc)
    {
        SetNumTasks(1);
        SetTask(0, rkTaskDesc);
        Report(0, 0, "");
    }

    virtual bool ShouldCancel() const = 0;

protected:
    virtual void UpdateProgress(const TString& rkTaskName, const TString& rkStepDesc, float ProgressPercent) = 0;
};

// Null progress notifier can be passed to functions that require a progress notifier if you don't want to use one.
class CNullProgressNotifier : public IProgressNotifier
{
public:
    bool ShouldCancel() const override{ return false; }
protected:
    void UpdateProgress(const TString&, const TString&, float) override {}
};
extern CNullProgressNotifier *gpNullProgress;

#endif // IPROGRESSNOTIFIER_H
