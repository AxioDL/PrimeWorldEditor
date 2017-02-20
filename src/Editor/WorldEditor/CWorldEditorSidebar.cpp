#include "CWorldEditorSidebar.h"
#include "CWorldEditor.h"

CWorldEditorSidebar::CWorldEditorSidebar(CWorldEditor *pEditor)
    : QWidget(pEditor)
    , mpWorldEditor(pEditor)
{
    setHidden(true);
}

CWorldEditor* CWorldEditorSidebar::Editor() const
{
    return mpWorldEditor;
}
