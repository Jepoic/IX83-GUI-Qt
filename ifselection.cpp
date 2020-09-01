#include "ifselection.h"
#include "ui_ifselection.h"
#include "mainwindow.h"
#include <DLL.h>
#include <QMessageBox>

IFSelection::IFSelection(QWidget *parent, void* pIF,
                         ptr_Initialize init,
                         ptr_EnumInterface enumIf,
                         ptr_GetInterfaceInfo getInfo,
                         ptr_OpenInterface openIf,
                         ptr_SendCommand sendCmd,
                         ptr_RegisterCallback reCb,
                         ptr_CloseInterface closeIf) :
    QDialog(parent),
    ui(new Ui::IFSelection)
{
    ui->setupUi(this);

    // pass parameters
    this->pIF = pIF;
    this->init = init;
    this->enumIf = enumIf;
    this->getInfo = getInfo;
    this->openIf = openIf;
    this->sendCmd = sendCmd;
    this->reCb = reCb;
    this->closeIf = closeIf;

    // enumerate interfaces
    int static count = this->enumIf();
    //count = 3;    // cheat code in case you don't have an interface
    while (!count)
    {
        QMessageBox:: StandardButton result = QMessageBox::critical(NULL,"Failed to enumerate interface",
                                                                    "There is no Olympus IX83 connected, "
                                                                    "please check physical connections and 1394 drivers.",
                                                                    QMessageBox::Retry|QMessageBox::Cancel);
        switch (result)
        {
        case QMessageBox::Retry:
            break;
        default:
            qApp->exit();
            return;
        }
    }

    // set widgets
    QString labelStr = ui->label->text();
    labelStr.append( QString::number(count) );
    ui->label->setText( labelStr );
    ui->comboBox->clear();
    for (int i=0; i<count; i++)
        ui->comboBox->addItem( QString::asprintf("Interface %d",i+1) );
}

IFSelection::~IFSelection()
{
    delete ui;
}

void IFSelection::on_buttonBox_accepted()
{
    // initialize
    void *pInterface = nullptr;

    // get a pointer to the selected interface address
    this->getInfo(ui->comboBox->currentIndex(), &pInterface);

    // check whether the interface is open or not
    bool ret = false;
    while (!ret)
    {
        ret = this->openIf(pInterface);
        //ret = true;   // cheat code
        if (!ret)
        {
            QMessageBox::StandardButton result = QMessageBox::critical(NULL,"Failed to open interface",
                                                                       "Cloud not open port!",
                                                                       QMessageBox::Retry|QMessageBox::Cancel);
            switch (result)
            {
            case QMessageBox::Cancel:
                // BUG 1: the application doesn't quit here, because main.cpp isn't finished yet.
                // fixed 0818
                emit sendQuitSymbol(true);
                return;
            default:
                break;
            }
        }
        if (ret)
        {
            // in case an interface is open before this one
            if (this->pIF != nullptr)
                this->closeIf(this->pIF);
            emit sendPointer(pInterface);
        }
    }
}

void IFSelection::on_buttonBox_rejected()
{
    this->close();
    return;
}
