#ifndef WCREATETAB_H
#define WCREATETAB_H

#include <QWidget>

namespace Ui {
class WCreateTab;
}

class WCreateTab : public QWidget
{
    Q_OBJECT

public:
    explicit WCreateTab(QWidget *parent = 0);
    ~WCreateTab();

private:
    Ui::WCreateTab *ui;
};

#endif // WCREATETAB_H
