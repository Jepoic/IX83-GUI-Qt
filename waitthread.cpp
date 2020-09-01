#include "waitthread.h"

WaitThread::WaitThread(QObject *parent) :
    QThread(parent)
{
    successed = false;
}

void WaitThread::run()
{
    while (!successed)
    {
        msleep(10);
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
