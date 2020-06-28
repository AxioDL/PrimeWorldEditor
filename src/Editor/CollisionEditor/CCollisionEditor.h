#ifndef CCOLLISIONEDITOR_H
#define CCOLLISIONEDITOR_H

#include "Editor/IEditor.h"
#include "CCollisionEditorViewport.h"
#include <Core/Scene/CCollisionNode.h>
#include <Core/Scene/CScene.h>
#include <memory>

namespace Ui {
class CCollisionEditor;
}

/** Editor window for dynamic collision (DCLN assets) */
class CCollisionEditor : public IEditor
{
    Q_OBJECT

    /** Qt UI */
    std::unique_ptr<Ui::CCollisionEditor>   mpUI;

    /** Collision mesh being edited */
    TResPtr<CCollisionMeshGroup>        mpCollisionMesh;

    /** Scene data */
    std::unique_ptr<CScene>             mpScene;
    std::unique_ptr<CCollisionNode>     mpCollisionNode;

public:
    /** Constructor/destructor */
    explicit CCollisionEditor(CCollisionMeshGroup* pCollisionMesh, QWidget* pParent = nullptr);
    ~CCollisionEditor() override;
    CCollisionEditorViewport* Viewport() const override;

public slots:
    void OnGridToggled(bool Enabled);
    void OnOrbitToggled(bool Enabled);
    void OnOBBTreeDepthChanged(int NewValue);
};

#endif // CCOLLISIONEDITOR_H
