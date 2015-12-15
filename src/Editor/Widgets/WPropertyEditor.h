#ifndef WPROPERTYEDITOR_H
#define WPROPERTYEDITOR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <Resource/script/CProperty.h>

class WPropertyEditor : public QWidget
{
    Q_OBJECT

    // Editor
    CPropertyBase *mpProperty;

    // UI
    struct {
        QLabel *PropertyName;
        QWidget *EditorWidget;
        QHBoxLayout *Layout;
    } mUI;

public:
    explicit WPropertyEditor(QWidget *pParent = 0, CPropertyBase *pProperty = 0);
    ~WPropertyEditor();
    void resizeEvent(QResizeEvent *pEvent);

    void SetProperty(CPropertyBase *pProperty);

private:
    void CreateEditor();
    void UpdateEditor();
    void CreateLabelText();
};

#endif // WPROPERTYEDITOR_H
