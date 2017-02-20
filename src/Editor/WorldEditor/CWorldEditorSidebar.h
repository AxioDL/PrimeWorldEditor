#ifndef CWORLDEDITORSIDEBAR_H
#define CWORLDEDITORSIDEBAR_H

#include <QWidget>
class CWorldEditor;

class CWorldEditorSidebar : public QWidget
{
    Q_OBJECT
    CWorldEditor *mpWorldEditor;

public:
    explicit CWorldEditorSidebar(CWorldEditor *pEditor);
    CWorldEditor* Editor() const;
    virtual void SidebarOpen() {}
    virtual void SidebarClose() {}
};

#endif // CWORLDEDITORSIDEBAR_H
