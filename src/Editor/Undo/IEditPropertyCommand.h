#ifndef IEDITPROPERTYCOMMAND_H
#define IEDITPROPERTYCOMMAND_H

#include "IUndoCommand.h"
#include "EUndoCommand.h"
#include "Editor/PropertyEdit/CPropertyModel.h"

class IEditPropertyCommand : public IUndoCommand
{
protected:
    // Has to be std::vector for compatibility with CVectorOutStream
    std::vector<char> mOldData;
    std::vector<char> mNewData;

    IProperty* mpProperty;
    CPropertyModel* mpModel;
    QModelIndex mIndex;
    bool mCommandEnded;
    bool mSavedOldData;
    bool mSavedNewData;

    /** Save the current state of the object properties to the given data buffer */
    void SaveObjectStateToArray(std::vector<char>& rVector);

    /** Restore the state of the object properties from the given data buffer */
    void RestoreObjectStateFromArray(std::vector<char>& rArray);

public:
    IEditPropertyCommand(
            IProperty* pProperty,
            CPropertyModel* pModel,
            const QModelIndex& kIndex,
            const QString& kCommandName = "Edit Property"
            );

    virtual ~IEditPropertyCommand() {}

    virtual void SaveOldData();
    virtual void SaveNewData();

    bool IsNewDataDifferent();
    void SetEditComplete(bool IsComplete);

    /** Interface */
    virtual void GetObjectDataPointers(QVector<void*>& rOutPointers) const = 0;

    /** IUndoCommand/QUndoCommand interface */
    int id() const;
    bool mergeWith(const QUndoCommand *pkOther);
    void undo();
    void redo();
    bool AffectsCleanState() const;
};

#endif // IEDITPROPERTYCOMMAND_H
