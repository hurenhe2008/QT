#include "TLogTracer.h"

#include <windows.h>

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QString>

#define  LOG_FILE_MAX_SIZE   10485760   /* 10*1024*1024: 10M */
#define  LOG_SAFE_DELETE(x)  do { if(x) { delete x; x = nullptr; } } while(0)

TLogTracer& TLogTracer::instance()
{
    static TLogTracer self;
    return self;
}

TLogTracer::TLogTracer()
    : mpFile(nullptr)
    , mpTextStream(nullptr)
    , mOriMsgHandler(nullptr)
{

}

TLogTracer::~TLogTracer()
{
    clear();
}

bool TLogTracer::init(const QString &logpath)
{
    if (mpFile && mpTextStream) {
        return true;
    }

    if (!innerInit(logpath)) {
        clear();
        return false;
    }

    /*save orignal console message handler*/
    mOriMsgHandler = qInstallMessageHandler(myMsgHandler);
    return true;
}

bool TLogTracer::innerInit(const QString &logpath)
{
    if (!(mpFile = new QFile(logpath))){
        return false;
    }

    QIODevice::OpenMode mode = QFile::WriteOnly | QFile::Text;
    if (mpFile->exists()) {  /*file exist*/
        if (mpFile->size() > LOG_FILE_MAX_SIZE) {
            mode = mode | QFile::Truncate;
        }
        else {
            mode = mode | QFile::Append;
        }
    }
    if (!mpFile->open(mode)) {
        return false;
    }

    if (!(mpTextStream = new QTextStream(mpFile))) {
        return false;
    }

    return true;
}

void TLogTracer::clear()
{
    LOG_SAFE_DELETE(mpTextStream);

    LOG_SAFE_DELETE(mpFile);
}

void TLogTracer::myMsgHandler(QtMsgType type, const QMessageLogContext &ctx,
    const QString &msg)
{
    TLogTracer::instance().handleMsg(type, ctx, msg);
}

void TLogTracer::handleMsg(QtMsgType type, const QMessageLogContext &ctx,
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
    ::GetLocalTime(&sm);

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

    if (mpTextStream) {   
        mpTextStreamMutex.lock();
#ifdef _DEBUG
       *mpTextStream << strBuff << endl;
#else  /*RELEASE*/
        if (QtDebugMsg != type) {
            *mpTextStream << strBuff << endl;
        }
#endif 
        mpTextStreamMutex.unlock();
    }
    
    /*output to original console: stderr*/
    if (mOriMsgHandler) {  
        mOriMsgHandler(type, ctx, msg);
    }
}



