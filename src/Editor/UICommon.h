#ifndef UICOMMON
#define UICOMMON

#include "CEditorApplication.h"
#include <Common/TString.h>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMap>
#include <QMessageBox>
#include <QString>

// App string variable handling - automatically fill in application name/version
#define UI_APPVAR_NAME          "%APP_NAME%"
#define UI_APPVAR_FULLNAME      "%APP_FULL_NAME%"
#define UI_APPVAR_VERSION       "%APP_VERSION%"

#define REPLACE_APPVARS(InQString) \
    InQString.replace(UI_APPVAR_NAME, APP_NAME); \
    InQString.replace(UI_APPVAR_FULLNAME, APP_FULL_NAME); \
    InQString.replace(UI_APPVAR_VERSION, APP_VERSION);

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

//@todo This name isn't ideal, too similar to ToWChar and so might cause confusion
#define TO_WCHAR(Str)       ToWChar( UICommon::ToT16String(Str) )

#include <nod/Util.hpp>

#if NOD_UCS2
#define QStringToNodString(string) TO_WCHAR(string)
#else
#define QStringToNodString(string) string.toUtf8().data()
#endif

namespace UICommon
{

// Utility
QWindow* FindWidgetWindowHandle(QWidget* pWidget);
void OpenContainingFolder(QWidget* parent, const QString& pathIn);
bool OpenInExternalApplication(const QString& rkPath);

// Searches the widget's ancestry tree to find an ancestor of type ObjectT.
// ObjectT must be a QObject subclass.
template<typename ObjectT>
ObjectT* FindAncestor(QObject* pObject)
{
    for (QObject* pParent = pObject->parent(); pParent; pParent = pParent->parent())
    {
        ObjectT* pCasted = qobject_cast<ObjectT*>(pParent);

        if (pCasted)
        {
            return pCasted;
        }
    }
    return nullptr;
}

// TString/TWideString <-> QString
inline QString ToQString(const TString& rkStr)
{
    return QString::fromUtf8(*rkStr);
}

inline QString ToQString(const T16String& rkStr)
{
    return QString::fromUtf16(*rkStr);
}

inline TString ToTString(const QString& rkStr)
{
    return TString(rkStr.toStdString());
}

inline T16String ToT16String(const QString& rkStr)
{
    return T16String(rkStr.toStdU16String());
}

// QFileDialog wrappers
// Note: pause editor ticks while file dialogs are open because otherwise there's a bug that makes it really difficult to tab out and back in
#define PUSH_TICKS_ENABLED \
    bool TicksEnabled = gpEdApp->AreEditorTicksEnabled(); \
    gpEdApp->SetEditorTicksEnabled(false);
#define POP_TICKS_ENABLED \
    gpEdApp->SetEditorTicksEnabled(TicksEnabled);

inline QString OpenFileDialog(QWidget *pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = "")
{
    PUSH_TICKS_ENABLED;
    QString Result = QFileDialog::getOpenFileName(pParent, rkCaption, rkStartingDir, rkFilter);
    POP_TICKS_ENABLED;
    return Result;
}

inline QStringList OpenFilesDialog(QWidget *pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = "")
{
    PUSH_TICKS_ENABLED;
    QStringList Result = QFileDialog::getOpenFileNames(pParent, rkCaption, rkStartingDir, rkFilter);
    POP_TICKS_ENABLED;
    return Result;
}

inline QString SaveFileDialog(QWidget *pParent, const QString& rkCaption, const QString& rkFilter, const QString& rkStartingDir = "")
{
    PUSH_TICKS_ENABLED;
    QString Result = QFileDialog::getSaveFileName(pParent, rkCaption, rkStartingDir, rkFilter);
    POP_TICKS_ENABLED;
    return Result;
}

inline QString OpenDirDialog(QWidget *pParent, const QString& rkCaption, const QString& rkStartingDir = "")
{
    PUSH_TICKS_ENABLED;
    QString Result = QFileDialog::getExistingDirectory(pParent, rkCaption, rkStartingDir);
    POP_TICKS_ENABLED;
    return Result;
}

// QMessageBox wrappers
inline void InfoMsg(QWidget *pParent, QString InfoBoxTitle, QString InfoText)
{
    QMessageBox::information(pParent, InfoBoxTitle, InfoText);
}

inline void ErrorMsg(QWidget *pParent, QString ErrorText)
{
    QMessageBox::warning(pParent, "Error", ErrorText);
}

inline bool YesNoQuestion(QWidget *pParent, QString InfoBoxTitle, QString Question)
{
    QMessageBox::StandardButton Button = QMessageBox::question(pParent, InfoBoxTitle, Question, QMessageBox::Yes | QMessageBox::No);
    return Button == QMessageBox::Yes;
}

inline bool OpenProject()
{
    QWidget* pMainWindow = (QWidget*) gpEdApp->WorldEditor();
    QString ProjPath = UICommon::OpenFileDialog(pMainWindow, "Open Project", "Game Project (*.prj)");
    return ProjPath.isEmpty() ? false : gpEdApp->OpenProject(ProjPath);
}

// Constants
const QColor kImportantButtonColor(36, 100, 100);

} // UICommon Namespace End

#endif // UICOMMON

