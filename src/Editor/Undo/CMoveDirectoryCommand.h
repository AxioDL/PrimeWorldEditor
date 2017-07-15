#ifndef CMOVEDIRECTORYCOMMAND_H
#define CMOVEDIRECTORYCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/CEditorApplication.h"
#include <Core/GameProject/CResourceStore.h>
#include <Core/GameProject/CVirtualDirectory.h>

class CMoveDirectoryCommand : public IUndoCommand
{
    CResourceStore *mpStore;
    TString mTargetDir;
    TString mOldParent;
    TString mNewParent;

public:
    CMoveDirectoryCommand(CResourceStore *pStore, CVirtualDirectory *pDir, CVirtualDirectory *pNewParent)
        : IUndoCommand("Move Directory")
        , mpStore(pStore)
        , mTargetDir(pDir->FullPath())
        , mOldParent(pDir->Parent()->FullPath())
        , mNewParent(pNewParent->FullPath())
    {}

    void undo()
    {
        CVirtualDirectory *pDir = mpStore->GetVirtualDirectory(mTargetDir, false);
        CVirtualDirectory *pParent = mpStore->GetVirtualDirectory(mOldParent, false);
        ASSERT(pDir && pParent);

        pDir->SetParent(pParent);
        mTargetDir = pDir->FullPath();

        gpEdApp->DirectoryRenamed(pDir);
    }

    void redo()
    {
        CVirtualDirectory *pDir = mpStore->GetVirtualDirectory(mTargetDir, false);
        CVirtualDirectory *pParent = mpStore->GetVirtualDirectory(mNewParent, false);
        ASSERT(pDir && pParent);

        pDir->SetParent(pParent);
        mTargetDir = pDir->FullPath();

        gpEdApp->DirectoryRenamed(pDir);
    }

    bool AffectsCleanState() const { return false; }
};

#endif // CMOVEDIRECTORYCOMMAND_H
