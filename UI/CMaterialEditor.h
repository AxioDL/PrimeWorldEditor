#ifndef CMATERIALEDITOR_H
#define CMATERIALEDITOR_H

#include <QDialog>

namespace Ui {
class CMaterialEditor;
}

class CMaterialEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CMaterialEditor(QWidget *parent = 0);
    ~CMaterialEditor();

private:
    Ui::CMaterialEditor *ui;
};

#endif // CMATERIALEDITOR_H
