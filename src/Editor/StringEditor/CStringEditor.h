#ifndef CSTRINGEDITOR_H
#define CSTRINGEDITOR_H

#include "IEditor.h"

#include "CStringListModel.h"
#include <Core/Resource/StringTable/CStringTable.h>

#include <QMainWindow>

namespace Ui {
class CStringEditor;
}

/** Editor window for string tables (STRG assets) */
class CStringEditor : public IEditor
{
    Q_OBJECT

    /** Qt UI */
    Ui::CStringEditor* mpUI;

    /** String table asset being edited */
    TResPtr<CStringTable> mpStringTable;

    /** Language being edited */
    ELanguage mCurrentLanguage;

    /** Index of the string being edited */
    uint mCurrentStringIndex;

    /** Model for the string list view */
    CStringListModel* mpListModel;

public:
    explicit CStringEditor(CStringTable* pStringTable, QWidget* pParent = 0);
    ~CStringEditor();
    void InitUI();
    void UpdateStatusBar();
    void SetActiveLanguage(ELanguage Language);
    void SetActiveString(int StringIndex);

    void LoadSettings();
    void SaveSettings();

public slots:
    void OnStringSelected(const QModelIndex& kIndex);
    void OnLanguageChanged(int LanguageIndex);
};

#endif // CSTRINGEDITOR_H
