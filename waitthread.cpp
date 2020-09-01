#include "WaitThread.h"
#include <QDebug>

WaitThread::WaitThread(QObject *parent) :
    QThread(parent)
{
    successed = false;
}

void WaitThread::run()
{
    qreal i = 0;
    while (!successed)
    {
        qDebug() << QString("in WorkThread: %1").arg(i);
        msleep(1000);
        i++;
    }
    successed = false;
    emit sendMode(mode);
}

void WaitThread::receiveSuccessed()
{
    successed = true;
}

void WaitThread::receiveMode(int modeIn)
{
    mode = modeIn;
}
