#include "mythread.h"
#include <QDebug>

MyThread::MyThread(QObject *parent) :
    QThread(parent)
{
    stopped = false;
    successed = false;
}

void MyThread::run()
{
    qreal i = 0;
    while (!successed)
    {
        qDebug() << QString("in MyThread: %1").arg(i);
        msleep(1000);
        i++;
    }
    successed = false;
}

void MyThread::stop()
{
    stopped = true;
}

void MyThread::receiveSuccessed(bool ret)
{
    successed = ret;
}
