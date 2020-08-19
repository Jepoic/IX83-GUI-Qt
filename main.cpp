#include <QApplication>
#include <QLibrary>
#include <QMessageBox>
#include "mainwindow.h"
#include "ifselection.h"
#include "DLL.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //// load DLL library ////
    int libLoadResult[4];
    QLibrary pm("msl_pm.dll");
    QLibrary pd("msl_pd_1394.dll");
    QLibrary fsi("fsi1394.dll");
    QLibrary gt("gt_log.dll");
    libLoadResult[0] = pm.load();
    libLoadResult[1] = pd.load();
    libLoadResult[2] = fsi.load();
    libLoadResult[3] = gt.load();
    // determine whether the loading is successful
    int count = 0;
    for(int i = 0; i < 4; i++)
    {
        if (libLoadResult[i] == 0) count++;
    }
    if (count != 0)
    {
        QMessageBox::critical(NULL,"Missing DLLs","Could not load library!\n" \
                                                  "Please check all the DLLs and CONFIG file are in the installation directory\n" \
                                                  "Here is the requiring DLL and CONFIG file list:\n" \
                                                  "msl_pm.dll\n" \
                                                  "msl_pd_1394.dll\n" \
                                                  "fsi1394.dll\n" \
                                                  "gt_log.dll\n"
                                                  "(*)gtlib.config");
        return 0;
    }
    //// end loading DLL library ////

    //// load function pointer ////
    ptr_Initialize init = (ptr_Initialize)pm.resolve("MSL_PM_Initialize");
    if (!init)
    {
        QMessageBox::critical(NULL,"Initialization failed","Please confirm DLLs are intact!");
        return 0;
    }
    init();

    ptr_EnumInterface enumIf = (ptr_EnumInterface)pm.resolve("MSL_PM_EnumInterface");
    if (!enumIf)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }

    ptr_GetInterfaceInfo getInfo = (ptr_GetInterfaceInfo)pm.resolve("MSL_PM_GetInterfaceInfo");
    if (!getInfo)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }

    ptr_OpenInterface openIf = (ptr_OpenInterface)pm.resolve("MSL_PM_OpenInterface");
    if (!openIf)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }

    ptr_SendCommand sendCmd = (ptr_SendCommand)pm.resolve("MSL_PM_SendCommand");
    if (!sendCmd)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }

    ptr_RegisterCallback reCb = (ptr_RegisterCallback)pm.resolve("MSL_PM_RegisterCallback");
    if (!reCb)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }

    ptr_CloseInterface closeIf = (ptr_CloseInterface)pm.resolve("MSL_PM_CloseInterface");
    if (!closeIf)
    {
        QMessageBox::critical(NULL,"Failed to link functions","Please confirm DLLs are intact!");
        return 0;
    }
    //// end loading function pointer ////

    // create IFSelection dialog and mainwindow at the same time
    // then connect their signals and slots
    IFSelection *IFdialog = new IFSelection(nullptr, nullptr, init, enumIf, getInfo, openIf, sendCmd, reCb, closeIf);
    static MainWindow *w = new MainWindow(nullptr, init, enumIf, getInfo, openIf, sendCmd, reCb, closeIf);
    QObject::connect(IFdialog, SIGNAL(sendPointer(void*)), w, SLOT(receivePointer(void*)));
    QObject::connect(IFdialog, SIGNAL(sendQuitSymbol(bool)), w, SLOT(receiveQuitSymbol(bool)));

    if( IFdialog->exec() == QDialog::Rejected )
    {
        a.exit();
        return 0;
    }

    // BUG 1: a known bug is occured here, details are in ifselection.cpp
    // fixed 0818
    w->show();
    if (w->quitSymbol)
        return 0;
    return a.exec();
}
