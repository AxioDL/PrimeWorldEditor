#ifndef CSCANEDITOR_H
#define CSCANEDITOR_H

#include "Editor/IEditor.h"
#include <Core/Resource/Scan/CScan.h>

namespace Ui {
class CScanEditor;
}

class CScanEditor : public IEditor
{
    Q_OBJECT

    /** Qt UI */
    Ui::CScanEditor* mpUI;

    /** Scan asset being edited */
    TResPtr<CScan> mpScan;

public:
    explicit CScanEditor(CScan* pScan, QWidget* pParent = 0);
    ~CScanEditor();
};

#endif // CSCANEDITOR_H
