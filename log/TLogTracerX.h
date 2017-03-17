#ifndef _TLOGTRACERX_H_
#define _TLOGTRACERX_H_

#include <qlogging.h>
#include <qdebug.h>
#include <qstring.h>
#include <qmutex.h>

class QFile;
class QTextStream;

class TLogTracerX {
public:
    static TLogTracerX& instance();

    ~TLogTracerX();

    bool init(const QString &logdir, 
        const QString &appname,
        unsigned logfilemaxsize = 10485760);

    void handleMsg(QtMsgType type,
        const QMessageLogContext &context,
        const QString &msg);

private:
    static void myMsgHandler(QtMsgType type,
        const QMessageLogContext &context,
        const QString &msg);

    bool innerInit();

    void writeFile(QtMsgType type, const QString &str);

    void clear();

private:
    TLogTracerX();
    TLogTracerX(const TLogTracerX &) = delete;
    TLogTracerX & operator = (const TLogTracerX &) = delete;

private:
    bool              mbInit;
    unsigned          mLogFileMaxSize;
    unsigned          mLogFileCurSize;
    unsigned          mLogFileIndex;

    QFile*            mpFile;
    QMutex            mFileMutex;
    QTextStream*      mpTextStream;
    QtMessageHandler  mOriMsgHandler;
    QString           mLogDir; 
    QString           mAppName;
};


#endif //_TLOGTRACERX_H_
