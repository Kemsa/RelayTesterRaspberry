#include "ADC24.h"
#include "ADCBase.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    ADC24 adc;

    MainWindow w;
    w.show();

    // return a.exec();
    return QApplication::exec();
}
