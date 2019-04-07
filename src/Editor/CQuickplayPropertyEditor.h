#ifndef CQUICKPLAYPROPERTYEDITOR_H
#define CQUICKPLAYPROPERTYEDITOR_H

#include <QMenu>
#include "NDolphinIntegration.h"

namespace Ui {
class CQuickplayPropertyEditor;
}

/** Property editor widget for quickplay.
 *  @todo may want this to use a CPropertyView eventually.
 */
class CQuickplayPropertyEditor : public QMenu
{
    Q_OBJECT

    Ui::CQuickplayPropertyEditor* mpUI;
    SQuickplayParameters& mParameters;

public:
    CQuickplayPropertyEditor(SQuickplayParameters& Parameters, QWidget* pParent = 0);
    ~CQuickplayPropertyEditor();

public slots:
    void BrowseForDolphin();
    void OnDolphinPathChanged(const QString& kNewPath);
    void OnBootToAreaToggled(bool Enabled);
    void OnSpawnAtCameraLocationToggled(bool Enabled);
};

#endif // CQUICKPLAYPROPERTYEDITOR_H
