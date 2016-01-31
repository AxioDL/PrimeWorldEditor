#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "Editor/PropertyEdit/CPropertyDelegate.h"
#include <QUndoCommand>

class CEditScriptPropertyCommand : public QUndoCommand
{
    CPropertyModel *mpModel;
    IProperty *mpProperty;
    QModelIndex mIndex;

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
