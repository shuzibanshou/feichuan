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
    void checkBroadcast(QString,QString);                                   //检查广播迂回地址
    QStringList oldLanDevices;                                      //局域网内设备IPv4地址集合-上次在线检查
    QStringList newLanDevices;                                      //局域网内设备IPv4地址集合-最新在线检查
    quint16 broadcastInterval = 2000;                               //局域网UDP循环广播时间间隔 默认2000毫秒
    QTimer*  broadcastTimer;                                        //局域网UDP循环广播定时器
    quint32 oldBroadcastIndex = 0;                                      //上次广播索引，在广播的时候作为数据发送出去，响应的时候再带回来
    quint32 newBroadcastIndex = 0;                                      //最新广播索引，在广播的时候作为数据发送出去，响应的时候再带回来

private slots:
    void onSocketStateChanged(QAbstractSocket::SocketState);
    void onSocketReadyRead();
    void lanBroadcast();                                            //程序启动时进行局域网广播
};

#endif // UDPTRANS_H
