#include "udptrans.h"
#include "receivefile.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    UDPTrans w;
    w.show();

    return a.exec();
}



