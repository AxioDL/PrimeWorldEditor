#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CEditScriptPropertyCommand : public IUndoCommand
{
    CWorldEditor *mpEditor;
    CPropertyPtr mpProp;
    IPropertyValue *mpOldValue;
    IPropertyValue *mpNewValue;
    bool mCommandEnded;

public:
    CEditScriptPropertyCommand(IProperty *pProp, CWorldEditor *pEditor, IPropertyValue *pOldValue, bool IsDone, const QString& rkCommandName = "Edit Property");
    ~CEditScriptPropertyCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
    bool AffectsCleanState() const { return true; }
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
