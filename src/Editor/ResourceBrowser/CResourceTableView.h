#ifndef CRESOURCETABLEVIEW_H
#define CRESOURCETABLEVIEW_H

#include <QTableView>

class CResourceTableView : public QTableView
{
    Q_OBJECT
    QAction *mpRenameAction;

public:
    explicit CResourceTableView(QWidget *pParent = 0);
    void dragEnterEvent(QDragEnterEvent *pEvent);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);

public slots:
    void RenameSelected();
};

#endif // CRESOURCETABLEVIEW_H
