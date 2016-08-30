#ifndef IEDITOR
#define IEDITOR

#include <QMainWindow>
#include "CEditorApplication.h"

class IEditor : public QMainWindow
{
public:
    IEditor(QWidget *pParent)
        : QMainWindow(pParent)
    {
        gpEdApp->AddEditor(this);
    }

    virtual void EditorTick(float /*DeltaTime*/)    { }
    virtual CBasicViewport* Viewport() const    { return nullptr; }
};

#endif // IEDITOR

