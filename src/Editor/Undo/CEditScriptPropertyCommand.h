#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "CBasicPropertyCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"

class CEditScriptPropertyCommand : public CBasicPropertyCommand
{
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
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
