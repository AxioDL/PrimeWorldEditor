#ifndef WPROPERTYEDITOR_H
#define WPROPERTYEDITOR_H

#include <Core/Resource/Script/CProperty.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

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
