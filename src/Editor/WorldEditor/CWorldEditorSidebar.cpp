#include "Editor/WorldEditor/CWorldEditorSidebar.h"
#include "Editor/WorldEditor/CWorldEditor.h"

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
