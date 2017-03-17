
#include <QCoreApplication>
#include <Windows.h>
#include <process.h>

#include "TLogTracer.h"
#include "TlogTracerX.h"

static int mainfun(int argc, char *argv[]);
static void test_logtracer();
static void test_logtracer_x();

static unsigned __stdcall log_thread_proc(void *);

int main(int argc, char *argv[])
{
    int hr = 0;

    __try {
        hr = mainfun(argc, argv);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

    }

    return hr;
}

int mainfun(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

#if 0
    test_logtracer();
#else 
    test_logtracer_x();
#endif

    return app.exec();
}

static void test_logtracer()
{
    if (!TLogTracer::instance().init("test.log")) {
        qDebug() << "TLogTracer init failed";
        abort();
    }

    qDebug() << "TLogTracer init success";
    qDebug() << "hello, world";
}

static void test_logtracer_x()
{
    if (!TLogTracerX::instance().init(".", "test_qt_log", 4000)) {
        qDebug() << "TLogTracerX init failed";
        abort();
    }

    unsigned i = 0; 
    while (i++ < 3) {
        /*not handle the memory for now*/
        unsigned *p = new(std::nothrow) unsigned(i);  
        HANDLE hthread = (HANDLE)_beginthreadex(nullptr, 0, 
            log_thread_proc, (void *)p, 0, nullptr);
        if (NULL == hthread) {
            qDebug("create log thread %d failed", i);
            abort();
        }
    }
    
    while (true) {
        qInfo() << "main thread test qInfo";
        Sleep(100);
        qDebug() << "main thread test qDebug";
        Sleep(100);
        qWarning() << "main thread test qWarning";
        Sleep(100);
        qCritical() << "main thread test qCritical";
        Sleep(100);
    }
    
    qDebug() << "TLogTracer init success";
    qDebug() << "hello, world";
}

unsigned __stdcall log_thread_proc(void *data)
{ 
    unsigned *p = (unsigned *)data;
    unsigned seq = *((unsigned *)data);

    while (true) {
        qDebug("thread %d input log test count: %d", seq,(*p)++);
        Sleep(100);
    }
}