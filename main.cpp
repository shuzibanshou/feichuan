#include "udptrans.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);


    QMap<QString,quint16> map1;
    QMap<QString,quint16> map2;
    map1.insert("a",1);
    map1.insert("b",2);
    map1.insert("c",3);
    map2 = map1;
    QMap<QString, quint16>::iterator map1_one = map1.begin();
    QMap<QString, quint16>::iterator temp = map1_one;
    QMap<QString, quint16>::iterator map1_two = ++temp;
    QMap<QString, quint16>::iterator temp2 = map1_two;
    QMap<QString, quint16>::iterator map1_three = ++temp2;
    QMap<QString, quint16>::iterator map1_end = map1.end();
    //iter++;

    QMap<QString, quint16>::iterator map2_one = map2.begin();
    QMap<QString, quint16>::iterator temp3 = map2_one;
    QMap<QString, quint16>::iterator map2_two = ++temp3;
    QMap<QString, quint16>::iterator temp5 = map2_two;
    QMap<QString, quint16>::iterator map2_three = ++temp5;
    QMap<QString, quint16>::iterator map2_end = map2.end();


//    if(iter == map.end()){
//       qDebug() << "end";
//    } else {
//        qDebug() << "not end";
//    }


//    while (iter != map.end())
//    {
//        qDebug() << iter.value();
//        iter++;
//        //map.insert("d",4);
//        map2 = map;
//    }

    UDPTrans w;
    w.show();

    return a.exec();
}
