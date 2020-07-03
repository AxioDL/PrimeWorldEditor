#include "CScanEditor.h"
#include "ui_CScanEditor.h"

CScanEditor::CScanEditor(CScan* pScan, QWidget* pParent)
    : IEditor(pParent)
    , mpUI(std::make_unique<Ui::CScanEditor>())
    , mpScan(pScan)
{
    mpUI->setupUi(this);
    mpUI->PropertyView->SetIntrinsicProperties(pScan->ScanData());
    mpUI->ToolBar->addSeparator();
    AddUndoActions(mpUI->ToolBar);

    const QString WindowTitle = tr("%APP_FULL_NAME% - Scan Editor - %1[*]")
                                    .arg(TO_QSTRING(mpScan->Entry()->CookedAssetPath(true).GetFileName()));
    SET_WINDOWTITLE_APPVARS(WindowTitle);
    
    connect(mpUI->ActionSave, &QAction::toggled, this, &CScanEditor::Save);
    connect(mpUI->ActionSaveAndCook, &QAction::toggled, this, &CScanEditor::SaveAndRepack);
}

CScanEditor::~CScanEditor() = default;

bool CScanEditor::Save()
{
    if (mpScan->Entry()->Save())
    {
        UndoStack().setClean();
        setWindowModified(false);
        return true;
    }

    return false;
}
