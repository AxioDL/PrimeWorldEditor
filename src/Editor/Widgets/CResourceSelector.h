#ifndef CRESOURCESELECTOR
#define CRESOURCESELECTOR

#include <Core/GameProject/CResourceEntry.h>
#include <Core/Resource/CResTypeFilter.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class CResourceSelector : public QWidget
{
    Q_OBJECT

    CResourceEntry *mpResEntry = nullptr;
    CResTypeFilter mTypeFilter;
    bool mIsEditable = true;

    // UI
    QVBoxLayout *mpLayout;
    QHBoxLayout *mpFrameLayout;
    QFrame *mpFrame;
    QPushButton *mpResNameButton;
    QPushButton *mpSelectButton;
    QPushButton *mpClearButton;

    // Context Menu
    QAction *mpEditAssetAction;
    QAction *mpCopyNameAction;
    QAction *mpCopyPathAction;

    // Drag and Drop
    bool mIsDragging = false;
    QPoint mDragStartPosition;

public:
    explicit CResourceSelector(QWidget *pParent = nullptr);

    void SetFrameVisible(bool Visible);
    void SetEditable(bool Editable);
    void SetTypeFilter(const CResTypeFilter& rkFilter);
    void SetTypeFilter(EGame Game, const TString& rkTypeList);
    void SetResource(const CAssetID& rkID);
    void SetResource(CResourceEntry *pEntry);
    void SetResource(CResource *pRes);

    // Interface
    bool eventFilter(QObject *pWatched, QEvent *pEvent) override;

    // Drag
    void mousePressEvent(QMouseEvent *pEvent) override;
    void mouseMoveEvent(QMouseEvent *pEvent) override;
    void mouseReleaseEvent(QMouseEvent *pEvent) override;

    // Drop
    void dragEnterEvent(QDragEnterEvent *pEvent) override;
    void dropEvent(QDropEvent *pEvent) override;

    // Accessors
    CResourceEntry* Entry() const            { return mpResEntry; }
    const CResTypeFilter& TypeFilter() const { return mTypeFilter; }
    bool IsEditable() const                  { return mIsEditable; }

public slots:
    void CreateContextMenu(const QPoint& rkPoint);
    void Select();
    void Find();
    void Clear();
    void EditAsset();
    void CopyName();
    void CopyPath();
    void OnResourceChanged();
    void OnResourceMoved(CResourceEntry *pEntry);
    void UpdateUI();

signals:
    void ResourceChanged(CResourceEntry *pNewRes);
};

#endif // CRESOURCESELECTOR

