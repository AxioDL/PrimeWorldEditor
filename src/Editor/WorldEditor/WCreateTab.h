#ifndef WCREATETAB_H
#define WCREATETAB_H

#include "CWorldEditor.h"
#include <Core/Resource/Script/CGameTemplate.h>
#include <QWidget>

#include <memory>

namespace Ui {
class WCreateTab;
}

class WCreateTab : public QWidget
{
    Q_OBJECT
    CWorldEditor *mpEditor;
    CScriptLayer *mpSpawnLayer = nullptr;

public:
    explicit WCreateTab(CWorldEditor *pEditor, QWidget *parent = nullptr);
    ~WCreateTab() override;

    bool eventFilter(QObject *, QEvent *) override;

    // Accessors
    CScriptLayer* SpawnLayer() const { return mpSpawnLayer; }

public slots:
    void OnActiveProjectChanged(CGameProject *pProj);
    void OnLayersChanged();
    void OnSpawnLayerChanged(int LayerIndex);

private:
    std::unique_ptr<Ui::WCreateTab> ui;
};

#endif // WCREATETAB_H
