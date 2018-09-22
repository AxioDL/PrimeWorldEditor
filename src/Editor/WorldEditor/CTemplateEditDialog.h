#ifndef CTEMPLATEEDITDIALOG_H
#define CTEMPLATEEDITDIALOG_H

#include "Editor/CPropertyNameValidator.h"
#include <Core/Resource/Script/Property/Properties.h>
#include <Core/Resource/Script/CMasterTemplate.h>
#include <QDialog>

namespace Ui {
class CTemplateEditDialog;
}

class CTemplateEditDialog : public QDialog
{
    Q_OBJECT
    Ui::CTemplateEditDialog* mpUI;
    CPropertyNameValidator* mpValidator;

    IPropertyNew *mpProperty;
    EGame mGame;

    TString mOriginalName;
    TString mOriginalDescription;
    bool mOriginalNameWasValid;

    // These members help track what templates need to be updated and resaved after the user clicks OK
    QVector<CScriptTemplate*> mScriptTemplatesToResave;
    QVector<CStructPropertyNew*> mStructTemplatesToResave;
    QVector<IPropertyNew*> mEquivalentProperties;

public:
    CTemplateEditDialog(IPropertyNew* pProperty, QWidget *pParent = 0);
    ~CTemplateEditDialog();

public slots:
    void ApplyChanges();

protected:
    void AddTemplate(IPropertyNew* pProperty);
    void UpdateDescription(const TString& rkNewDesc);
    void FindEquivalentProperties(IPropertyNew *pTemp);
};

#endif // CTEMPLATEEDITDIALOG_H
