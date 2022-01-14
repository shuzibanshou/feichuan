#include "udptrans.h"
#include "ui_udptrans.h"


UDPTrans::UDPTrans(QWidget *parent) :QMainWindow(parent),ui(new Ui::UDPTrans)
{
    ui->setupUi(this);
    //qsrand(QTime::currentTime().msec());
    localIPv4 = getHostIP();
    checkEnv();
    //启动UDP协议
    udpSocket = new QUdpSocket(this);
    broadcastTimer = new QTimer(this);
    scanDevicesTimer = new QTimer(this);
//    for(quint16 port = initPort;; port++){
//        if(udpSocket->bind(port)){
//            actualPort = port;
//            break;
//        }
//    }
    if(!udpSocket->bind(initPort)){
       qDebug("绑定失败,%s",qPrintable(udpSocket->errorString()));
       udpSocket->bind(initPort);
    } else {
       qDebug("绑定成功");
    }
    connect(udpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(onSocketReadyRead()));
    connect(broadcastTimer,SIGNAL(timeout()),this,SLOT(lanBroadcast()));
    connect(scanDevicesTimer,SIGNAL(timeout()),this,SLOT(scanDevices()));

    lanBroadcast();
    broadcastTimer->start(broadcastInterval);
    scanDevicesTimer->start(scanDevicesInterval);
}

UDPTrans::~UDPTrans()
{
    delete ui;
}

QString UDPTrans::protocolName(QAbstractSocket::NetworkLayerProtocol protocol)
{
    switch (protocol) {
        case QAbstractSocket::IPv4Protocol:
            return "IPv4";
        case QAbstractSocket::IPv6Protocol:
            return "IPv6";
        case QAbstractSocket::AnyIPProtocol:
            return "AnyIP";
        default:
            return "UnknownNetwork";
    }
}

/**
 * 获取本地IPV4地址(废弃 有可能获取到虚拟机路由ip)
 * @brief UDPTrans::getLocalIPv4
 * @return
 */
QString UDPTrans::getLocalIPv4()
{
    QString localIP = "";   //本地IPv4地址
    QString hostName = QHostInfo::localHostName();
    QHostInfo hostInfo = QHostInfo::fromName(hostName);

    QList<QHostAddress> addrList = hostInfo.addresses();
    for(int i = 0; i < addrList.size(); i++){
        if(addrList[i].protocol() == QAbstractSocket::IPv4Protocol){
            localIP = addrList[i].toString();
            qDebug() << localIP;
            //return localIP;
        }
    }
    return localIP;
}


/**
 * 读取所有本地地址信息（物理网卡和虚拟机）
 * @brief UDPTrans::getHostIP
 * @return
 */
QMap<QString,QString> UDPTrans::getHostIP()
{
    QMap<QString,QString> map;
    QHostAddress strIpAddress;
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface interfaceItem, interfaceList)
    {
        //qDebug() << interfaceItem.humanReadableName();
        if(interfaceItem.flags().testFlag(QNetworkInterface::IsUp)
                &&interfaceItem.flags().testFlag(QNetworkInterface::IsRunning)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanBroadcast)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanMulticast)
                &&!interfaceItem.flags().testFlag(QNetworkInterface::IsLoopBack)
                /*&&interfaceItem.hardwareAddress()!="00:50:56:C0:00:01"          //虚拟机保留Mac地址
                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:08"
                &&
                (interfaceItem.humanReadableName()=="WLAN"
                ||
                interfaceItem.humanReadableName()=="以太网")*/)
        {
            QList<QNetworkAddressEntry> addressEntryList=interfaceItem.addressEntries();
            foreach(QNetworkAddressEntry addressEntryItem, addressEntryList)
            {
                if(addressEntryItem.ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
//                    qDebug()<<"------------------------------------------------------------";
//                    qDebug()<<"Adapter Name:"<<interfaceItem.name();
//                    qDebug()<<"Adapter Name:"<<interfaceItem.index();
//                    qDebug()<<"Adapter Name:"<<interfaceItem.humanReadableName();
//                    qDebug()<<"Adapter Address:"<<interfaceItem.hardwareAddress();
//                    qDebug()<<"IP Address:"<<addressEntryItem.ip().toString();
//                    qDebug()<<"IP Mask:"<<addressEntryItem.netmask().toString();

                    strIpAddress = addressEntryItem.ip();
                    //qDebug() << strIpAddress;
                    map.insert(strIpAddress.toString(),interfaceItem.hardwareAddress());
                }
            }
        }
    }

    return map;
}

/**
 * 检查环境
 */
void UDPTrans::checkEnv()
{
    QMap<QString, QString>::iterator iter = localIPv4.begin();
    while (iter != localIPv4.end())
    {
        if(iter.value() == "00:50:56:C0:00:08" || iter.value() == "00:50:56:C0:00:01"){
            QMessageBox::warning(this, tr("提示"),tr("检查到您的主机安装有虚拟网卡.\n请确保您的虚拟机与宿主机为桥接模式,否则可能无法通信"),QMessageBox::Ok,QMessageBox::Ok);
            return;
        }
        iter++;
    }
}

/**
 * 检查广播迂回地址
 * 如果地址包含在本地所有地址中 则需要检查是否为虚拟网卡地址 如果是虚拟地址则本机可能在一个孤立的子网中 需要用户重启虚拟网卡 否则可能无法通信
 * 如果地址未包含在本地所有地址中 则说明是局域网内其他设备的广播响应
 * @brief UDPTrans::checkBroadcast
 * @param remoteIPv4Addr
 */
void UDPTrans::checkBroadcast(QString remoteIPv4Addr)
{
//    qDebug() << remoteIPv4Addr;
//    qDebug() << data;
//    qDebug() << QString::number(oldBroadcastIndex).toUtf8();

    quint32 timestamp = QDateTime::currentDateTime().toTime_t();   //获取当前时间
    if(localIPv4.contains(remoteIPv4Addr)){
        if(localIPv4.find(remoteIPv4Addr).value() == "00:50:56:C0:00:01" || localIPv4.find(remoteIPv4Addr).value() == "00:50:56:C0:00:08"){
            QMessageBox::critical(this, tr("错误"),tr("广播迂回地址为虚拟网卡地址,初始化失败,请重启虚拟网卡尝试解决,详情请见教程"),QMessageBox::Ok,QMessageBox::Ok);
            return;
        }
    } else {
        //将局域网其他主机写入列表 存在则更新 不存在则添加
        lanDevices.insert(remoteIPv4Addr,timestamp);
    }
}


/**
 * TODO 广播携带主机名和设备信息
 * 程序初始化后进行局域网UDP广播
 * @brief UDPTrans::lanBroadcast
 */
void UDPTrans::lanBroadcast()
{
    udpSocket->writeDatagram("",QHostAddress::Broadcast,initPort);
}

/**
 * 遍历局域网活跃设备列表 比对时间戳
 * 将超时设备踢出设备列表
 * @brief UDPTrans::scanDevices
 */
void UDPTrans::scanDevices()
{
    ui->textEdit->clear();
    quint32 now = QDateTime::currentDateTime().toTime_t();
    QMap<QString, quint64>::iterator iter = lanDevices.begin();
    while (iter != lanDevices.end())
    {
//        qDebug() << now << "+" << iter.key() << "+" << iter.value();
//        if(now - iter.value() >= unactiveTimeout){
//            lanDevices.remove(iter.key());        //这种写法Linux下删除节点会有问题
//        } else {
//            ui->textEdit->append("远程主机IPv4:"+iter.key());
//        }
//        iter++;

        if(now - iter.value() >= unactiveTimeout){
            lanDevices.erase(iter++);
        } else {
            ui->textEdit->append("远程主机IPv4:"+iter.key());
            iter++;
        }
    }
}

/**
 * @brief UDPTrans::onSocketStateChanged
 * @param socketState
 */
void UDPTrans::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            qDebug() << "UnconnectedState";
            break;
        case QAbstractSocket::HostLookupState:
            qDebug() << "HostLookupState";
            break;
        case QAbstractSocket::ConnectingState:
            qDebug() << "ConnectingState";
            break;
        case QAbstractSocket::ConnectedState:
            qDebug() << "ConnectedState";
            break;
        case QAbstractSocket::BoundState:
            qDebug() << "BoundState";
            break;
        case QAbstractSocket::ListeningState:
            qDebug() << "ListeningState";
            break;
        case QAbstractSocket::ClosingState:
            qDebug() << "ClosingState";
            break;
    }
}

/**
 * @brief UDPTrans::onSocketReadyRead
 * 监听网络的UDP广播 忽略本机IP地址
 */
void UDPTrans::onSocketReadyRead()
{
    //qDebug() << udpSocket->state();
    while(udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
        QHostAddress remoteIPv6Addr;        //远程主机地址ipv6
        quint16 remotePort;             //远程主机UDP端口
        udpSocket->readDatagram(datagram.data(),datagram.size(),&remoteIPv6Addr,&remotePort);
        QString remoteIPv4Addr = QHostAddress(remoteIPv6Addr.toIPv4Address()).toString();
        checkBroadcast(remoteIPv4Addr);
        /*if(remoteIPv6Addr.isEqual(localIPv4,QHostAddress::ConversionModeFlag::ConvertV4MappedToIPv4)){
            ui->textEdit->append("忽略本机广播");
        }*/
    }
}


