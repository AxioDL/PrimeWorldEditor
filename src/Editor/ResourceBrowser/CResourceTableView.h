#ifndef CRESOURCETABLEVIEW_H
#define CRESOURCETABLEVIEW_H

#include <QTableView>

class CResourceTableView : public QTableView
{
    Q_OBJECT

public:
    explicit CResourceTableView(QWidget *pParent = 0);
    void dragEnterEvent(QDragEnterEvent *pEvent);
};

#endif // CRESOURCETABLEVIEW_H
