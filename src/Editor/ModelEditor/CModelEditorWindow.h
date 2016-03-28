#ifndef CMODELEDITORWINDOW_H
#define CMODELEDITORWINDOW_H

#include "CModelEditorViewport.h"
#include <Core/Render/CRenderer.h>
#include <Core/Resource/CFont.h>
#include <Core/Resource/CResCache.h>
#include <Core/Resource/Model/CModel.h>
#include <Core/Scene/CScene.h>
#include <Core/Scene/CModelNode.h>

#include <QMainWindow>
#include <QTimer>

namespace Ui {
class CModelEditorWindow;
}

class CModelEditorWindow : public QMainWindow
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
    QTimer mRefreshTimer;

public:
    explicit CModelEditorWindow(QWidget *pParent = 0);
    ~CModelEditorWindow();
    void SetActiveModel(CModel *pModel);
    void closeEvent(QCloseEvent *pEvent);

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
    void UpdateAnimParamUI(int Mode);

private:
    void ActivateMatEditUI(bool Active);
    void RefreshMaterial();

    enum EModelEditorWidget
    {
        eSetSelectComboBox,
        eMatSelectComboBox,
        eEnableTransparencyCheckBox,
        eEnablePunchthroughCheckBox,
        eEnableReflectionCheckBox,
        eEnableSurfaceReflectionCheckBox,
        eEnableDepthWriteCheckBox,
        eEnableOccluderCheckBox,
        eEnableLightmapCheckBox,
        eEnableLightingCheckBox,
        eSourceBlendComboBox,
        eDestBlendComboBox,
        eIndTextureResSelector,
        eKonstColorPickerA,
        eKonstColorPickerB,
        eKonstColorPickerC,
        eKonstColorPickerD,
        ePassTableWidget,
        eTevKColorSelComboBox,
        eTevKAlphaSelComboBox,
        eTevRasSelComboBox,
        eTevTexSelComboBox,
        eTevTexSourceComboBox,
        ePassTextureResSelector,
        eTevColorComboBoxA,
        eTevColorComboBoxB,
        eTevColorComboBoxC,
        eTevColorComboBoxD,
        eTevColorOutputComboBox,
        eTevAlphaComboBoxA,
        eTevAlphaComboBoxB,
        eTevAlphaComboBoxC,
        eTevAlphaComboBoxD,
        eTevAlphaOutputComboBox,
        eAnimModeComboBox,
        eAnimParamASpinBox,
        eAnimParamBSpinBox,
        eAnimParamCSpinBox,
        eAnimParamDSpinBox,
    };

private slots:
    void Open();
    void Import();
    void Save();
    void SaveAs();
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
