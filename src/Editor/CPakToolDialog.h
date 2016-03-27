#ifndef CPAKTOOLDIALOG
#define CPAKTOOLDIALOG

#include <Common/Log.h>
#include <Common/types.h>
#include <Core/Resource/EGame.h>
#include "Editor/UICommon.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProgressDialog>
#include <QTimer>

class CPakToolDialog : public QProgressDialog
{
    Q_OBJECT

public:
    enum EResult { eSuccess, eError, eUserCancelled };

private:
    enum { eExtract, eRepack, eListDump } mMode;
    QProcess *mpPakTool;
    QString mPakFilename;
    QString mListFilename;
    QString mFolderPath;
    EGame mGame;
    EResult mResult;

    QString mPartialString;
    bool mUpdating;

    int mCur;
    int mMax;
    bool mSetMax;

    // Private Functions
    CPakToolDialog(QWidget *pParent = 0)
        : QProgressDialog(pParent)
        , mpPakTool(nullptr)
        , mSetMax(false)
        , mUpdating(false)
    {
    }

    virtual QSize sizeHint() const
    {
        QSize Size = QProgressDialog::sizeHint();
        Size.rwidth() *= 3;
        return Size;
    }

    void Run()
    {
        mpPakTool = new QProcess(this);
        QStringList Args;

        if (mMode == eExtract)
        {
            Args << "-x" << mPakFilename;
            setLabelText("Extracting...");
        }
        else if (mMode == eRepack)
        {
            Args << "-r" << GameString(mGame) << mFolderPath << mPakFilename << mListFilename;
            setLabelText("Repacking...");
        }
        else if (mMode == eListDump)
            Args << "-d" << mPakFilename;

        setWindowTitle(labelText());

        // List dump is really fast, so showing progress dialog isn't necessary for it.
        if (mMode != eListDump)
        {
            connect(mpPakTool, SIGNAL(readyReadStandardOutput()), this, SLOT(ReadStdOut()));
            connect(mpPakTool, SIGNAL(finished(int)), this, SLOT(PakToolFinished(int)));
            mpPakTool->start("PakTool.exe", Args);
            exec();
        }

        else
        {
            mpPakTool->start("PakTool.exe", Args);
            mpPakTool->waitForFinished(-1);
        }
    }

    EResult Result() const
    {
        if (wasCanceled()) return eUserCancelled;
        else return (EResult) result();
    }

private slots:
    void ReadStdOut()
    {
        if (mUpdating) return;
        mUpdating = true;

        QString StdOut = mpPakTool->readAllStandardOutput();
        QStringList Strings = StdOut.split(QRegExp("[\\r\\n]"), QString::SkipEmptyParts);

        if (!Strings.isEmpty())
        {
            Strings.front().prepend(mPartialString);

            QCharRef LastChar = StdOut[StdOut.size() - 1];
            if (LastChar != '\n' && LastChar != '\r')
            {
                mPartialString = Strings.back();

                if (Strings.size() > 1)
                    Strings.pop_back();
            }
            else
                mPartialString = "";

            const QRegExp kExtracting("^Extracting file ([0-9]{1,6}) of ([0-9]{1,6})");
            const QRegExp kRepacking("^Repacking file ([0-9]{1,6}) of ([0-9]{1,6})");
            const QRegExp& rkRegExp = (mMode == eExtract ? kExtracting : kRepacking);

            int Result = rkRegExp.indexIn(Strings.last());

            if (Result != -1)
            {
                mCur = rkRegExp.cap(1).toInt();
                mMax = rkRegExp.cap(2).toInt();

                if (!mSetMax)
                {
                    setMinimum(0);
                    setMaximum(mMax);
                    mSetMax = true;
                }

                // Deferring UI updates is necessary because trying to do them on this thread causes crashes for a lot of people
                QTimer::singleShot(0, this, SLOT(UpdateUI()));
            }
        }

        mUpdating = false;
    }

    void UpdateUI()
    {
        setLabelText(QString("%1 file %2 of %3...").arg(mMode == eExtract ? "Extracting" : "Repacking").arg(mCur).arg(mMax));
        setWindowTitle(labelText());
        setValue(mCur);
    }

    void PakToolFinished(int ExitCode)
    {
        while (mUpdating) {}
        done(ExitCode == 0 ? eSuccess : eError);
    }

public:
    static EResult Extract(QString PakFilename, QString *pOutFolder = 0, QWidget *pParent = 0)
    {
        Log::Write("Extracting pak " + TO_TSTRING(PakFilename));

        CPakToolDialog Dialog(pParent);
        Dialog.mMode = eExtract;
        Dialog.mPakFilename = PakFilename;
        Dialog.Run();
        EResult Result = Dialog.Result();

        if (pOutFolder)
        {
            QFileInfo Pak(PakFilename);
            *pOutFolder = Pak.absolutePath() + '/' + Pak.baseName() + "-pak/";
        }

        Log::Write("Result: " + TO_TSTRING(ResultString(Result)));
        return Result;
    }

    static EResult Repack(EGame Game, QString TargetPak, QString ListFile, QString FolderPath, QString *pOutPak = 0, QWidget *pParent = 0)
    {
        Log::Write("Repacking folder " + TO_TSTRING(FolderPath) + " into pak " + TO_TSTRING(TargetPak));

        CPakToolDialog Dialog(pParent);
        Dialog.mMode = eRepack;
        Dialog.mPakFilename = TargetPak;
        Dialog.mListFilename = ListFile;
        Dialog.mFolderPath = FolderPath;
        Dialog.mGame = Game;
        Dialog.Run();
        EResult Result = Dialog.Result();

        if (pOutPak)
            *pOutPak = TargetPak;

        Log::Write("Result: " + TO_TSTRING(ResultString(Result)));
        return Result;
    }

    static EResult DumpList(QString PakFilename, QString *pOutTxt = 0, QWidget *pParent = 0)
    {
        Log::Write("Dumping file list for pak " + TO_TSTRING(PakFilename));

        CPakToolDialog Dialog(pParent);
        Dialog.mMode = eListDump;
        Dialog.mPakFilename = PakFilename;
        Dialog.Run();
        EResult Result = Dialog.Result();

        if (pOutTxt)
        {
            QFileInfo Pak(PakFilename);
            *pOutTxt = Pak.absolutePath() + '/' + Pak.baseName() + "-pak.txt";
        }

        Log::Write("Result: " + TO_TSTRING(ResultString(Result)));
        return Result;
    }

    static QString TargetPakForFolder(QString Folder)
    {
        QDir Dir(Folder);
        QString PakName = Dir.dirName();
        if (PakName.endsWith("-pak")) PakName.chop(4);
        Dir.cdUp();
        QString DirName = Dir.absolutePath();

        QString PakPath = DirName + '/' + PakName + ".pak";

        if (QFile::exists(PakPath))
            return PakPath;
        else
            return "";
    }

    static QString TargetListForFolder(QString Folder)
    {
        QDir Dir(Folder);
        QString TxtName = Dir.dirName();
        Dir.cdUp();
        QString DirName = Dir.absolutePath();

        QString ListPath = DirName + '/' + TxtName + ".txt";

        if (QFile::exists(ListPath))
            return ListPath;
        else
            return "";
    }

    static QString GameString(EGame Game)
    {
        switch (Game)
        {
        case ePrimeDemo:        return "mp1demo";
        case ePrime:            return "mp1";
        case eEchoesDemo:       return "mp2demo";
        case eEchoes:           return "mp2";
        case eCorruptionProto:  return "mp3proto";
        case eCorruption:       return "mp3";
        case eReturns:          return "dkcr";
        default:                return "INVALID";
        }
    }

    static QString ResultString(EResult Result)
    {
        switch (Result)
        {
        case eSuccess:          return "Success";
        case eError:            return "Error";
        case eUserCancelled:    return "User Cancelled";
        default:                return "";
        }
    }
};

#endif // CPAKTOOLDIALOG

