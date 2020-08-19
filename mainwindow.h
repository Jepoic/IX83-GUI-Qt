#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DLL.h>
#include <QCloseEvent>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

extern Ui::MainWindow *uiself;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr,
               ptr_Initialize init = nullptr,
               ptr_EnumInterface enumIf = nullptr,
               ptr_GetInterfaceInfo getInfo = nullptr,
               ptr_OpenInterface openIf = nullptr,
               ptr_SendCommand sendCmd = nullptr,
               ptr_RegisterCallback reCb = nullptr,
               ptr_CloseInterface closeIf = nullptr);
    ~MainWindow();

    void closeEvent(QCloseEvent *event);
    bool quitSymbol = false;

    QLabel* ocWidget;

    void *pInterface;
    ptr_Initialize init;
    ptr_EnumInterface enumIf;
    ptr_GetInterfaceInfo getInfo;
    ptr_OpenInterface openIf;
    ptr_SendCommand sendCmd;
    ptr_RegisterCallback reCb;
    ptr_CloseInterface closeIf;

    MDK_MSL_CMD	m_Cmd;

    // in order to visit pointer to Ui (private) without passing an pointer to Ui::MainWindow,
    // which is not allowed by Olympus SDK Library
    friend int CALLBACK CommandCallback(ULONG MsgId, ULONG wParam, ULONG lParam, PVOID pv, PVOID pContext, PVOID pCaller);
    friend int CALLBACK NotifyCallback(ULONG MsgId, ULONG wParam, ULONG lParam, PVOID pv, PVOID pContext, PVOID pCaller);
    friend int CALLBACK ErrorCallback(ULONG MsgId, ULONG wParam, ULONG lParam, PVOID pv, PVOID pContext, PVOID pCaller);


private slots:
    void receivePointer(void*);
    void receiveQuitSymbol(bool);
    void defaultSettings(bool a, bool b);
    void closeInterface();
    bool SendCMD(QString cmd);

    void on_actionSelection_triggered();

    void on_closeBtn_clicked();

    void on_loginBtn_clicked();

    void on_sendBtn_clicked();

    void on_fineBtn_clicked();

    void on_NFPBtn_clicked();

    void on_setBtn_clicked();

    void on_resetBtn_clicked();

    void on_escapeBtn_clicked();

    void on_setBtn_2_clicked();

    void on_resetBtn_2_clicked();

    void on_zSlider_sliderReleased();

    void on_doubleSpinBox_valueChanged();

    void on_syncBtn_clicked();

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
