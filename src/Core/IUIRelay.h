#ifndef IUIRELAY_H
#define IUIRELAY_H

#include <Common/TString.h>

class IUIRelay
{
public:
    virtual ~IUIRelay() = default;
    virtual void ShowMessageBox(const TString& rkInfoBoxTitle, const TString& rkMessage) = 0;
    virtual void ShowMessageBoxAsync(const TString& rkInfoBoxTitle, const TString& rkMessage) = 0;
    virtual bool AskYesNoQuestion(const TString& rkInfoBoxTitle, const TString& rkQuestion) = 0;
    virtual bool OpenProject(const TString& kPath = "") = 0;
};
extern IUIRelay *gpUIRelay;

#endif // IUIRELAY_H
