#ifndef IUNDOCOMMAND
#define IUNDOCOMMAND

#include <QUndoCommand>

class IUndoCommand : public QUndoCommand
{
public:
    IUndoCommand(QUndoCommand *pParent = 0)
        : QUndoCommand(pParent) {}

    IUndoCommand(const QString& rkText, QUndoCommand *pParent = 0)
        : QUndoCommand(rkText, pParent) {}

    virtual bool AffectsCleanState() const = 0;
};

#endif // IUNDOCOMMAND

