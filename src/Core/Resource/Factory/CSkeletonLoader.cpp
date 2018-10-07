#include "CSkeletonLoader.h"
#include <Common/AssertMacro.h>
#include <Common/Log.h>

#include <vector>

void CSkeletonLoader::SetLocalBoneCoords(CBone *pBone)
{
    for (u32 iChild = 0; iChild < pBone->NumChildren(); iChild++)
        SetLocalBoneCoords(pBone->ChildByIndex(iChild));

    if (pBone->mpParent)
        pBone->mLocalPosition = pBone->mPosition - pBone->mpParent->mPosition;
    else
        pBone->mLocalPosition = pBone->mPosition;
}

void CSkeletonLoader::CalculateBoneInverseBindMatrices()
{
    for (u32 iBone = 0; iBone < mpSkeleton->mBones.size(); iBone++)
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

        // Version test. No version number. The next value is the linked bone count in MP1 and the
        // rotation value in MP2. The max bone count is 100 so the linked bone count will not be higher
        // than that. Additionally, every bone links to its parent at least and every skeleton (as far as I
        // know) has at least two bones so the linked bone count will never be 0.
        if (Game == EGame::Invalid)
        {
            u32 Check = rCINF.PeekLong();
            Game = ((Check > 100 || Check == 0) ? EGame::Echoes : EGame::Prime);
        }
        if (Game >= EGame::Echoes)
        {
            pBone->mRotation = CQuaternion(rCINF);
            pBone->mLocalRotation = CQuaternion(rCINF);
        }

        u32 NumLinkedBones = rCINF.ReadLong();
        ASSERT(NumLinkedBones != 0);

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

    Loader.SetLocalBoneCoords(pSkel->mpRootBone);
    Loader.CalculateBoneInverseBindMatrices();

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
