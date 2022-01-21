#include "udptrans.h"
#include "receivefile.h"
#include <QApplication>


int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    UDPTrans w;
    w.show();

    //打开接收文件句柄
//    QFile testFileWrite;
//    testFileWrite.setFileName("D:/Qt/project/build-UDPTrans-Desktop_Qt_5_13_2_MinGW_64_bit-Debug/debug/receiveFiles/1.txt");
//    testFileWrite.open(QIODevice::WriteOnly);
//    QByteArray arr = QByteArray("成功");
//    qDebug() << arr;
//    testFileWrite.write(arr);
//    testFileWrite.close();
//    QFile testFileRead;
//    testFileRead.setFileName("C:/Users/Administrator/Desktop/ThinkCMF/ThinkCMF/public/sureGiveBackCallBack100.txt");
//    testFileRead.open(QIODevice::ReadOnly);
//    //char buff[2048] = {0};      //4096
//    QByteArray buff = testFileRead.read(2048);
//    qDebug() << buff.length();

    return a.exec();
}
