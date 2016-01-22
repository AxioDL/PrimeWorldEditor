#ifndef TWIDGETWRAPPER
#define TWIDGETWRAPPER

#include <QModelIndex>
#include <QWidget>

// Small class that associates a QWidget with a QModelIndex and relays widget signals back to the delegate
// so that property edits can be reflected in other parts of the application in realtime instead of when the
// widget is done being edited.
class CPropertyRelay : public QObject
{
    Q_OBJECT

    QModelIndex mIndex;
    QWidget *mpWidget;

public:
    CPropertyRelay(QWidget *pWidget, const QModelIndex& rkIndex)
        : QObject(pWidget), mIndex(rkIndex), mpWidget(pWidget) {}

public slots:
    void OnWidgetEdited() { emit WidgetEdited(mpWidget, mIndex); }

signals:
    void WidgetEdited(QWidget *pWidget, const QModelIndex& rkIndex);
};

#endif // TWIDGETWRAPPER

