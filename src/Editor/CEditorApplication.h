#ifndef CEDITORAPPLICATION_H
#define CEDITORAPPLICATION_H

#include <QApplication>
#include <QTimer>
#include <QVector>

class CBasicViewport;
class IEditor;

class CEditorApplication : public QApplication
{
    Q_OBJECT

    QTimer mRefreshTimer;
    QVector<IEditor*> mEditorWindows;
    double mLastUpdate;

public:
    CEditorApplication(int& rArgc, char **ppArgv);

public slots:
    void AddEditor(IEditor *pEditor);
    void TickEditors();
    void OnEditorClose();
};

#define gpEdApp static_cast<CEditorApplication*>(qApp)

#endif // CEDITORAPPLICATION_H
