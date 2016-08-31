#ifndef IEDITOR
#define IEDITOR

#include <QMainWindow>
#include "CEditorApplication.h"

class IEditor : public QMainWindow
{
    Q_OBJECT

public:
    IEditor(QWidget *pParent)
        : QMainWindow(pParent)
    {
        gpEdApp->AddEditor(this);
    }

    virtual void closeEvent(QCloseEvent*)           { emit Closed(); }
    virtual void EditorTick(float /*DeltaTime*/)    { }
    virtual CBasicViewport* Viewport() const        { return nullptr; }

signals:
    void Closed();
};

#endif // IEDITOR

