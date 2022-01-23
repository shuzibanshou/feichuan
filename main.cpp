#include "udptrans.h"
#include "receivefile.h"
#include <QApplication>



int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    UDPTrans w;
    w.show();
    //QByteArray x = intToByte(1);
    //qDebug() << bytesToInt(x);

    return a.exec();
}



