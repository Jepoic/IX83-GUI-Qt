#ifndef IFSELECTION_H
#define IFSELECTION_H

#include <QDialog>
#include <mainwindow.h>
#include <DLL.h>

namespace Ui {
class IFSelection;
}

class IFSelection : public QDialog
{
    Q_OBJECT

public:
    explicit IFSelection(QWidget *parent = nullptr,
                         void* pIF = nullptr,
                         ptr_Initialize init = nullptr,
                         ptr_EnumInterface enumIf = nullptr,
                         ptr_GetInterfaceInfo getInfo = nullptr,
                         ptr_OpenInterface openIf = nullptr,
                         ptr_SendCommand sendCmd = nullptr,
                         ptr_RegisterCallback reCb = nullptr,
                         ptr_CloseInterface closeIf = nullptr);
    ~IFSelection();

    void* pIF;
    ptr_Initialize init;
    ptr_EnumInterface enumIf;
    ptr_GetInterfaceInfo getInfo;
    ptr_OpenInterface openIf;
    ptr_SendCommand sendCmd;
    ptr_RegisterCallback reCb;
    ptr_CloseInterface closeIf;

signals:
    void sendPointer(void *);
    void sendQuitSymbol(bool);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();


private:
    Ui::IFSelection *ui;
};

#endif // IFSELECTION_H
