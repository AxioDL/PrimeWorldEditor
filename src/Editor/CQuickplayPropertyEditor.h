#ifndef CQUICKPLAYPROPERTYEDITOR_H
#define CQUICKPLAYPROPERTYEDITOR_H

#include <QListWidgetItem>
#include <QMenu>
#include "NDolphinIntegration.h"

#include <Core/Resource/CWorld.h>
#include <Core/Resource/Area/CGameArea.h>

#include <memory>

namespace Ui {
class CQuickplayPropertyEditor;
}

/** Property editor widget for quickplay.
 *  @todo may want this to use a CPropertyView eventually.
 */
class CQuickplayPropertyEditor : public QMenu
{
    Q_OBJECT

    std::unique_ptr<Ui::CQuickplayPropertyEditor> mpUI;
    SQuickplayParameters& mParameters;

public:
    explicit CQuickplayPropertyEditor(SQuickplayParameters& Parameters, QWidget* pParent = nullptr);
    ~CQuickplayPropertyEditor() override;

public slots:
    void BrowseForDolphin();
    void OnDolphinPathChanged(const QString& kNewPath);
    void OnBootToAreaToggled(bool Enabled);
    void OnSpawnAtCameraLocationToggled(bool Enabled);
    void OnGiveAllItemsToggled(bool Enabled);
    void OnLayerListItemChanged(QListWidgetItem* pItem);

    void OnWorldEditorAreaChanged(CWorld* pWorld, CGameArea* pArea);
};

#endif // CQUICKPLAYPROPERTYEDITOR_H
