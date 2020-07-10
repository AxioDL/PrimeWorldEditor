#ifndef IUNDOCOMMAND
#define IUNDOCOMMAND

#include <QUndoCommand>

class IUndoCommand : public QUndoCommand
{
public:
    explicit IUndoCommand(QUndoCommand *pParent = nullptr)
        : QUndoCommand(pParent) {}

    explicit IUndoCommand(const QString& rkText, QUndoCommand *pParent = nullptr)
        : QUndoCommand(rkText, pParent) {}

    virtual bool AffectsCleanState() const = 0;
};

#endif // IUNDOCOMMAND

