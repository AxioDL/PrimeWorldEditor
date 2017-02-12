#ifndef CTIMEDLINEEDIT_H
#define CTIMEDLINEEDIT_H

#include <QLineEdit>
#include <QTimer>

// Simple line edit subclass that emits a signal when the user stops typing.
class CTimedLineEdit : public QLineEdit
{
    Q_OBJECT

    QString mCachedText;
    float mTimeoutDuration;
    QTimer mTimer;

public:
    CTimedLineEdit(QWidget *pParent = 0)
        : QLineEdit(pParent)
        , mTimeoutDuration(0.3f)
    {
        connect(this, SIGNAL(textChanged(QString)), this, SLOT(OnTextChanged()));
        connect(&mTimer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
    }

    inline void SetTimeoutDuration(float Duration)  { mTimeoutDuration = Duration; }
    inline float TimeoutDuration() const            { return mTimeoutDuration; }

signals:
    void StoppedTyping(const QString& rkText);

protected slots:
    virtual void OnTextChanged()
    {
        mTimer.start(mTimeoutDuration * 1000);
    }

    virtual void OnTimeout()
    {
        mTimer.stop();

        // Don't emit if the text is the same
        if (mCachedText != text())
            emit StoppedTyping(text());

        mCachedText = text();
    }
};

#endif // CTIMEDLINEEDIT_H
