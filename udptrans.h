#ifndef UDPTRANS_H
#define UDPTRANS_H

#include <QMainWindow>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QDebug>
#include <QHostInfo>
#include <QMap>
#include <QString>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
//#include <QStringList>

namespace Ui {
class UDPTrans;
}

class UDPTrans : public QMainWindow
{
    Q_OBJECT

public:
    explicit UDPTrans(QWidget *parent = nullptr);
    ~UDPTrans();


private:
    Ui::UDPTrans *ui;
    quint16 initPort = 10000;   //初始化UDP绑定端口 若端口占用绑定失败则在此基础上+1再次进行绑定直到绑定成功为止
    quint16 actualPort = 10000; //最后实际上绑定的UDP端口

    QUdpSocket* udpSocket;
    QString protocolName(QAbstractSocket::NetworkLayerProtocol);    //协议族名称转换
    QMap<QString,QString> localIPv4;                                //保存本地IPv4地址的变量
    QString getLocalIPv4();                                         //获取本地IPv4地址
    QMap<QString,QString> getHostIP();

    void checkEnv();  //检查环境
    void checkBroadcast(QString);                                   //检查广播迂回地址

    quint16 broadcastInterval = 5000;                               //局域网UDP循环广播时间间隔 默认5000毫秒
    quint16 scanDevicesInterval = 5000;                             //扫描活跃设备的时间间隔
    quint16 unactiveTimeout = 8;                                    //非活跃设备超时时间 默认 8秒
    QTimer*  broadcastTimer;                                        //局域网UDP循环广播定时器
    QTimer*  scanDevicesTimer;                                      //扫描活跃设备定时器
    QMap<QString,quint64> lanDevices;                               //局域网内设备IPv4地址合集-定时扫描踢出下线设备

private slots:
    void onSocketStateChanged(QAbstractSocket::SocketState);
    void onSocketReadyRead();
    void lanBroadcast();                                            //程序启动时进行局域网广播
    void scanDevices();                                             //扫描活跃设备
};

#endif // UDPTRANS_H
