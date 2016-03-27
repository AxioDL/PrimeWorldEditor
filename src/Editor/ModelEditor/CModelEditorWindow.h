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
    void on_actionConvert_to_DDS_triggered();

    void on_actionOpen_triggered();
    void on_actionSave_triggered();

    void on_MeshPreviewButton_clicked();

    void on_SpherePreviewButton_clicked();

    void on_FlatPreviewButton_clicked();

    void on_ClearColorPicker_colorChanged(const QColor &);

    void on_actionImport_triggered();

    void on_actionSave_as_triggered();

    void on_CameraModeButton_clicked();

    void on_actionConvert_DDS_to_TXTR_triggered();

    void on_ToggleGridButton_toggled(bool checked);

signals:
    void Closed();
};

#endif // CMODELEDITORWINDOW_H
