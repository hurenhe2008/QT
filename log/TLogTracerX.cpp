#include "TLogTracerX.h"
#include <Windows.h>
#include <new>

#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QString>

#define  LOG_SAFE_DELETE(x)  do { if(x) { delete x; x = nullptr; } } while(0)

TLogTracerX& TLogTracerX::instance()
{
    static TLogTracerX self;
    return self;
}

TLogTracerX::TLogTracerX()
    : mbInit(false)
    , mLogFileMaxSize(10485760) /* 10*1024*1024 */
    , mLogFileCurSize(0)
    , mLogFileIndex(0)
    , mpFile(nullptr)
    , mpTextStream(nullptr)
    , mOriMsgHandler(nullptr)
{

}

TLogTracerX::~TLogTracerX()
{
    clear();
}

bool TLogTracerX::init(const QString &logdir, const QString &appname, 
    unsigned logfilemaxsize)
{
    if (mbInit) return true;

    mLogDir = logdir;
    mAppName = appname;
    mLogFileMaxSize = logfilemaxsize;

    if (mbInit = innerInit()) {
        /*save orignal console message handler*/
        mOriMsgHandler = qInstallMessageHandler(myMsgHandler);
    }

    return mbInit;
}

bool TLogTracerX::innerInit()
{
    if (mLogFileMaxSize <= 0) return false;

    ++mLogFileIndex;

    SYSTEMTIME sm;
    ::GetLocalTime(&sm);

    QString logfile = mLogDir + "\\" + mAppName +
        "_" + QString::number(mLogFileIndex) + "_" +
        QString("%1-%2-%3_%4-%5-%6-%7").arg(sm.wYear).arg(sm.wMonth).
        arg(sm.wDay).arg(sm.wHour).arg(sm.wMinute).arg(sm.wSecond).
        arg(sm.wMilliseconds) + ".log";

    QFile *oldFile = (mpFile) ? mpFile : nullptr;

    mpFile = new(std::nothrow) QFile(logfile);
    if (!mpFile || !mpFile->open(QFile::WriteOnly | QFile::Text)) {
        delete mpFile;
        mpFile = oldFile;
        return false;
    }

    if (!mpTextStream && 
        !(mpTextStream = new(std::nothrow) QTextStream(mpFile))) {
        return false;
    }
    else {
        mpTextStream->setDevice(mpFile);
    }
    
    delete oldFile;
    mLogFileCurSize = 0;

    return true;
}

void TLogTracerX::clear()
{
    LOG_SAFE_DELETE(mpTextStream);

    LOG_SAFE_DELETE(mpFile);
}

void TLogTracerX::myMsgHandler(QtMsgType type, const QMessageLogContext &ctx,
    const QString &msg)
{
    TLogTracerX::instance().handleMsg(type, ctx, msg);
}

void TLogTracerX::handleMsg(QtMsgType type, const QMessageLogContext &ctx,
    const QString &msg)
{
    const char *level = nullptr;
    bool bout_detail = false;

    switch (type)
    {
    case QtDebugMsg:
        level = "Debug";
        bout_detail = false;
        break;

    case QtInfoMsg:
        level = "Info";
        bout_detail = false;
        break;

    case QtWarningMsg:
        level = "Warning";
        bout_detail = true;
        break;

    case QtCriticalMsg:
        level = "Critical";
        bout_detail = true;
        break;

    case QtFatalMsg:
        level = "Fatal";
        bout_detail = true;
        break;

    default:
        level = "Debug";
        bout_detail = false;
        break;
    }

    SYSTEMTIME sm;
    GetLocalTime(&sm);

    QString strBuff;
    if (bout_detail) {
        strBuff = QString("{%1-%2-%3 %4:%5:%6.%7} [tid:%8] [%9]: %10"
            " --> {file:%11 line:%12 func:%13}").
            arg(sm.wYear).arg(sm.wMonth).arg(sm.wDay).arg(sm.wHour).
            arg(sm.wMinute).arg(sm.wSecond).arg(sm.wMilliseconds).
            arg(::GetCurrentThreadId()).arg(level).arg(msg).
            arg(ctx.file).arg(ctx.line).arg(ctx.function);
    }
    else {
        strBuff = QString("{%1-%2-%3 %4:%5:%6.%7} [tid:%8] [%9]: %10").
            arg(sm.wYear).arg(sm.wMonth).arg(sm.wDay).arg(sm.wHour).
            arg(sm.wMinute).arg(sm.wSecond).arg(sm.wMilliseconds).
            arg(::GetCurrentThreadId()).arg(level).arg(msg);
    }

    /*write file*/
    writeFile(type, strBuff);

    /*write stderr*/
    if (mOriMsgHandler) {
        mOriMsgHandler(type, ctx, msg);
    }
}

void TLogTracerX::writeFile(QtMsgType type, const QString &str)
{
    mFileMutex.lock();

    if (mLogFileCurSize + str.length() > mLogFileMaxSize) {
        if (!innerInit()) {
            //abort();
            return;
        }
    }

    if (mpTextStream) {     
#ifdef _DEBUG
        *mpTextStream << str << endl;
        mLogFileCurSize += str.length();
#else  /*RELEASE*/
        if (QtDebugMsg != type) {
            *mpTextStream << str << endl;
            mLogFileCurSize += str.length();
        }
#endif   
    }

    mFileMutex.unlock();
}