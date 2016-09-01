#ifndef WRESOURCESELECTOR_H
#define WRESOURCESELECTOR_H

#include "IPreviewPanel.h"
#include <Common/CFourCC.h>
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

    // Preview Panel
    IPreviewPanel *mpPreviewPanel;
    bool mEnablePreviewPanel;
    bool mPreviewPanelValid;
    bool mShowingPreviewPanel;
    bool mAdjustPreviewToParent;

    // Resource
    CResourceEntry *mpResource;
    bool mResourceValid;

    // UI
    struct {
        QLineEdit *LineEdit;
        QPushButton *BrowseButton;
        QHBoxLayout *Layout;
    } mUI;

    // Functions
signals:
    void ResourceChanged(CResourceEntry *pNewRes);

public:
    explicit WResourceSelector(QWidget *pParent = 0);
    ~WResourceSelector();
    bool event(QEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool IsSupportedExtension(const QString& rkExtension);
    bool HasSupportedExtension(CResourceEntry *pEntry);
    void UpdateFrameColor();

    // Getters
    CResourceEntry* GetResourceEntry();
    CResource* GetResource();
    QString GetText();
    bool IsPreviewPanelEnabled();

    // Setters
    void SetResource(CResource *pRes);
    void SetResource(CResourceEntry *pEntry);
    void SetResource(const CAssetID& rkID);
    void SetResource(const QString& rkResPath);
    void SetAllowedExtensions(const QString& rkExtension);
    void SetAllowedExtensions(const QStringList& rkExtensions);
    void SetAllowedExtensions(const TStringList& rkExtensions);
    void SetText(const QString& rkResPath);
    void SetPreviewPanelEnabled(bool Enabled);
    void AdjustPreviewToParent(bool Adjust);

    // Slots
public slots:
    void OnLineEditTextEdited();
    void OnBrowseButtonClicked();

private:
    void CreatePreviewPanel();
    void ShowPreviewPanel();
    void HidePreviewPanel();
};

#endif // WRESOURCESELECTOR_H
