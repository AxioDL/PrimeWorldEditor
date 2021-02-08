#ifndef CMODELEDITORWINDOW_H
#define CMODELEDITORWINDOW_H

#include "IEditor.h"
#include "CModelEditorViewport.h"
#include <Core/GameProject/CResourceStore.h>
#include <Core/Render/CRenderer.h>
#include <Core/Resource/CFont.h>
#include <Core/Resource/Model/CModel.h>
#include <Core/Scene/CScene.h>
#include <Core/Scene/CModelNode.h>

#include <QMainWindow>
#include <QTimer>

#include <memory>

namespace Ui {
class CModelEditorWindow;
}

// the model editor is messy and old as fuck, it needs a total rewrite
class CModelEditorWindow : public IEditor
{
    Q_OBJECT

    std::unique_ptr<Ui::CModelEditorWindow> ui;
    std::unique_ptr<CScene> mpScene;
    QString mOutputFilename;
    TResPtr<CModel> mpCurrentModel;
    std::unique_ptr<CModelNode> mpCurrentModelNode;
    CMaterial *mpCurrentMat = nullptr;
    CMaterialPass *mpCurrentPass = nullptr;
    bool mIgnoreSignals = false;

public:
    explicit CModelEditorWindow(CModel *pModel, QWidget *pParent = nullptr);
    ~CModelEditorWindow() override;

    bool Save() override;
    void SetActiveModel(CModel *pModel);
    CModelEditorViewport* Viewport() const override;

    enum class EModelEditorWidget
    {
        SetSelectComboBox,
        MatSelectComboBox,
        EnableTransparencyCheckBox,
        EnablePunchthroughCheckBox,
        EnableReflectionCheckBox,
        EnableSurfaceReflectionCheckBox,
        EnableDepthWriteCheckBox,
        EnableOccluderCheckBox,
        EnableLightmapCheckBox,
        EnableLightingCheckBox,
        SourceBlendComboBox,
        DestBlendComboBox,
        IndTextureResSelector,
        KonstColorPickerA,
        KonstColorPickerB,
        KonstColorPickerC,
        KonstColorPickerD,
        PassTableWidget,
        TevKColorSelComboBox,
        TevKAlphaSelComboBox,
        TevRasSelComboBox,
        TevTexSelComboBox,
        TevTexSourceComboBox,
        PassTextureResSelector,
        TevColorComboBoxA,
        TevColorComboBoxB,
        TevColorComboBoxC,
        TevColorComboBoxD,
        TevColorOutputComboBox,
        TevAlphaComboBoxA,
        TevAlphaComboBoxB,
        TevAlphaComboBoxC,
        TevAlphaComboBoxD,
        TevAlphaOutputComboBox,
        AnimModeComboBox,
        AnimParamASpinBox,
        AnimParamBSpinBox,
        AnimParamCSpinBox,
        AnimParamDSpinBox,
    };

public slots:
    void RefreshViewport();
    void SetActiveMaterial(int MatIndex);
    void SetActivePass(int PassIndex);
    void UpdateMaterial();
    void UpdateMaterial(int Value);
    void UpdateMaterial(int ValueA, int ValueB);
    void UpdateMaterial(double Value);
    void UpdateMaterial(bool Value);
    void UpdateMaterial(QColor Value);
    void UpdateMaterial(QString Value);
    void UpdateUI(int Value);
    void UpdateAnimParamUI(EUVAnimMode Mode);

private:
    void ActivateMatEditUI(bool Active);
    void RefreshMaterial();

private slots:
    void Import();
    void ConvertToDDS();
    void ConvertToTXTR();
    void SetMeshPreview();
    void SetSpherePreview();
    void SetFlatPreview();
    void ClearColorChanged(const QColor& rkNewColor);
    void ToggleCameraMode();
    void ToggleGrid(bool Enabled);

signals:
    void Closed();
};

#endif // CMODELEDITORWINDOW_H
