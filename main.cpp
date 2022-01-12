#include "udptrans.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    UDPTrans w;
    w.show();
     //QMap<QString,QString> m;
     //m.insert("1","123");

    return a.exec();
}
