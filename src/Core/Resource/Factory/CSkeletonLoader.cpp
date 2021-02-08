#include "CSkeletonLoader.h"
#include <Common/Macros.h>
#include <Common/Log.h>

#include <vector>

void CSkeletonLoader::SetLocalBoneCoords(CBone *pBone)
{
    for (size_t iChild = 0; iChild < pBone->NumChildren(); iChild++)
        SetLocalBoneCoords(pBone->ChildByIndex(iChild));

    if (pBone->mpParent != nullptr)
        pBone->mLocalPosition = pBone->mPosition - pBone->mpParent->mPosition;
    else
        pBone->mLocalPosition = pBone->mPosition;
}

void CSkeletonLoader::CalculateBoneInverseBindMatrices()
{
    for (auto& bone : mpSkeleton->mBones)
    {
        bone->mInvBind = CTransform4f::TranslationMatrix(-bone->Position());
    }
}

// ************ STATIC ************
std::unique_ptr<CSkeleton> CSkeletonLoader::LoadCINF(IInputStream& rCINF, CResourceEntry *pEntry)
{
    auto ptr = std::make_unique<CSkeleton>(pEntry);

    CSkeletonLoader Loader;
    Loader.mpSkeleton = ptr.get();
    EGame Game = pEntry->Game();

    // We don't support DKCR CINF right now
    if (rCINF.PeekULong() == 0x9E220006)
        return ptr;

    const uint32 NumBones = rCINF.ReadULong();
    ptr->mBones.reserve(NumBones);

    // Read bones
    struct SBoneInfo
    {
        uint32 ParentID;
        std::vector<uint32> ChildIDs;
    };
    std::vector<SBoneInfo> BoneInfo(NumBones);

    for (uint32 iBone = 0; iBone < NumBones; iBone++)
    {
        auto& pBone = ptr->mBones.emplace_back(std::make_unique<CBone>(ptr.get()));

        pBone->mID = rCINF.ReadULong();
        BoneInfo[iBone].ParentID = rCINF.ReadULong();
        pBone->mPosition = CVector3f(rCINF);

        // Version test. No version number. The next value is the linked bone count in MP1 and the
        // rotation value in MP2. The max bone count is 100 so the linked bone count will not be higher
        // than that. Additionally, every bone links to its parent at least and every skeleton (as far as I
        // know) has at least two bones so the linked bone count will never be 0.
        if (Game == EGame::Invalid)
        {
            const uint32 Check = rCINF.PeekULong();
            Game = ((Check > 100 || Check == 0) ? EGame::Echoes : EGame::Prime);
        }
        if (Game >= EGame::Echoes)
        {
            pBone->mRotation = CQuaternion(rCINF);
            pBone->mLocalRotation = CQuaternion(rCINF);
        }

        const uint32 NumLinkedBones = rCINF.ReadULong();
        ASSERT(NumLinkedBones != 0);

        for (uint32 iLink = 0; iLink < NumLinkedBones; iLink++)
        {
            const uint32 LinkedID = rCINF.ReadULong();

            if (LinkedID != BoneInfo[iBone].ParentID)
                BoneInfo[iBone].ChildIDs.push_back(LinkedID);
        }
    }

    // Fill in bone info
    for (uint32 iBone = 0; iBone < NumBones; iBone++)
    {
        CBone *pBone = ptr->mBones[iBone].get();
        const SBoneInfo& rInfo = BoneInfo[iBone];

        pBone->mpParent = ptr->BoneByID(rInfo.ParentID);

        for (const auto childID : rInfo.ChildIDs)
        {
            if (CBone* pChild = ptr->BoneByID(childID))
                pBone->mChildren.push_back(pChild);
            else
                errorf("%s: Bone %u has invalid child ID: %u", *rCINF.GetSourceString(), pBone->mID, childID);
        }

        if (pBone->mpParent == nullptr)
        {
            if (ptr->mpRootBone == nullptr)
                ptr->mpRootBone = pBone;
            else
                errorf("%s: Multiple root bones?", *rCINF.GetSourceString());
        }
    }

    Loader.SetLocalBoneCoords(ptr->mpRootBone);
    Loader.CalculateBoneInverseBindMatrices();

    // Skip bone ID array
    const uint32 NumBoneIDs = rCINF.ReadULong();
    rCINF.Seek(NumBoneIDs * 4, SEEK_CUR);

    // Read bone names
    const uint32 NumBoneNames = rCINF.ReadULong();

    for (uint32 iName = 0; iName < NumBoneNames; iName++)
    {
        TString Name = rCINF.ReadString();
        const uint32 BoneID = rCINF.ReadULong();

        ptr->BoneByID(BoneID)->mName = std::move(Name);
    }

    return ptr;
}
