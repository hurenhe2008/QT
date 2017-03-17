#ifndef __TLOGTRACER_H__
#define __TLOGTRACER_H__


#include <qlogging.h>
#include <qdebug.h>
#include <qmutex.h>

class QFile;
class QTextStream;

class TLogTracer
{
public:
    static TLogTracer& instance();
    ~TLogTracer();

    bool init(const QString &logpath); /*absolute path*/

    void handleMsg(QtMsgType type, 
        const QMessageLogContext &context, 
        const QString &msg);

private:
    static void myMsgHandler(QtMsgType type, 
        const QMessageLogContext &context,
        const QString &msg);

    bool innerInit(const QString &logpath);

    void clear();

private:
    TLogTracer();  
    TLogTracer(const TLogTracer &) = delete;
    TLogTracer & operator = (const TLogTracer &) = delete;

private:
    QFile*            mpFile;
    QTextStream*      mpTextStream;
    QMutex            mpTextStreamMutex;
    QtMessageHandler  mOriMsgHandler;
};

#endif //__TLOGTRACER_H__