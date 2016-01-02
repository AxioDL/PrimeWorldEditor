#ifndef WPROPERTYEDITOR_H
#define WPROPERTYEDITOR_H

#include <Core/Resource/Script/IProperty.h>
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

class WPropertyEditor : public QWidget
{
    Q_OBJECT

    // Editor
    IProperty *mpProperty;

    // UI
    struct {
        QLabel *PropertyName;
        QWidget *EditorWidget;
        QHBoxLayout *Layout;
    } mUI;

public:
    explicit WPropertyEditor(QWidget *pParent = 0, IProperty *pProperty = 0);
    ~WPropertyEditor();
    void resizeEvent(QResizeEvent *pEvent);

    void SetProperty(IProperty *pProperty);

private:
    void CreateEditor();
    void UpdateEditor();
    void CreateLabelText();
};

#endif // WPROPERTYEDITOR_H
