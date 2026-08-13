#ifndef WEDITORPROPERTIES_H
#define WEDITORPROPERTIES_H

#include <QWidget>

class CSceneNode;
class CScriptNode;
class CScriptObject;
class CWorldEditor;
class QCheckBox;
class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class IProperty;

class WEditorProperties : public QWidget
{
    Q_OBJECT
    CWorldEditor* mpEditor = nullptr;
    CSceneNode* mpDisplayNode = nullptr;

    QVBoxLayout* mpMainLayout;

    QLabel* mpInstanceInfoLabel;
    QHBoxLayout* mpInstanceInfoLayout;

    QCheckBox* mpActiveCheckBox;
    QLineEdit* mpInstanceNameLineEdit;
    QHBoxLayout* mpNameLayout;

    QLabel* mpLayersLabel;
    QComboBox* mpLayersComboBox;
    QHBoxLayout* mpLayersLayout;

    bool mHasEditedName = false;

public:
    explicit WEditorProperties(QWidget* pParent = nullptr);
    ~WEditorProperties() override;

    void SyncToEditor(CWorldEditor* pEditor);
    void SetLayerComboBox();

public slots:
    void OnSelectionModified();
    void OnPropertyModified(const IProperty* pProp, const CScriptObject* pInstance);
    void OnInstancesLayerChanged(const QList<CScriptNode*>& rkNodeList);
    void OnLayersModified();
    void UpdatePropertyValues();

protected slots:
    void OnActiveChanged();
    void OnInstanceNameEdited();
    void OnInstanceNameEditFinished();
    void OnLayerChanged();
};

#endif // WEDITORPROPERTIES_H
