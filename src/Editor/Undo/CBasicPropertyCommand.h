#ifndef CBASICPROPERTYCOMMAND_H
#define CBASICPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/WorldEditor/CWorldEditor.h"

class CBasicPropertyCommand : public IUndoCommand
{
protected:
    IProperty *mpProperty;
    IPropertyTemplate *mpTemplate;
    CPropertyStruct *mpBaseStruct;
    CWorldEditor *mpEditor;

    bool mIsInArray;
    QVector<u32> mArrayIndices;

public:
    CBasicPropertyCommand(IProperty *pProp, CWorldEditor *pEditor, const QString& rkCommandName = "Edit Property");
    virtual void UpdateArraySubProperty();
    virtual bool AffectsCleanState() const { return true; }
};

#endif // CBASICPROPERTYCOMMAND_H
