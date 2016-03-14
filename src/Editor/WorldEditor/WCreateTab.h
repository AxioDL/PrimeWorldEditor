#ifndef WCREATETAB_H
#define WCREATETAB_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CMasterTemplate.h>
#include <QWidget>

namespace Ui {
class WCreateTab;
}

class WCreateTab : public QWidget
{
    Q_OBJECT
    CWorldEditor *mpEditor;

public:
    explicit WCreateTab(QWidget *parent = 0);
    ~WCreateTab();
    bool eventFilter(QObject *, QEvent *);
    void SetEditor(CWorldEditor *pEditor);
    void SetMaster(CMasterTemplate *pMaster);
private:
    Ui::WCreateTab *ui;
};

#endif // WCREATETAB_H
