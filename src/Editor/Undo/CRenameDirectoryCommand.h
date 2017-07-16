#ifndef CRENAMEDIRECTORYCOMMAND_H
#define CRENAMEDIRECTORYCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/CEditorApplication.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Core/GameProject/CVirtualDirectory.h>

class CRenameDirectoryCommand : public IUndoCommand
{
    CVirtualDirectory *mpDir;
    TString mOldName;
    TString mNewName;

public:
    CRenameDirectoryCommand(CVirtualDirectory *pDir, const TString& rkNewName)
        : IUndoCommand("Rename Directory")
        , mpDir(pDir)
        , mOldName(pDir->Name())
        , mNewName(rkNewName)
    {}

    void undo() { DoMove(mOldName); }
    void redo() { DoMove(mNewName); }
    bool AffectsCleanState() const { return false; }

protected:
    void DoMove(const TString& rkName)
    {
        TString OldName = mpDir->Name();
        bool Success = mpDir->Rename(rkName);
        ASSERT(Success);

        gpEdApp->ResourceBrowser()->DirectoryMoved(mpDir, mpDir->Parent(), OldName);
    }
};

#endif // CRENAMEDIRECTORYCOMMAND_H
