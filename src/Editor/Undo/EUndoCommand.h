#ifndef EUNDOCOMMAND
#define EUNDOCOMMAND

// This enum is used as an ID for merging undo commands.
// If a command can't merge, then it doesn't have to be listed here.
enum EUndoCommand
{
    eTranslateNodeCmd,
    eRotateNodeCmd,
    eScaleNodeCmd,
    eEditPropertyCmd
};

#endif // EUNDOCOMMAND

