#ifndef COPENINGBANNER_H
#define COPENINGBANNER_H

#include <Common/Common.h>
class CGameProject;

// This class can be expanded later for better editing functionality.
// For the moment, we're only worried about editing the long game name.
class COpeningBanner
{
    CGameProject *mpProj;
    std::vector<uint8> mBannerData;
    bool mWii;

public:
    explicit COpeningBanner(CGameProject *pProj);
    TString EnglishGameName() const;
    void SetEnglishGameName(const TString& rkName);
    void Save();

    uint32 MaxGameNameLength() const;
};

#endif // COPENINGBANNER_H
