#ifndef CSCANEDITOR_H
#define CSCANEDITOR_H

#include "Editor/IEditor.h"
#include <Core/Resource/Scan/CScan.h>

#include <memory>

namespace Ui {
class CScanEditor;
}

class CScanEditor : public IEditor
{
    Q_OBJECT

    /** Qt UI */
    std::unique_ptr<Ui::CScanEditor> mpUI;

    /** Scan asset being edited */
    TResPtr<CScan> mpScan;

public:
    explicit CScanEditor(CScan* pScan, QWidget* pParent = nullptr);
    ~CScanEditor() override;

public slots:
    bool Save() override;
};

#endif // CSCANEDITOR_H
