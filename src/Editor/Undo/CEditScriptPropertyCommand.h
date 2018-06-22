#ifndef CEDITSCRIPTPROPERTYCOMMAND_H
#define CEDITSCRIPTPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "ObjReferences.h"
#include "EUndoCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"
#include "Editor/WorldEditor/CWorldEditor.h"

template<class PropertyClass>
class TEditScriptPropertyCommand : public IUndoCommand
{
    typedef typename PropertyClass::ValueType ValueType;

    CWorldEditor *mpEditor;
    CInstancePtr mpInstance;
    PropertyClass* mpProperty;
    ValueType mOldValue;
    ValueType mNewValue;
    bool mCommandEnded;

public:
    TEditScriptPropertyCommand(PropertyClass* pProp, CScriptObject* pInstance, CWorldEditor* pEditor, ValueType NewValue, bool IsDone, const QString& rkCommandName = "Edit Property")
        : IUndoCommand(rkCommandName)
        , mpEditor(pEditor)
        , mpInstance(pInstance)
        , mpProperty(pProp)
        , mOldValue(pProp->Value(pInstance->PropertyData()))
        , mNewValue(NewValue)
        , mCommandEnded(IsDone)
    {}

    int id() const
    {
        return eEditScriptPropertyCmd;
    }

    bool mergeWith(const QUndoCommand *pkOther)
    {
        if (!mCommandEnded)
        {
            TEditScriptPropertyCommand* pkCmd = dynamic_cast<TEditScriptPropertyCommand>(pkOther);

            if (pkCmd && pkCmd->mpProperty == mpProperty && pkCmd->mpInstance == mpInstance)
            {
                mNewValue = pkCmd->mNewValue;
                mCommandEnded = pkCmd->mCommandEnded;
                return true;
            }
        }

        return false;
    }

    void undo()
    {
        void* pData = mpInstance->PropertyData();
        mpProperty->ValueRef(pData) = mOldValue;
        mpEditor->OnPropertyModified(mpProperty);
        mCommandEnded = true;
    }

    void redo()
    {
        void* pData = mpInstance->PropertyData();
        mpProperty->ValueRef(pData) = mNewValue;
        mpEditor->OnPropertyModified(mpProperty);
    }

    bool AffectsCleanState() const
    {
        return true;
    }
};

/*class CEditScriptPropertyCommand : public IUndoCommand
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
};*/

#endif // CEDITSCRIPTPROPERTYCOMMAND_H
