#ifndef CTEMPLATEEDITDIALOG_H
#define CTEMPLATEEDITDIALOG_H

#include "Editor/CPropertyNameValidator.h"
#include <Core/Resource/Script/IPropertyTemplate.h>
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

    IPropertyTemplate *mpTemplate;
    EGame mGame;

    TString mOriginalName;
    TString mOriginalDescription;
    bool mOriginalNameWasValid;

    // These members help track what templates need to be updated and resaved after the user clicks OK
    QVector<CScriptTemplate*> mScriptTemplatesToResave;
    QVector<CStructTemplate*> mStructTemplatesToResave;
    QVector<IPropertyTemplate*> mEquivalentProperties;

public:
    CTemplateEditDialog(IPropertyTemplate *pTemplate, QWidget *pParent = 0);
    ~CTemplateEditDialog();

public slots:
    void ApplyChanges();

protected:
    void AddTemplate(IPropertyTemplate *pTemp);
    void UpdateDescription(const TString& rkNewDesc);
    void FindEquivalentProperties(IPropertyTemplate *pTemp);
};

#endif // CTEMPLATEEDITDIALOG_H
