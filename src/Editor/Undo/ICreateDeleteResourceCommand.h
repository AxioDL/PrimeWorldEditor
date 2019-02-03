#ifndef ICREATEDELETERESOURCECOMMAND_H
#define ICREATEDELETERESOURCECOMMAND_H

#include "IUndoCommand.h"
#include "Editor/CEditorApplication.h"
#include "Editor/ResourceBrowser/CResourceBrowser.h"
#include <Core/GameProject/CResourceEntry.h>
#include <Core/GameProject/CResourceStore.h>

class ICreateDeleteResourceCommand : public IUndoCommand
{
protected:
    CResourceEntry* mpEntry;
    TString mDirPath;

public:
    ICreateDeleteResourceCommand(const QString& kText, CResourceEntry* pEntry)
        : IUndoCommand(kText)
        , mpEntry(pEntry)
    {
        mDirPath = mpEntry->Directory()->FullPath();
    }

    void DoCreate()
    {
        CVirtualDirectory* pDir = mpEntry->ResourceStore()->GetVirtualDirectory(mDirPath, true);
        gpEdApp->ResourceBrowser()->ResourceAboutToBeCreated(pDir);

        // restore directory and undelete
        mpEntry->MarkDeleted(false);

        gpEdApp->ResourceBrowser()->ResourceCreated(mpEntry);
    }

    void DoDelete()
    {
        gpEdApp->ResourceBrowser()->ResourceAboutToBeDeleted(mpEntry);

        // save directory and delete
        mpEntry->MarkDeleted(true);

        gpEdApp->ResourceBrowser()->ResourceDeleted();
    }

    bool AffectsCleanState() const { return false; }
};

class CCreateResourceCommand : public ICreateDeleteResourceCommand
{
public:
    CCreateResourceCommand(CResourceEntry* pEntry)
        : ICreateDeleteResourceCommand("Create Resource", pEntry)
    {}

    void undo() { DoDelete(); }
    void redo() { DoCreate(); }
};

class CDeleteResourceCommand : public ICreateDeleteResourceCommand
{
public:
    CDeleteResourceCommand(CResourceEntry* pEntry)
        : ICreateDeleteResourceCommand("Delete Resource", pEntry)
    {}

    void undo() { DoCreate(); }
    void redo() { DoDelete(); }
};

#endif // ICREATEDELETERESOURCECOMMAND_H
