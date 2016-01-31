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
    void ResourceChanged(const QString& NewResPath);
    void EditResource(const CResourceInfo& rkRes);
    void ExportResource(const CResourceInfo& rkRes);

public:
    explicit WResourceSelector(QWidget *parent = 0);
    ~WResourceSelector();
    bool event(QEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool IsSupportedExtension(const QString& extension);
    bool HasSupportedExtension(const CResourceInfo& rkRes);

    // Getters
    CResourceInfo GetResourceInfo();
    CResource* GetResource();
    QString GetText();
    bool IsEditButtonEnabled();
    bool IsExportButtonEnabled();
    bool IsPreviewPanelEnabled();

    // Setters
    void SetResource(CResource *pRes);
    void SetResource(const CResourceInfo& rkRes);
    void SetAllowedExtensions(const QString& extension);
    void SetAllowedExtensions(const QStringList& extensions);
    void SetAllowedExtensions(const TStringList& extensions);
    void SetText(const QString& ResPath);
    void SetEditButtonEnabled(bool Enabled);
    void SetExportButtonEnabled(bool Enabled);
    void SetPreviewPanelEnabled(bool Enabled);
    void AdjustPreviewToParent(bool adjust);

    // Slots
public slots:
    void OnLineEditTextEdited();
    void OnBrowseButtonClicked();
    void OnEditButtonClicked();
    void OnExportButtonClicked();

private:
    void Edit();
    void Export();
    void LoadResource(const QString& ResPath);
    void CreatePreviewPanel();
    void ShowPreviewPanel();
    void HidePreviewPanel();
    void SetButtonsBasedOnResType();
};

#endif // WRESOURCESELECTOR_H
