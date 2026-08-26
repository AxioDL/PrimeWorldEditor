#ifndef CRESOURCESELECTOR_H
#define CRESOURCESELECTOR_H

#include <Core/Resource/CResTypeFilter.h>
#include <QWidget>

class CResource;
class CResourceEntry;
class QFrame;
class QHBoxLayout;
class QPushButton;
class QVBoxLayout;

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
    ~CResourceSelector() override;

    void SetFrameVisible(bool Visible);
    void SetEditable(bool Editable);
    void SetTypeFilter(const CResTypeFilter& rkFilter);
    void SetTypeFilter(EGame Game, const TString& rkTypeList);
    void SetResource(const CAssetID& rkID);
    void SetResource(CResource* pRes);
    void SetResourceEntry(CResourceEntry* pEntry);

    // Interface
    bool eventFilter(QObject *pWatched, QEvent *pEvent) override;

    // Accessors
    CResourceEntry* Entry() const            { return mpResEntry; }
    const CResTypeFilter& TypeFilter() const { return mTypeFilter; }
    bool IsEditable() const                  { return mIsEditable; }

signals:
    void ResourceChanged(CResourceEntry* pNewRes);

public slots:
    void Clear();

protected:
    // Drag
    void mousePressEvent(QMouseEvent* pEvent) override;
    void mouseMoveEvent(QMouseEvent* pEvent) override;
    void mouseReleaseEvent(QMouseEvent* pEvent) override;

    // Drop
    void dragEnterEvent(QDragEnterEvent* pEvent) override;
    void dropEvent(QDropEvent* pEvent) override;

private slots:
    void CreateContextMenu(const QPoint& rkPoint);
    void Select();
    void Find();
    void EditAsset();
    void CopyName();
    void CopyPath();
    void OnResourceChanged();
    void OnResourceMoved(CResourceEntry *pEntry);
    void UpdateUI();
};

#endif // CRESOURCESELECTOR_H
