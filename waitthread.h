#ifndef WAITTHREAD_H
#define WAITTHREAD_H

#include <QThread>
#include <cmd.h>

class WaitThread : public QThread
{
    Q_OBJECT
public:
    explicit WaitThread(QObject *parent = nullptr);

protected:
    void run();

private slots:
    void receiveMode(int);
    void receiveSuccessed();

signals:
    void sendMode(int);

private:
    volatile bool successed;
    volatile int mode;

};

#endif // WAITTHREAD_H
