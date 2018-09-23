#ifndef WCREATETAB_H
#define WCREATETAB_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <QWidget>

namespace Ui {
class WCreateTab;
}

class WCreateTab : public QWidget
{
    Q_OBJECT
    CWorldEditor *mpEditor;
    CScriptLayer *mpSpawnLayer;

public:
    explicit WCreateTab(CWorldEditor *pEditor, QWidget *parent = 0);
    ~WCreateTab();
    bool eventFilter(QObject *, QEvent *);

    // Accessors
    inline CScriptLayer* SpawnLayer() const { return mpSpawnLayer; }

public slots:
    void OnActiveProjectChanged(CGameProject *pProj);
    void OnLayersChanged();
    void OnSpawnLayerChanged(int LayerIndex);

private:
    Ui::WCreateTab *ui;
};

#endif // WCREATETAB_H
