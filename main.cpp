#include "QtWidgetsApplication1.h"
#include <QtWidgets/QApplication>

#include <iostream>
#include "kinect/k4a.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtWidgetsApplication1 w;
    
    w.show();


    return a.exec();
}
