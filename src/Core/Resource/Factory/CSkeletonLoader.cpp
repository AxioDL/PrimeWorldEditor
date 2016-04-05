#include "CSkeletonLoader.h"
#include <Common/Log.h>

#include <vector>

CSkeletonLoader::CSkeletonLoader()
{
}

// ************ STATIC ************
CSkeleton* CSkeletonLoader::LoadCINF(IInputStream& rCINF)
{
    CSkeleton *pSkel = new CSkeleton();

    u32 NumBones = rCINF.ReadLong();
    pSkel->mBones.reserve(NumBones);

    // Read bones
    struct SBoneInfo
    {
        u32 ParentID;
        std::vector<u32> ChildIDs;
    };
    std::vector<SBoneInfo> BoneInfo(NumBones);

    for (u32 iBone = 0; iBone < NumBones; iBone++)
    {
        CBone *pBone = new CBone(pSkel);
        pSkel->mBones.push_back(pBone);

        pBone->mID = rCINF.ReadLong();
        BoneInfo[iBone].ParentID = rCINF.ReadLong();
        pBone->mPosition = CVector3f(rCINF);

        u32 NumLinkedBones = rCINF.ReadLong();

        for (u32 iLink = 0; iLink < NumLinkedBones; iLink++)
        {
            u32 LinkedID = rCINF.ReadLong();

            if (LinkedID != BoneInfo[iBone].ParentID)
                BoneInfo[iBone].ChildIDs.push_back(LinkedID);
        }
    }

    // Fill in bone info
    for (u32 iBone = 0; iBone < NumBones; iBone++)
    {
        CBone *pBone = pSkel->mBones[iBone];
        SBoneInfo& rInfo = BoneInfo[iBone];

        pBone->mpParent = pSkel->BoneByID(rInfo.ParentID);

        for (u32 iChild = 0; iChild < rInfo.ChildIDs.size(); iChild++)
        {
            u32 ChildID = rInfo.ChildIDs[iChild];
            CBone *pChild = pSkel->BoneByID(ChildID);

            if (pChild)
                pBone->mChildren.push_back(pChild);
            else
                Log::FileError(rCINF.GetSourceString(), "Bone " + TString::FromInt32(pBone->mID, 0, 10) + " has invalid child ID: " + TString::FromInt32(ChildID, 0, 10));
        }

        if (!pBone->mpParent)
        {
            if (!pSkel->mpRootBone)
                pSkel->mpRootBone = pBone;
            else
                Log::FileError(rCINF.GetSourceString(), "Multiple root bones?");
        }
    }

    // Skip bone ID array
    u32 NumBoneIDs = rCINF.ReadLong();
    rCINF.Seek(NumBoneIDs * 4, SEEK_CUR);

    // Read bone names
    u32 NumBoneNames = rCINF.ReadLong();

    for (u32 iName = 0; iName < NumBoneNames; iName++)
    {
        TString Name = rCINF.ReadString();
        u32 BoneID = rCINF.ReadLong();

        pSkel->BoneByID(BoneID)->mName = Name;
    }

    return pSkel;
}
