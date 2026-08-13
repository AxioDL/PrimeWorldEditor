#ifndef UICOMMON_H
#define UICOMMON_H

#include "Editor/CEditorApplication.h"
#include <Common/TString.h>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>

// App string variable handling - automatically fill in application name/version
#define UI_APPVAR_NAME          "%APP_NAME%"
#define UI_APPVAR_FULLNAME      "%APP_FULL_NAME%"
#define UI_APPVAR_VERSION       "%APP_VERSION%"

#define REPLACE_APPVARS(InQString) \
    InQString.replace(QStringLiteral(UI_APPVAR_NAME), QStringLiteral(APP_NAME)); \
    InQString.replace(QStringLiteral(UI_APPVAR_FULLNAME), QStringLiteral(APP_FULL_NAME)); \
    InQString.replace(QStringLiteral(UI_APPVAR_VERSION), QStringLiteral(APP_VERSION));

#define SET_WINDOWTITLE_APPVARS(InString) \
    { \
        QString APPVAR_MACRO_NEWTITLE = InString; \
        REPLACE_APPVARS(APPVAR_MACRO_NEWTITLE) \
        setWindowTitle(APPVAR_MACRO_NEWTITLE); \
    }

#define REPLACE_WINDOWTITLE_APPVARS \
    SET_WINDOWTITLE_APPVARS(windowTitle());

// Common conversion functions
#define TO_QSTRING(Str)     UICommon::ToQString(Str)
#define TO_TSTRING(Str)     UICommon::ToTString(Str)
#define TO_CCOLOR(Clr)      CColor::Integral(Clr.red(), Clr.green(), Clr.blue(), Clr.alpha())
#define TO_QCOLOR(Clr)      QColor(Clr.R * 255, Clr.G * 255, Clr.B * 255, Clr.A * 255)

namespace UICommon
{

// Utility
QWindow* FindWidgetWindowHandle(QWidget* pWidget);
void OpenContainingFolder(QWidget* parent, const QString& pathIn);
bool OpenInExternalApplication(const QString& rkPath);

// Searches the widget's ancestry tree to find an ancestor of type ObjectT.
// ObjectT must be a QObject subclass.
template <typename ObjectT>
ObjectT* FindAncestor(QObject* pObject)
{
    for (QObject* pParent = pObject->parent(); pParent; pParent = pParent->parent())
    {
        if (auto* pCasted = qobject_cast<ObjectT*>(pParent))
            return pCasted;
    }
    return nullptr;
}

// TString/TWideString <-> QString
inline QString ToQString(const TString& rkStr)
{
    return QString::fromStdString(rkStr.ToStdString());
}

inline QString ToQString(const T16String& rkStr)
{
    return QString::fromUtf16(rkStr.Data(), rkStr.Size());
}
inline QString ToQString(const std::string& str)
{
    return QString::fromStdString(str);
}

inline TString ToTString(const QString& rkStr)
{
    return TString(rkStr.toStdString());
}

// QFileDialog wrappers
// Note: pause editor ticks while file dialogs are open because otherwise there's a bug that makes it really difficult to tab out and back in
class CEditorTickDisabler
{
public:
    CEditorTickDisabler() : m_state{gpEdApp->AreEditorTicksEnabled()} {
        gpEdApp->SetEditorTicksEnabled(false);
    }
    ~CEditorTickDisabler() {
        gpEdApp->SetEditorTicksEnabled(m_state);
    }

private:
    bool m_state{};
};
inline QString OpenFileDialog(QWidget* pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = {})
{
    [[maybe_unused]] const auto disabler = CEditorTickDisabler();
    return QFileDialog::getOpenFileName(pParent, rkCaption, rkStartingDir, rkFilter);
}

inline QStringList OpenFilesDialog(QWidget* pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = {})
{
    [[maybe_unused]] const auto disabler = CEditorTickDisabler();
    return QFileDialog::getOpenFileNames(pParent, rkCaption, rkStartingDir, rkFilter);
}

inline QString SaveFileDialog(QWidget* pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = {})
{
    [[maybe_unused]] const auto disabler = CEditorTickDisabler();
    return QFileDialog::getSaveFileName(pParent, rkCaption, rkStartingDir, rkFilter);
}

inline QString OpenDirDialog(QWidget* pParent, const QString& rkCaption, const QString& rkStartingDir = {})
{
    [[maybe_unused]] const auto disabler = CEditorTickDisabler();
    return QFileDialog::getExistingDirectory(pParent, rkCaption, rkStartingDir);
}

// QMessageBox wrappers
inline void InfoMsg(QWidget *pParent, const QString& InfoBoxTitle, const QString& InfoText)
{
    QMessageBox::information(pParent, InfoBoxTitle, InfoText);
}

inline void ErrorMsg(QWidget *pParent, const QString& ErrorText)
{
    QMessageBox::warning(pParent, QCoreApplication::translate("ErrorMsg", "Error"), ErrorText);
}

inline bool YesNoQuestion(QWidget *pParent, const QString& InfoBoxTitle, const QString& Question)
{
    const auto Button = QMessageBox::question(pParent, InfoBoxTitle, Question, QMessageBox::Yes | QMessageBox::No);
    return Button == QMessageBox::Yes;
}

inline bool OpenProject()
{
    QWidget* pMainWindow = (QWidget*) gpEdApp->WorldEditor();
    QString ProjPath = UICommon::OpenFileDialog(pMainWindow, QCoreApplication::translate("OpenProject", "Open Project"), QStringLiteral("Game Project (*.prj)"));
    return ProjPath.isEmpty() ? false : gpEdApp->OpenProject(ProjPath);
}

// Constants
constexpr QColor kImportantButtonColor(36, 100, 100);

} // UICommon Namespace End

#endif // UICOMMON_H

