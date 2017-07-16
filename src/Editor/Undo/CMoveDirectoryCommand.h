#ifndef CMOVEDIRECTORYCOMMAND_H
#define CMOVEDIRECTORYCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/CEditorApplication.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
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

    void undo() { DoMove(mOldParent); }
    void redo() { DoMove(mNewParent); }
    bool AffectsCleanState() const { return false; }

protected:
    void DoMove(const TString& rkPath)
    {
        CVirtualDirectory *pDir = mpStore->GetVirtualDirectory(mTargetDir, false);
        CVirtualDirectory *pParent = mpStore->GetVirtualDirectory(rkPath, false);
        ASSERT(pDir && pParent);

        TString OldName = pDir->Name();
        CVirtualDirectory *pOldParent = pDir->Parent();
        pDir->SetParent(pParent);
        mTargetDir = pDir->FullPath();

        gpEdApp->ResourceBrowser()->DirectoryMoved(pDir, pOldParent, OldName);
    }
};

#endif // CMOVEDIRECTORYCOMMAND_H
