#ifndef WANIMPARAMSEDITOR_H
#define WANIMPARAMSEDITOR_H

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVector>

#include "WIntegralSpinBox.h"
#include "WResourceSelector.h"
#include <Resource/CAnimationParameters.h>

class WAnimParamsEditor : public QWidget
{
    Q_OBJECT
    CAnimationParameters mParams;

    QGroupBox *mpGroupBox;
    QVBoxLayout *mpGroupLayout;

    QHBoxLayout *mpValueLayouts[5];
    QLabel *mpLabels[5];
    WResourceSelector *mpSelector;
    QComboBox *mpCharComboBox;
    WIntegralSpinBox *mpSpinBoxes[4];
    QVector<QObject*> mLayoutWidgets;

public:
    WAnimParamsEditor(QWidget *pParent = 0);
    WAnimParamsEditor(const CAnimationParameters& params, QWidget *pParent = 0);
    ~WAnimParamsEditor();
    void SetTitle(const QString& title);
    void SetParameters(const CAnimationParameters& params);

signals:
    void ParametersChanged(const CAnimationParameters& params);

private slots:
    void OnResourceChanged(QString path);
    void OnCharacterChanged(int index);
    void OnUnknownChanged();

private:
    void SetupUI();
};

#endif // WANIMPARAMSEDITOR_H
