#ifndef PWEMATERIALEDITOR_H
#define PWEMATERIALEDITOR_H

#include <QDialog>

namespace Ui {
class PWEMaterialEditor;
}

class PWEMaterialEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PWEMaterialEditor(QWidget *parent = 0);
    ~PWEMaterialEditor();

private:
    Ui::PWEMaterialEditor *ui;
};

#endif // PWEMATERIALEDITOR_H
