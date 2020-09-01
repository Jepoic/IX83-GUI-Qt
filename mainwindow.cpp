#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ifselection.h>
#include <DLL.h>
#include <cmd.h>
#include <QMessageBox>
#include <QDebug>

Ui::MainWindow *uiself;
MainWindow *mwself;
static int escapeVal;
static bool isReset;
static bool isReset2;

MainWindow::MainWindow(QWidget *parent,
                       ptr_Initialize init,
                       ptr_EnumInterface enumIf,
                       ptr_GetInterfaceInfo getInfo,
                       ptr_OpenInterface openIf,
                       ptr_SendCommand sendCmd,
                       ptr_RegisterCallback reCb,
                       ptr_CloseInterface closeIf):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mwself = this;
    uiself = ui;

    //this->setAttribute(Qt::WA_QuitOnClose, false);

    // pass parameters
    this->pInterface = nullptr;
    this->init = init;
    this->enumIf = enumIf;
    this->getInfo = getInfo;
    this->openIf = openIf;
    this->sendCmd = sendCmd;
    this->reCb = reCb;
    this->closeIf = closeIf;

    // add Permanent Widget to StatusBar for displaying interface's status (open/close)
    ocWidget = new QLabel(this);
    ui->statusbar->addPermanentWidget(ocWidget);

    // create a new wait sub thread
    waitThread = new WaitThread(this);

    // callback emit signal entry
    connect(this, SIGNAL(sendSuccessed()), waitThread, SLOT(receiveSuccessed()));

    // emit mode signal, see cmd.h for MODE definition
    connect(this, SIGNAL(sendMode(int)), waitThread, SLOT(receiveMode(int)));

    // receive mode signal, then run the specific code block corresponding to mode
    connect(waitThread, SIGNAL(sendMode(int)), this, SLOT(receiveModeAndRun(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//	COMMAND: call back entry from SDK port manager
int CALLBACK CommandCallback(ULONG MsgId, ULONG wParam, ULONG lParam,
                             PVOID pv, PVOID pContext, PVOID pCaller)
{
    UNREFERENCED_PARAMETER(MsgId);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pCaller);
    UNREFERENCED_PARAMETER(pv);

    MainWindow	*dlg = (MainWindow *)pContext;
    if (dlg == nullptr)
        return 0;
    if (pContext != nullptr)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QString str = " > ";
        QString str2 = QString(QLatin1String((char *)dlg->m_Cmd.m_Rsp));
        str2.remove("\r\n", Qt::CaseInsensitive);
        str.append(str2);
        item->setText(str);
        uiself->listWidget->addItem(item);
        uiself->listWidget->setCurrentRow(uiself->listWidget->count() - 1);
        if (str.contains("+",Qt::CaseSensitive))
        {
            uiself->statusbar->showMessage("Command success.", 3000);
            if (!mwself->waitThread->isFinished())
                emit mwself->sendSuccessed();
        }

    }
    return 0;
}

//	NOTIFICATION: call back entry from SDK port manager
int	CALLBACK NotifyCallback(ULONG MsgId, ULONG wParam, ULONG lParam,
                            PVOID pv, PVOID pContext, PVOID pCaller)
{
    UNREFERENCED_PARAMETER(MsgId);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pCaller);
    UNREFERENCED_PARAMETER(pv);

    char *dlgn = (char *)pv;
    QString rsp(dlgn);
    QListWidgetItem *item = new QListWidgetItem;
    QString str = " > ";
    rsp.remove("\r\n", Qt::CaseInsensitive);
    str.append(rsp);
    item->setText(str);
    uiself->listWidget->addItem(item);
    uiself->listWidget->setCurrentRow(uiself->listWidget->count() - 1);
    if (rsp.contains("NFP",Qt::CaseSensitive))
    {
        rsp.remove("NFP ");
        double cvVal = rsp.toDouble() / 100;
        uiself->cvLabel->setText(QString::number(cvVal,10,2));
    }
    return 0;

}

//	ERROR NOTIFICATON: call back entry from SDK port manager
int	CALLBACK ErrorCallback(ULONG MsgId, ULONG wParam, ULONG lParam,
                           PVOID pv, PVOID pContext, PVOID pCaller)
{
    UNREFERENCED_PARAMETER(MsgId);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pCaller);
    UNREFERENCED_PARAMETER(pv);

    QMessageBox::warning(NULL, "Error", "Disconnect interface?");
    mwself->on_closeBtn_clicked();
    return 0;
}

// applicatioin exit inquiry & close interface
// the SDK library has a defect that if an interface is not closed,
// it will not open next time until rebooting OS.
void MainWindow::closeEvent ( QCloseEvent *e )
{
    if( QMessageBox::question(this,
                              "Quit",
                              "Are you sure to quit this application?",
                              QMessageBox::Yes, QMessageBox::No )
            == QMessageBox::Yes){
        if(ui->closeBtn->isEnabled())
        {
            sendCmdPackage("L 0,0");
            Sleep(3000);
        }
        closeIf(pInterface);
        e->accept();
        qApp->quit();
    }
    else
        e->ignore();
}

// slots function which receive a pointer to an interface address
// which is opened by IFSelection dialog
// then register callback functions
void MainWindow::receivePointer(void *pInterface_new)
{
    this->pInterface = pInterface_new;
    bool rc = this->reCb(this->pInterface, CommandCallback, NotifyCallback, ErrorCallback, this);
    if(!rc)
        QMessageBox::warning(NULL, "Failed to Register Callback",
                             "Unkown reason!\n"
                             "Maybe there is no Interface connected.");

    ocWidget->setText("An interface is open.");

    defaultSettings(false, true);
    ui->sendBtn->setEnabled(true);
    ui->loginBtn->setEnabled(true);
    ui->closeBtn->setEnabled(true);
}

// for fixing BUG 1
void MainWindow::receiveQuitSymbol(bool symbol)
{
    this->quitSymbol = symbol;
    return;
}

void MainWindow::defaultSettings(bool a, bool b)
{
    if (b)
        ui->listWidget->clear();

    ui->loginBtn->setEnabled(a);

    ui->closeBtn->setEnabled(a);

    ui->cmdLine->clear();
    ui->cmdLine->setEnabled(a);

    ui->sendBtn->setEnabled(a);

    ui->doubleSpinBox->setValue(0);
    ui->doubleSpinBox->setMaximum(10500);
    ui->doubleSpinBox->setSingleStep(0.1);
    ui->doubleSpinBox->setMinimum(0);
    ui->doubleSpinBox->setEnabled(a);
    ui->doubleSpinBox->setKeyboardTracking(false);

    ui->NFPBtn->setEnabled(a);

    ui->label_6->setVisible(false);
    ui->cvLabel->setVisible(false);
    ui->syncBtn->setVisible(false);
    ui->syncBtn->setEnabled(false);

    ui->escapeBtn->setEnabled(a);

    ui->fineBtn->setEnabled(a);

    ui->maxLine->setText("10500");
    ui->maxLine->setEnabled(a);

    ui->stepLine->setText("0.1");
    ui->stepLine->setEnabled(false);

    ui->minLine->setText("0");
    ui->minLine->setEnabled(a);

    ui->setBtn->setEnabled(a);

    ui->resetBtn->setEnabled(false);

    ui->initLine->setText("700");
    ui->initLine->setEnabled(a);

    ui->achievedLine->setText("3000");
    ui->achievedLine->setEnabled(a);

    ui->acceleratedLine->setText("60");
    ui->acceleratedLine->setEnabled(a);

    ui->setBtn_2->setEnabled(a);

    ui->resetBtn_2->setEnabled(false);

    ui->zSlider->setValue(0);
    ui->zSlider->setMaximum(105000);
    ui->zSlider->setEnabled(a);
}

void MainWindow::waitSettings(bool a)
{
    ui->loginBtn->setEnabled(a);
    ui->closeBtn->setEnabled(a);
    ui->cmdLine->setEnabled(a);
    ui->sendBtn->setEnabled(a);
    ui->doubleSpinBox->setEnabled(a);
    ui->NFPBtn->setEnabled(a);
    ui->syncBtn->setEnabled(a);
    ui->escapeBtn->setEnabled(a);
    ui->fineBtn->setEnabled(a);
    ui->maxLine->setEnabled(a);
    ui->minLine->setEnabled(a);
    ui->setBtn->setEnabled(a);
    ui->initLine->setEnabled(a);
    ui->achievedLine->setEnabled(a);
    ui->acceleratedLine->setEnabled(a);
    ui->setBtn_2->setEnabled(a);
    ui->zSlider->setEnabled(a);

    if(isReset)
    {
        ui->resetBtn->setEnabled(!ui->resetBtn->isEnabled());
    }

    if(isReset2)
    {
        ui->resetBtn_2->setEnabled(!ui->resetBtn_2->isEnabled());
    }
}

// send command function
bool MainWindow::sendCmdPackage(QString cmd)
{
    // command initiate
    int	len;
    memset(&m_Cmd, 0x00, sizeof(MDK_MSL_CMD));

    // save to command history
    QListWidgetItem *item = new QListWidgetItem;
    QString str = " < ";
    str.append(cmd);
    item->setText(str);
    ui->listWidget->addItem(item);
    ui->listWidget->setCurrentRow(ui->listWidget->count() - 1);

    // add a new line to command string
    cmd.append("\r\n");
    len = cmd.length();

    // QString to BYTE( unsigned char* )
    QByteArray tmp = cmd.toLatin1();

    // write to memory
    memcpy(m_Cmd.m_Cmd, tmp.data(), len);

    // set parameters for command
    // copy from sample source code
    // maybe need a little revise
    m_Cmd.m_CmdSize		= 0;
    m_Cmd.m_Callback	= (void *)CommandCallback;
    m_Cmd.m_Context		= NULL;		// this pointer passed by pv
    m_Cmd.m_Timeout		= 10000;	// (ms)
    m_Cmd.m_Sync		= FALSE;
    m_Cmd.m_Command		= TRUE;		// TRUE: Command , FALSE: it means QUERY form ('?').

    // send command
    bool ret = sendCmd(pInterface, &m_Cmd);

    return ret;
}

// menubar action function
void MainWindow::on_actionSelection_triggered()
{
    IFSelection *IFdialog = new IFSelection(nullptr, this->pInterface, this->init, this->enumIf, this->getInfo, this->openIf, this->sendCmd, this->reCb, this->closeIf);
    connect(IFdialog, SIGNAL(sendPointer(void*)), this, SLOT(receivePointer(void*)));
    IFdialog->exec();
}

// BUG 2
// 想要实现接收退出成功信号再closeIf，而不是用Sleep强行等待
// loginBtn 同理
void MainWindow::on_closeBtn_clicked()
{
    sendCmdPackage("L 0,0");

    isReset = ui->resetBtn->isEnabled();
    isReset2 = ui->resetBtn_2->isEnabled();
    waitSettings(false);

    // wait until command finished
    waitThread->start();
    emit sendMode(1);
}

// 更好的效果是检测到登录成功，打开右侧面板
// 能否写一个并行线程检测到 < L 1,1后返回 > L +，打开右侧面板，关闭Login Button，然后销毁自身
// 也就是同时允许用户输入命令来登录，而不是一定使用Login Button
void MainWindow::on_loginBtn_clicked()
{
    sendCmdPackage("L 1,1");

    isReset = ui->resetBtn->isEnabled();
    isReset2 = ui->resetBtn_2->isEnabled();
    waitSettings(false);

    // wait until command finished
    waitThread->start();
    emit sendMode(2);

}

void MainWindow::receiveModeAndRun(int mode)
{
    switch (mode)
    {
    default:
        return;
    case 1:
        this->closeIf(this->pInterface);

        // in case clicking the Button 'Close Interface'
        // reset all the widgets
        ocWidget->setText("No interface is opened. Please open one from menu.");

        defaultSettings(false, true);
        return;
    case 2:
        sendCmdPackage("OPE 0");

        // wait until command finished
        waitThread->start();
        emit sendMode(3);
        return;
    case 3:
        defaultSettings(true, false);
        ui->loginBtn->setEnabled(false);
        return;

    }
}

void MainWindow::on_sendBtn_clicked()
{
    QString cmd = ui->cmdLine->text();
    sendCmdPackage(cmd);
    ui->cmdLine->clear();
}

// Fine Button is a checkable button
void MainWindow::on_fineBtn_clicked()
{
    // The complexity is mainly because the slider's step size only supports int
    double initVal = ui->doubleSpinBox->value();
    double maxVal = ui->maxLine->text().toDouble();
    double minVal = ui->minLine->text().toDouble();
    int maxValInt;
    int minValInt;
    int interval;
    int sliderVal = ui->zSlider->value();

    if (ui->fineBtn->isChecked())
    {
        // form Unchecked to Checked
        ui->stepLine->setText("0.01");
        ui->doubleSpinBox->setSingleStep(0.01);

        maxValInt = maxVal * 100;
        minValInt = minVal * 100;
        interval = 510;
        sliderVal = sliderVal * 10;

        ui->fineBtn->setChecked(true);
    }
    else
    {
        // form Checked to Unchecked
        ui->stepLine->setText("0.1");
        ui->doubleSpinBox->setSingleStep(0.1);

        maxValInt = maxVal * 10;
        minValInt = minVal * 10;
        interval = 51;
        sliderVal = sliderVal / 10;

        ui->fineBtn->setChecked(false);
    }

    ui->zSlider->setMaximum(maxValInt);
    ui->zSlider->setMinimum(minValInt);
    ui->zSlider->setValue(sliderVal);
    ui->zSlider->setTickInterval((maxValInt - minValInt) / interval);

    // in case setting minimum above current focus
    double cVal = ui->doubleSpinBox->value();
    if (initVal != cVal)
    {
        QString command = "FG ";
        command.append(QString::number(maxVal*100, 10, 0));
        sendCmdPackage(command);
    }
}

// Active Notification Button is a checkable button
void MainWindow::on_NFPBtn_clicked()
{
    if (ui->NFPBtn->isChecked())
    {
        // from Unchecked to Checked
        sendCmdPackage("NFP 1");
        ui->NFPBtn->setChecked(true);
        ui->label_6->setVisible(true);
        ui->cvLabel->setVisible(true);
        ui->syncBtn->setVisible(true);
        ui->syncBtn->setEnabled(true);
    }
    else
    {
        // from Checked to Unchecked
        sendCmdPackage("NFP 0");
        ui->fineBtn->setChecked(false);
        ui->label_6->setVisible(false);
        ui->cvLabel->setVisible(false);
        ui->syncBtn->setVisible(false);
        ui->syncBtn->setEnabled(false);
    }
}

// this function will simutaneously set Focus Near Limit
void MainWindow::on_setBtn_clicked()
{
    ui->statusbar->showMessage("Slider Settings are processing, please wait...", 3000);
    // if Escape Button is checked, uncheck it
    if (ui->escapeBtn->isChecked())
        ui->escapeBtn->setChecked(false);

    double initVal;
    if (ui->NFPBtn->isChecked())
        initVal = ui->cvLabel->text().toDouble();
    else
        initVal = ui->doubleSpinBox->value();

    double maxVal = ui->maxLine->text().toDouble();
    double minVal = ui->minLine->text().toDouble();

    if (minVal < 0)
    {
        QMessageBox::warning(NULL, "Below the lower bound", "Minimum is too small!");
        ui->minLine->setText("0");
        return;
    }
    else if (maxVal > 10500)
    {
        QMessageBox::warning(NULL, "Above the upper bound", "Maximum is too big!");
        ui->maxLine->setText("10500");
        return;
    }

    QString command = "NL ";
    QString val = QString::number(maxVal*100, 10, 0);
    // 这里需要6个参数是因为支持6个物镜独立设置Near Focus Limit的原因
    // 后续加入物镜控制的话，需要修改这里
    command.append(val);
    QString comma = QString(",");
    comma.append(val);
    for(int i=0; i<5; i++)
    {
        command.append(comma);
    }
    sendCmdPackage(command);

    ui->doubleSpinBox->setMaximum(maxVal);
    ui->doubleSpinBox->setMinimum(minVal);

    int maxValInt;
    int minValInt;
    int interval;
    if (ui->fineBtn->isChecked())
    {
        maxValInt = maxVal * 100;
        minValInt = minVal * 100;
        interval = 510;
    }
    else
    {
        maxValInt = maxVal * 10;
        minValInt = minVal * 10;
        interval = 51;
    }

    ui->zSlider->setMaximum(maxValInt);
    ui->zSlider->setMinimum(minValInt);
    ui->zSlider->setTickInterval((maxValInt - minValInt) / interval);

    // in case setting minimum is above current focus
    // or setting maximum is below current focus
    double cVal = ui->doubleSpinBox->value();
    if (initVal != cVal)
    {
        QString newCommand = "FG ";
        if (cVal > initVal)
        {
            newCommand.append(QString::number(minVal*100, 10, 0));
        }
        else
        {
            newCommand.append(QString::number(maxVal*100, 10, 0));
        }
        Sleep(3000);
        sendCmdPackage(newCommand);
    }

    // enable Reset Button
    if (!ui->resetBtn->isEnabled())
        ui->resetBtn->setEnabled(true);

    ui->statusbar->showMessage("Slider Settings complete.", 3000);
}

void MainWindow::on_resetBtn_clicked()
{
    if (ui->escapeBtn->isChecked())
        ui->escapeBtn->setChecked(false);
    ui->maxLine->setText("10500");
    ui->minLine->setText("0");
    this->on_setBtn_clicked();
    ui->resetBtn->setEnabled(false);
}

// Escape Button is a checkable button
void MainWindow::on_escapeBtn_clicked()
{
    if (ui->escapeBtn->isChecked())
    {
        // from Unchecked to Checked
        escapeVal = ui->cvLabel->text().toDouble() * 100;

        QString command = "FG ";
        command.append(QString::number(ui->doubleSpinBox->minimum()*100,10,0));
        sendCmdPackage(command);

        if (ui->fineBtn->isChecked())
            ui->zSlider->setValue((int)ui->doubleSpinBox->minimum()*10);
        else
            ui->zSlider->setValue((int)ui->doubleSpinBox->minimum()*100);

        ui->doubleSpinBox->setValue(ui->doubleSpinBox->minimum());

        ui->escapeBtn->setChecked(true);
    }
    else
    {
        // from Checked to Unchecked
        QString command = "FG ";
        command.append(QString::number(escapeVal));
        sendCmdPackage(command);

        double spinVal = (double)escapeVal / 100;
        ui->doubleSpinBox->setValue(spinVal);

        if (ui->fineBtn->isChecked())
            ui->zSlider->setValue(escapeVal / 10);
        else
            ui->zSlider->setValue(escapeVal);

        ui->escapeBtn->setChecked(false);
    }
}

void MainWindow::on_setBtn_2_clicked()
{
    int initSpeed = ui->initLine->text().toInt();
    int achiSpeed = ui->achievedLine->text().toInt();
    int acceSpeed = ui->acceleratedLine->text().toInt();
    int calRet = (achiSpeed - initSpeed) / acceSpeed;
    if (initSpeed < 1 ||
            initSpeed > 70000 ||
            achiSpeed < 1 ||
            achiSpeed > 300000 ||
            acceSpeed < 1 ||
            acceSpeed > 1000 ||
            calRet > 38)
        QMessageBox::warning(NULL, "Warning",
                             "This combination does not satisfy the conditions, "
                             "please check the Application Note!");
    else
    {
        QString command = "FSPD ";
        command.append(QString::number(initSpeed * 100));
        command.append(",");
        command.append(QString::number(achiSpeed * 100));
        command.append(",");
        command.append(QString::number(acceSpeed));
        sendCmdPackage(command);
    }
    ui->resetBtn_2->setEnabled(true);
}

void MainWindow::on_resetBtn_2_clicked()
{
    ui->initLine->setText("700");
    ui->achievedLine->setText("3000");
    ui->acceleratedLine->setText("60");
    sendCmdPackage("FSPD 70000,300000,60");
    ui->resetBtn_2->setEnabled(false);
}

void MainWindow::on_zSlider_sliderReleased()
{
    if (ui->escapeBtn->isChecked())
        ui->escapeBtn->setChecked(false);
    int val = ui->zSlider->value();
    if (!ui->fineBtn->isChecked())
        val = val * 10;
    QString command = "FG ";
    command.append(QString::number(val));
    sendCmdPackage(command);
    double dVal = (double)val / 100;
    ui->doubleSpinBox->setValue(dVal);
}

void MainWindow::on_doubleSpinBox_valueChanged()
{
    if (ui->escapeBtn->isChecked())
        ui->escapeBtn->setChecked(false);
    double targetVal = ui->doubleSpinBox->value();
    QString command = "FG ";
    command.append(QString::number(targetVal * 100, 10, 0));
    sendCmdPackage(command);

    // set slider value
    if (ui->fineBtn->isChecked())
        ui->zSlider->setValue((int)(targetVal*100));
    else
        ui->zSlider->setValue((int)(targetVal*10));
}

void MainWindow::on_syncBtn_clicked()
{
    double cvVal = ui->cvLabel->text().toDouble();
    // set double spinbox
    ui->doubleSpinBox->setValue(cvVal);

    // set slider value
    if (ui->fineBtn->isChecked())
        ui->zSlider->setValue((int)(cvVal*100));
    else
        ui->zSlider->setValue((int)(cvVal*10));
}

void MainWindow::on_actionTest_triggered()
{
    emit sendSuccessed();
}
