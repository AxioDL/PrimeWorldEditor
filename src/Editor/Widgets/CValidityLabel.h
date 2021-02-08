#ifndef CVALIDITYLABEL_H
#define CVALIDITYLABEL_H

#include <QLabel>

/** QLabel subclass that displays different text in either red or green depending on a flag. */
class CValidityLabel : public QLabel
{
    Q_OBJECT

    /** String to display if valid */
    QString mValidString;

    /** String to display if invalid */
    QString mInvalidString;

    /** Whether we are displaying the valid or invalid string */
    bool mValid;

public:
    explicit CValidityLabel(QWidget* pParent = nullptr)
        : QLabel(pParent)
    {
        SetValid(true);
    }

    explicit CValidityLabel(QString rkValidText, QString rkInvalidText, QWidget* pParent = nullptr)
        : QLabel(rkValidText, pParent), mValidString(std::move(rkValidText)), mInvalidString(std::move(rkInvalidText))
    {
        SetValid(true);
    }

    /** Configure the strings to display */
    void SetValidityText(const QString& rkValidText, const QString& rkInvalidText)
    {
        mValidString = rkValidText;
        mInvalidString = rkInvalidText;
        setText(mValid ? mValidString : mInvalidString);
    }

    /** Returns whether we are valid */
    bool IsValid() const
    {
        return mValid;
    }

public slots:
    /** Updates the label as either valid or invalid */
    void SetValid(bool Valid)
    {
        mValid = Valid;
        QPalette NewPalette;

        if (mValid)
        {
            NewPalette.setColor(foregroundRole(), Qt::green);
            setText(mValidString);
        }
        else
        {
            NewPalette.setColor(foregroundRole(), Qt::red);
            setText(mInvalidString);
        }

        setPalette(NewPalette);
    }
};

#endif // CVALIDITYLABEL_H
