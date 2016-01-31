#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "CBasicPropertyCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include <QUndoCommand>

class CEditScriptPropertyCommand : public CBasicPropertyCommand
{
    IPropertyValue *mpOldValue;
    IPropertyValue *mpNewValue;
    bool mCommandEnded;

public:
    CEditScriptPropertyCommand(CPropertyModel *pModel, const QModelIndex& rkIndex, IPropertyValue *pOldValue, bool IsDone);
    ~CEditScriptPropertyCommand();
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
};

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
