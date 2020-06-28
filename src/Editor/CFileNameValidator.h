#ifndef CFILENAMEVALIDATOR_H
#define CFILENAMEVALIDATOR_H

#include <QValidator>

#include "UICommon.h"
#include <Common/FileUtil.h>

class CFileNameValidator : public QValidator
{
    bool mIsDirectory;

public:
    CFileNameValidator(bool IsDirectory, QObject *pParent = nullptr)
        : QValidator(pParent)
        , mIsDirectory(IsDirectory)
    {}

    QValidator::State validate(QString& rInput, int&) const override
    {
        QValidator::State Out = QValidator::Acceptable;

        if (!FileUtil::IsValidName( TO_TSTRING(rInput), mIsDirectory ))
        {
            // Uh oh, the input is invalid. Only invalid characters will be considered entirely
            // invalid; other errors will be considered intermediate.
            Out = QValidator::Intermediate;

            for (int ChrIdx = 0; ChrIdx < rInput.size(); ChrIdx++)
            {
                char Chr = rInput.at(ChrIdx).toLatin1();

                if (!FileUtil::IsValidFileNameCharacter(Chr))
                {
                    Out = QValidator::Invalid;
                    break;
                }
            }
        }

        return Out;
    }

    void fixup(QString& rInput) const override
    {
        const TString Sanitized = FileUtil::SanitizeName(TO_TSTRING(rInput), mIsDirectory);
        rInput = TO_QSTRING(Sanitized);
    }
};

#endif // CFILENAMEVALIDATOR_H
