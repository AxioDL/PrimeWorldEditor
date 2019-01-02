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

    /** Current string count */
    uint mCurrentStringCount;

    /** Model for the string list view */
    CStringListModel* mpListModel;

    /** Editor state flags */
    bool mIsEditingStringName;
    bool mIsEditingStringData;

public:
    explicit CStringEditor(CStringTable* pStringTable, QWidget* pParent = 0);
    ~CStringEditor();

    bool eventFilter(QObject* pWatched, QEvent* pEvent);

    void InitUI();
    void UpdateStatusBar();
    void SetActiveLanguage(ELanguage Language);
    void SetActiveString(int StringIndex);

    void LoadSettings();
    void SaveSettings();

    // Accessors
    inline CStringTable* StringTable() const { return mpStringTable; }

public slots:
    void UpdateUI();
    void OnStringSelected(const QModelIndex& kIndex);
    void OnLanguageChanged(int LanguageIndex);
    void OnStringNameEdited();
    void OnStringTextEdited();
    void OnAddString();
    void OnRemoveString();
    
    void IncrementStringIndex();
    void DecrementStringIndex();
    void IncrementLanguageIndex();
    void DecrementLanguageIndex();

signals:
    void StringNameEdited();
    void StringEdited();
};

#endif // CSTRINGEDITOR_H
