#ifndef IPROGRESSNOTIFIER_H
#define IPROGRESSNOTIFIER_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>

class IProgressNotifier
{
    std::string mTaskName;
    int mTaskIndex = 0;
    int mTaskCount = 1;

public:
    IProgressNotifier() = default;
    virtual ~IProgressNotifier() = default;

    void SetNumTasks(int NumTasks)
    {
        mTaskName.clear();
        mTaskIndex = 0;
        mTaskCount = NumTasks;
    }

    void SetTask(int TaskIndex, std::string TaskName)
    {
        mTaskName = std::move(TaskName);
        mTaskIndex = TaskIndex;
        mTaskCount = std::max(mTaskCount, TaskIndex + 1);
    }

    void Report(uint64_t StepIndex, uint64_t StepCount, const std::string& rkStepDesc = "")
    {
        assert(mTaskCount >= 1);

        // Make sure TaskCount and StepCount are at least 1 so we don't have divide-by-zero errors
        int TaskCount = std::max(mTaskCount, 1);
        StepCount = std::max<uint64_t>(StepCount, 1);

        // Calculate percentage
        double TaskPercent = 1.f / (double) TaskCount;
        double StepPercent = (StepCount >= 0 ? (double) StepIndex / (double) StepCount : 0.f);
        double ProgressPercent = (TaskPercent * mTaskIndex) + (TaskPercent * StepPercent);
        UpdateProgress(mTaskName, rkStepDesc, (float) ProgressPercent);
    }

    void Report(const std::string& rkStepDesc)
    {
        Report(0, 0, rkStepDesc);
    }

    void SetOneShotTask(const std::string& rkTaskDesc)
    {
        SetNumTasks(1);
        SetTask(0, rkTaskDesc);
        Report(0, 0, "");
    }

    virtual bool ShouldCancel() const = 0;

protected:
    virtual void UpdateProgress(const std::string& rkTaskName, const std::string& rkStepDesc, float ProgressPercent) = 0;
};

#endif // IPROGRESSNOTIFIER_H
