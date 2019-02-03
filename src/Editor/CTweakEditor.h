#ifndef CTWEAKEDITOR_H
#define CTWEAKEDITOR_H

#include "Editor/IEditor.h"

namespace Ui {
class CTweakEditor;
}

class CTweakEditor : public IEditor
{
    Q_OBJECT

    /** Qt UI */
    Ui::CTweakEditor* mpUI;

    /** List of editable tweak assets */
    QVector<CTweakData*> mTweakAssets;

    /** Whether the editor window has been shown before */
    bool mHasBeenShown;

    /** Index of tweak data currently being edited */
    int mCurrentTweakIndex;

public:
    explicit CTweakEditor(QWidget* pParent = 0);
    ~CTweakEditor();
    bool HasTweaks();

    virtual bool Save() override;

public slots:
    void SetActiveTweakData(CTweakData* pTweakData);
    void SetActiveTweakIndex(int Index);
    void OnTweakTabClicked(int Index);
    void OnProjectChanged(CGameProject* pNewProject);
};

#endif // CTWEAKEDITOR_H
