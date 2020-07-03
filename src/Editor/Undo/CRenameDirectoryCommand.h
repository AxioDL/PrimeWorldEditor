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

    void undo() override { DoMove(mOldName); }
    void redo() override { DoMove(mNewName); }
    bool AffectsCleanState() const override { return false; }

protected:
    void DoMove(const TString& rkName)
    {
        QString ParentPath = TO_QSTRING(mpDir->Parent() ? mpDir->Parent()->FullPath() : "");
        gpEdApp->ResourceBrowser()->DirectoryAboutToBeMoved(mpDir, ParentPath + TO_QSTRING(rkName));

        TString OldName = mpDir->Name();
        bool Success = mpDir->Rename(rkName);
        ASSERT(Success);

        gpEdApp->ResourceBrowser()->DirectoryMoved(mpDir, mpDir->Parent(), OldName);
    }
};

#endif // CRENAMEDIRECTORYCOMMAND_H
