#include "CSkeletonLoader.h"
#include <Common/Macros.h>
#include <Common/Log.h>

#include <vector>

void CSkeletonLoader::SetLocalBoneCoords(CBone *pBone)
{
    for (uint32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
        SetLocalBoneCoords(pBone->ChildByIndex(iChild));

    if (pBone->mpParent)
        pBone->mLocalPosition = pBone->mPosition - pBone->mpParent->mPosition;
    else
        pBone->mLocalPosition = pBone->mPosition;
}

void CSkeletonLoader::CalculateBoneInverseBindMatrices()
{
    for (uint32 iBone = 0; iBone < mpSkeleton->mBones.size(); iBone++)
    {
        CBone *pBone = mpSkeleton->mBones[iBone];
        pBone->mInvBind = CTransform4f::TranslationMatrix(-pBone->Position());
    }
}

// ************ STATIC ************
CSkeleton* CSkeletonLoader::LoadCINF(IInputStream& rCINF, CResourceEntry *pEntry)
{
    CSkeletonLoader Loader;
    CSkeleton *pSkel = new CSkeleton(pEntry);
    Loader.mpSkeleton = pSkel;
    EGame Game = pEntry->Game();

    // We don't support DKCR CINF right now
    if (rCINF.PeekLong() == 0x9E220006)
        return pSkel;

    uint32 NumBones = rCINF.ReadLong();
    pSkel->mBones.reserve(NumBones);

    // Read bones
    struct SBoneInfo
    {
        uint32 ParentID;
        std::vector<uint32> ChildIDs;
    };
    std::vector<SBoneInfo> BoneInfo(NumBones);

    for (uint32 iBone = 0; iBone < NumBones; iBone++)
    {
        CBone *pBone = new CBone(pSkel);
        pSkel->mBones.push_back(pBone);

        pBone->mID = rCINF.ReadLong();
        BoneInfo[iBone].ParentID = rCINF.ReadLong();
        pBone->mPosition = CVector3f(rCINF);

        // Version test. No version number. The next value is the linked bone count in MP1 and the
        // rotation value in MP2. The max bone count is 100 so the linked bone count will not be higher
        // than that. Additionally, every bone links to its parent at least and every skeleton (as far as I
        // know) has at least two bones so the linked bone count will never be 0.
        if (Game == EGame::Invalid)
        {
            uint32 Check = rCINF.PeekLong();
            Game = ((Check > 100 || Check == 0) ? EGame::Echoes : EGame::Prime);
        }
        if (Game >= EGame::Echoes)
        {
            pBone->mRotation = CQuaternion(rCINF);
            pBone->mLocalRotation = CQuaternion(rCINF);
        }

        uint32 NumLinkedBones = rCINF.ReadLong();
        ASSERT(NumLinkedBones != 0);

        for (uint32 iLink = 0; iLink < NumLinkedBones; iLink++)
        {
            uint32 LinkedID = rCINF.ReadLong();

            if (LinkedID != BoneInfo[iBone].ParentID)
                BoneInfo[iBone].ChildIDs.push_back(LinkedID);
        }
    }

    // Fill in bone info
    for (uint32 iBone = 0; iBone < NumBones; iBone++)
    {
        CBone *pBone = pSkel->mBones[iBone];
        SBoneInfo& rInfo = BoneInfo[iBone];

        pBone->mpParent = pSkel->BoneByID(rInfo.ParentID);

        for (uint32 iChild = 0; iChild < rInfo.ChildIDs.size(); iChild++)
        {
            uint32 ChildID = rInfo.ChildIDs[iChild];
            CBone *pChild = pSkel->BoneByID(ChildID);

            if (pChild)
                pBone->mChildren.push_back(pChild);
            else
                errorf("%s: Bone %d has invalid child ID: %d", *rCINF.GetSourceString(), pBone->mID, ChildID);
        }

        if (!pBone->mpParent)
        {
            if (!pSkel->mpRootBone)
                pSkel->mpRootBone = pBone;
            else
                errorf("%s: Multiple root bones?", *rCINF.GetSourceString());
        }
    }

    Loader.SetLocalBoneCoords(pSkel->mpRootBone);
    Loader.CalculateBoneInverseBindMatrices();

    // Skip bone ID array
    uint32 NumBoneIDs = rCINF.ReadLong();
    rCINF.Seek(NumBoneIDs * 4, SEEK_CUR);

    // Read bone names
    uint32 NumBoneNames = rCINF.ReadLong();

    for (uint32 iName = 0; iName < NumBoneNames; iName++)
    {
        TString Name = rCINF.ReadString();
        uint32 BoneID = rCINF.ReadLong();

        pSkel->BoneByID(BoneID)->mName = Name;
    }

    return pSkel;
}
