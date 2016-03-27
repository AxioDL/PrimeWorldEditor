#ifndef WANIMPARAMSEDITOR_H
#define WANIMPARAMSEDITOR_H

#include "WIntegralSpinBox.h"
#include "WResourceSelector.h"
#include <Core/Resource/CAnimationParameters.h>

#include <QWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVector>

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
    WAnimParamsEditor(const CAnimationParameters& rkParams, QWidget *pParent = 0);
    ~WAnimParamsEditor();
    void SetTitle(const QString& rkTitle);
    void SetParameters(const CAnimationParameters& rkParams);

signals:
    void ParametersChanged(const CAnimationParameters& rkParams);

private slots:
    void OnResourceChanged(QString Path);
    void OnCharacterChanged(int Index);
    void OnUnknownChanged();

private:
    void SetupUI();
};

#endif // WANIMPARAMSEDITOR_H
