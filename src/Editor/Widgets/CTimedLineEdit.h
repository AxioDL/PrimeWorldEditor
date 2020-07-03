#ifndef CTIMEDLINEEDIT_H
#define CTIMEDLINEEDIT_H

#include <QLineEdit>
#include <QTimer>

// Simple line edit subclass that emits a signal when the user stops typing.
class CTimedLineEdit : public QLineEdit
{
    Q_OBJECT

    QString mCachedText;
    float mTimeoutDuration{0.3f};
    QTimer mTimer;

public:
    explicit CTimedLineEdit(QWidget *pParent = nullptr)
        : QLineEdit(pParent)
    {
        connect(this, &CTimedLineEdit::textChanged, this, &CTimedLineEdit::OnTextChanged);
        connect(&mTimer, &QTimer::timeout, this, &CTimedLineEdit::OnTimeout);
    }

    void SetTimeoutDuration(float Duration)  { mTimeoutDuration = Duration; }
    float TimeoutDuration() const            { return mTimeoutDuration; }

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
