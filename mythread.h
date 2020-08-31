#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr);
    void stop();

protected:
    void run();

private slots:
    void receiveSuccessed(bool);

private:
    volatile bool stopped;
    volatile bool successed;
};

#endif // MYTHREAD_H
