#include "IUIRelay.h"

namespace
{
class NullUIRelay final : public IUIRelay
{
public:
    void ShowMessageBox(const std::string&, const std::string&) override {}
    void ShowMessageBoxAsync(const std::string&, const std::string&) override {}
    bool AskYesNoQuestion(const std::string&, const std::string&) override { return false; }
    OpenProjectResult OpenProject(const std::string&) override { return {}; }
};

constinit NullUIRelay sNullRelay;
constinit IUIRelay* sUIRelay = &sNullRelay;
}  // Anonymous namespace

void SetUIRelay(IUIRelay* relay)
{
    if (relay)
        sUIRelay = relay;
    else
        sUIRelay = &sNullRelay;
}

IUIRelay* GetUIRelay()
{
    return sUIRelay;
}
