#ifndef WRESOURCESELECTOR_H
#define WRESOURCESELECTOR_H

#include "IPreviewPanel.h"
#include <Common/CFourCC.h>
#include <Core/Resource/CResourceInfo.h>
#include <Core/Resource/EResType.h>

#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class WResourceSelector : public QWidget
{
    Q_OBJECT

    // Selector
    QStringList mSupportedExtensions;
    bool mShowEditButton;
    bool mShowExportButton;

    // Preview Panel
    IPreviewPanel *mpPreviewPanel;
    bool mEnablePreviewPanel;
    bool mPreviewPanelValid;
    bool mShowingPreviewPanel;
    bool mAdjustPreviewToParent;

    // Resource
    CResourceInfo mResource;
    bool mResourceValid;

    // UI
    struct {
        QLineEdit *LineEdit;
        QPushButton *BrowseButton;
        QPushButton *ExportButton;
        QPushButton *EditButton;
        QHBoxLayout *Layout;
    } mUI;

    // Functions
signals:
    void ResourceChanged(const QString& rkNewResPath);
    void EditResource(const CResourceInfo& rkRes);
    void ExportResource(const CResourceInfo& rkRes);

public:
    explicit WResourceSelector(QWidget *pParent = 0);
    ~WResourceSelector();
    bool event(QEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool IsSupportedExtension(const QString& rkExtension);
    bool HasSupportedExtension(const CResourceInfo& rkRes);
    void UpdateFrameColor();

    // Getters
    CResourceInfo GetResourceInfo();
    CResource* GetResource();
    QString GetText();
    bool IsEditButtonEnabled();
    bool IsExportButtonEnabled();
    bool IsPreviewPanelEnabled();

    // Setters
    void SetResource(CResource *pRes);
    void SetResource(const QString& rkRes);
    void SetResource(const CResourceInfo& rkRes);
    void SetAllowedExtensions(const QString& rkExtension);
    void SetAllowedExtensions(const QStringList& rkExtensions);
    void SetAllowedExtensions(const TStringList& rkExtensions);
    void SetText(const QString& rkResPath);
    void SetEditButtonEnabled(bool Enabled);
    void SetExportButtonEnabled(bool Enabled);
    void SetPreviewPanelEnabled(bool Enabled);
    void AdjustPreviewToParent(bool Adjust);

    // Slots
public slots:
    void OnLineEditTextEdited();
    void OnBrowseButtonClicked();
    void OnEditButtonClicked();
    void OnExportButtonClicked();

private:
    void Edit();
    void Export();
    void CreatePreviewPanel();
    void ShowPreviewPanel();
    void HidePreviewPanel();
    void SetButtonsBasedOnResType();
};

#endif // WRESOURCESELECTOR_H
