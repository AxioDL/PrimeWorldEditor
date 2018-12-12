#ifndef CPROGRESSBARNOTIFIER_H
#define CPROGRESSBARNOTIFIER_H

#include <Common/Math/MathUtil.h>
#include <Core/IProgressNotifier.h>
#include <QProgressBar>

/** Progress notifier class that updates a QProgressBar. */
class CProgressBarNotifier : public IProgressNotifier
{
    /** The progress bar we are relaying updates to */
    QProgressBar* mpProgressBar;

    /** Whether the user has requested to cancel */
    bool mCancel;

public:
    CProgressBarNotifier()
        : IProgressNotifier()
        , mpProgressBar(nullptr)
        , mCancel(false)
    {}

    inline void SetProgressBar(QProgressBar* pProgressBar)
    {
        mpProgressBar = pProgressBar;
    }

    inline void SetCanceled(bool ShouldCancel)
    {
        mCancel = ShouldCancel;
    }

    /** IProgressNotifier interface */
    virtual bool ShouldCancel() const
    {
        return mCancel;
    }

protected:
    virtual void UpdateProgress(const TString &, const TString &, float ProgressPercent)
    {
        if (mpProgressBar)
        {
            int Alpha = Math::Lerp(mpProgressBar->minimum(), mpProgressBar->maximum(), ProgressPercent);

            // Defer setValue call so it runs on the correct thread
            QMetaObject::invokeMethod(mpProgressBar, "setValue", Qt::AutoConnection, Q_ARG(int, Alpha));
        }
    }
};

#endif // CPROGRESSBARNOTIFIER_H
