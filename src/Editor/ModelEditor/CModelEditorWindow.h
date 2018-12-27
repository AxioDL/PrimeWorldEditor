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

namespace Ui {
class CModelEditorWindow;
}

// the model editor is messy and old as fuck, it needs a total rewrite
class CModelEditorWindow : public IEditor
{
    Q_OBJECT

    Ui::CModelEditorWindow *ui;
    CScene *mpScene;
    QString mOutputFilename;
    TResPtr<CModel> mpCurrentModel;
    CModelNode *mpCurrentModelNode;
    CMaterial *mpCurrentMat;
    CMaterialPass *mpCurrentPass;
    bool mIgnoreSignals;

public:
    explicit CModelEditorWindow(CModel *pModel, QWidget *pParent = 0);
    ~CModelEditorWindow();
    bool Save();
    void SetActiveModel(CModel *pModel);
    CModelEditorViewport* Viewport() const;

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
