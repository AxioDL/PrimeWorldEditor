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
    void TickEditors();

    // Accessors
public:
    inline void AddEditor(IEditor *pEditor)     { mEditorWindows << pEditor; }
    inline void RemoveEditor(IEditor *pEditor)  { mEditorWindows.removeOne(pEditor); }
};

#define gpEdApp static_cast<CEditorApplication*>(qApp)

#endif // CEDITORAPPLICATION_H
