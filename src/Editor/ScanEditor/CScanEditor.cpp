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

    QString WindowTitle = "%APP_FULL_NAME% - Scan Editor - %1[*]";
    WindowTitle = WindowTitle.arg( TO_QSTRING(mpScan->Entry()->CookedAssetPath(true).GetFileName()) );
    SET_WINDOWTITLE_APPVARS(WindowTitle);
    
    connect( mpUI->ActionSave, SIGNAL(toggled(bool)), this, SLOT(Save()) );
    connect( mpUI->ActionSaveAndCook, SIGNAL(toggled(bool)), this, SLOT(SaveAndRepack()) );
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
