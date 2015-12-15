#ifndef WROLLOUT_H
#define WROLLOUT_H

#include <QPushButton>
#include <QWidget>

class WRollout : public QWidget
{
    Q_PROPERTY(bool mCollapsed READ isCollapsed WRITE setCollapsed)
    QPushButton *mpTopButton;
    QWidget *mpContainerWidget;
    bool mCollapsed;

public:
    explicit WRollout(QWidget *parent = 0);
    ~WRollout();
    void setCollapsed(bool collapsed);
    bool isCollapsed();
    void setName(const QString& name);
    QString getName();

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:
    bool mouseInButton(int x, int y);
};

#endif // WROLLOUT_H
