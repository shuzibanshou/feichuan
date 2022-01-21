#include "udptrans.h"
#include "ui_udptrans.h"

#include "receivefile.h"
#include "progress.h"

UDPTrans::UDPTrans(QWidget *parent) :QMainWindow(parent),ui(new Ui::UDPTrans)
{
    ui->setupUi(this);
    //qsrand(QTime::currentTime().msec());
    localIPv4 = getHostIP();
    localDevice = getDeviceInfo();
    checkEnv();
    //启动UDP协议
    udpSocket = new QUdpSocket(this);
    udpSocketFile = new QUdpSocket(this);
    broadcastTimer = new QTimer(this);
    scanDevicesTimer = new QTimer(this);
//    for(quint16 port = initPort;; port++){
//        if(udpSocket->bind(port)){
//            actualPort = port;
//            break;
//        }
//    }
    if(!udpSocket->bind(initPort)){
       qDebug("绑定UDP广播端口失败,%s",qPrintable(udpSocket->errorString()));
       udpSocket->bind(initPort);
    } else {
       qDebug("绑定UDP广播端口成功");
    }
    if(!udpSocketFile->bind(filePort)){
       qDebug("绑定UDP文件端口失败,%s",qPrintable(udpSocketFile->errorString()));
       udpSocketFile->bind(filePort);
    } else {
       qDebug("绑定UDP文件端口成功");
    }
    connect(udpSocket,SIGNAL(stateChanged(QAbstractSocket::SocketState)),this,SLOT(onSocketStateChanged(QAbstractSocket::SocketState)));
    connect(udpSocket,SIGNAL(readyRead()),this,SLOT(onSocketReadyRead()));
    connect(broadcastTimer,SIGNAL(timeout()),this,SLOT(lanBroadcast()));
    connect(scanDevicesTimer,SIGNAL(timeout()),this,SLOT(scanDevices()));

    //收发文件信号槽
    connect(udpSocketFile,SIGNAL(readyRead()),this,SLOT(onSocketFileReadyRead()));

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
//QString UDPTrans::getLocalIPv4()
//{
//    QString localIP = "";   //本地IPv4地址
//    QString hostName = QHostInfo::localHostName();
//    QHostInfo hostInfo = QHostInfo::fromName(hostName);

//    QList<QHostAddress> addrList = hostInfo.addresses();
//    for(int i = 0; i < addrList.size(); i++){
//        if(addrList[i].protocol() == QAbstractSocket::IPv4Protocol){
//            localIP = addrList[i].toString();
//            qDebug() << localIP;
//            //return localIP;
//        }
//    }
//    return localIP;
//}




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
 * 封装所有本地IPv4地址和设备信息
 * 主机名获取
 * @brief UDPTrans::getDeviceInfo
 * @return
 */
QString UDPTrans::getDeviceInfo()
{
    QString localHost = QHostInfo::localHostName();
    QString os = QSysInfo::prettyProductName();
    //deviceInfo dev = {.deviceOS = os,.deviceName = localHost};
    return os+"##"+localHost;
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
void UDPTrans::checkBroadcast(QString data,QString remoteIPv4Addr)
{
    //qDebug() << remoteIPv4Addr;
    //qDebug() << data;

    if(!data.isEmpty()){
        QStringList list = data.split("##");
        QString deviceOS = list[0];
        QString deviceName = list[1];
        quint32 timestamp = QDateTime::currentDateTimeUtc().toTime_t();   //获取当前时间

        deviceItem di = {.deviceOS = deviceOS, .deviceName = deviceName, .deviceIPv4 = remoteIPv4Addr, .item = NULL, .timestamp = timestamp};
        if(localIPv4.contains(remoteIPv4Addr)){
            if(localIPv4.find(remoteIPv4Addr).value() == "00:50:56:C0:00:01" || localIPv4.find(remoteIPv4Addr).value() == "00:50:56:C0:00:08"){
                QMessageBox::critical(this, tr("错误"),tr("广播迂回地址为虚拟网卡地址,初始化失败,请重启虚拟网卡尝试解决,详情请见教程"),QMessageBox::Ok,QMessageBox::Ok);
                return;
            }
            //根据广播迂回数据填充本机信息
            ui->localName->setText(deviceName);
            ui->localIPv4->setText(remoteIPv4Addr);
        } else {
            //将局域网其他主机写入最新列表 存在则更新 不存在则添加
            newLanDevices.insert(remoteIPv4Addr,di);
        }
    }
}


/**
 * 广播携带主机名和设备信息
 * 程序初始化后进行局域网UDP广播
 * @brief UDPTrans::lanBroadcast
 */
void UDPTrans::lanBroadcast()
{
    udpSocket->writeDatagram(localDevice.toUtf8(),QHostAddress::Broadcast,initPort);
}

/**
 * 遍历局域网活跃设备列表 比对时间戳
 * 将超时设备踢出设备列表
 * @brief UDPTrans::scanDevices
 */
void UDPTrans::scanDevices()
{
    //ui->remoteDevice->clear();
    quint32 now = QDateTime::currentDateTimeUtc().toTime_t();
    QMap<QString, deviceItem>::iterator iter = newLanDevices.begin();
    //qDebug() << newLanDevices.count();

    while (iter != newLanDevices.end())
    {
        //第0种写法  这种写法Linux下删除节点会有问题
//        if(now - iter.value() >= unactiveTimeout){
//            lanDevices.remove(iter.key());
//        } else {
//            ui->textEdit->append("远程主机IPv4:"+iter.key());
//        }
//        iter++;

        //第一种写法
        if(now - iter.value().timestamp >= unactiveTimeout){
            newLanDevices.erase(iter++);
            delWidgetItem(iter.key());
        } else {
            if(lanDevices.contains(iter.value().deviceIPv4)){
                QString ipv4 = iter.value().deviceIPv4;
                if((iter.value().deviceOS != lanDevices[ipv4].deviceOS) || (iter.value().deviceName != lanDevices[ipv4].deviceName)){
                    updateWidgetItem(iter.key(),iter.value());
                }
            } else {
                addWidgetItem(iter.key(),iter.value());
            }
            iter++;
        }

    }
    lanDevices = newLanDevices;

    //第二种写法
//    QMapIterator<QString, deviceItem> iter(lanDevices);
//    if(iter.hasNext()){
//        iter.next();
//        if(now - iter.value().timestamp >= unactiveTimeout){
//            lanDevices.remove(iter.key());
//        } else {
//            addWidgetItem(iter.value());
//        }
//    }
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
        checkBroadcast(datagram.data(),remoteIPv4Addr);
        /*if(remoteIPv6Addr.isEqual(localIPv4,QHostAddress::ConversionModeFlag::ConvertV4MappedToIPv4)){
            ui->textEdit->append("忽略本机广播");
        }*/
    }
}

/**
 * @brief UDPTrans::addWidgetItem
 * @param dev
 */
void UDPTrans:: addWidgetItem(QString key,deviceItem di){
    QListWidgetItem* remoteItem = new  QListWidgetItem(ui->remoteDevice);
    remoteItem->setSizeHint(QSize(10,100));
    //保存该item
    newLanDevices[key].item = remoteItem;

    //设置item布局
    QWidget *itemWidget = new QWidget;

    QLabel *deviceName = new QLabel(itemWidget);
    deviceName->setObjectName(QStringLiteral("deviceName"));
    QFont font1;
    font1.setFamily(QStringLiteral("Arial"));
    font1.setPointSize(12);
    deviceName->setFont(font1);
    //deviceName->setAlignment(Qt::AlignCenter);
    deviceName->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    //deviceName->setMargin(10);
    deviceName->setText(di.deviceName);

    QLabel *deviceIPv4 = new QLabel(itemWidget);
    deviceName->setObjectName(QStringLiteral("deviceIPv4"));
    QFont font2;
    font2.setFamily(QStringLiteral("Arial"));
    font2.setPointSize(10);
    deviceIPv4->setFont(font2);
    deviceName->setAlignment(Qt::AlignCenter);
    deviceIPv4->setText(di.deviceIPv4);

    QPushButton *sendFile = new QPushButton(itemWidget);
    sendFile->setObjectName(QStringLiteral("sendFile"));
    sendFile->setProperty("ip",di.deviceIPv4);
    sendFile->setText(QApplication::translate("UDPTrans", "\345\217\221\351\200\201\346\226\207\344\273\266", Q_NULLPTR));

    QPushButton *sendMsg = new QPushButton(itemWidget);
    sendMsg->setObjectName(QStringLiteral("sendMsg"));
    sendMsg->setProperty("ip",di.deviceIPv4);
    sendMsg->setText(QApplication::translate("UDPTrans", "\345\217\221\351\200\201\346\266\210\346\201\257", Q_NULLPTR));

    QHBoxLayout *layout = new QHBoxLayout(itemWidget);
    layout->addWidget(deviceName);
    layout->addWidget(deviceIPv4);
    layout->addWidget(sendFile);
    layout->addWidget(sendMsg);
    itemWidget->setLayout(layout);
    ui->remoteDevice->setItemWidget(remoteItem,itemWidget);

    //sendMsgBtnList.append(sendMsg);
    //是否会内存泄漏
    connect(sendFile,SIGNAL(clicked()),this,SLOT(openFile()));
    connect(sendMsg,SIGNAL(clicked()),this,SLOT(openMsgDialog()));
    //itemWidget->installEventFilter(this);
}


//void UDPTrans::itemClicked(QListWidgetItem * item){
//    qDebug() << item;
//}

//void UDPTrans::itemDoubleClicked(QListWidgetItem * item){
//    qDebug() << item;
//}

//void UDPTrans::itemPressed(QListWidgetItem * item){
//    qDebug() << item;
//}

/**
 * 查找widget的控件并更新信息
 * @brief UDPTrans::updateWidgetItem
 * @param key 键名 IPv4地址
 * @param di
 */
void UDPTrans:: updateWidgetItem(QString key,deviceItem di){
    QWidget* widget = ui->remoteDevice->itemWidget(newLanDevices[key].item);
    QLabel* q = widget->findChild<QLabel*>("deviceName");
    q->setText(di.deviceName);
    qDebug() << q->text();
}

void UDPTrans:: delWidgetItem(QString key){
    ui->remoteDevice->removeItemWidget(newLanDevices[key].item);
    newLanDevices[key].item = NULL;
}

/**
 * 打开系统文件管理器
 * @brief UDPTrans::openFile
 */
void UDPTrans:: openFile(){
    QObject* o = sender();
    QString ip = o->property("ip").toString();
    QString filePath = QFileDialog::getOpenFileName(this,"open","../");
    if(!filePath.isEmpty()){
        //QString fileName = "";
        //quint32 fileSize = 0;
        QFileInfo info(filePath);
        fileName = info.fileName();
        fileSize = info.size();

        //只读方式打开 file文件对象在私有成员变量中定义
        file.setFileName(filePath);
        bool succ = file.open(QIODevice::ReadOnly);
        if(succ){
            //file.read();
            //向文件接收方发送文件信息
            QString fi = QString("%1##%2").arg(fileName).arg(fileSize); //整型消息格式不能以QString格式发送 会被转成对应字符的ASCII码
            //qDebug() << fi;
            udpSocketFile->writeDatagram(fi.toUtf8().insert(0,MessageType::fileInfo),QHostAddress(ip),filePort);
        } else {
            qDebug() << "打开文件失败";
        }
    } else {
        qDebug() << "文件路径有误";
    }
}

/**
 * 打开消息发送窗口
 * @brief UDPTrans::openMsgDialog
 */
void UDPTrans:: openMsgDialog(){
    //qDebug() << "打开文件管理器";
    QObject* o = sender();
    QString ip = o->property("ip").toString();
    qDebug() << ip;
}
/**
 * 文件消息达到的时候弹出一个模态对话框进行确定
 * @brief UDPTrans::onSocketFileReadyRead
 */
void UDPTrans::onSocketFileReadyRead()
{
    while(udpSocketFile->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(udpSocketFile->pendingDatagramSize()));
        QHostAddress remoteIPv6Addr;     //远程主机地址ipv6
        //quint16 remotePort;             //远程主机UDP端口
        udpSocketFile->readDatagram(datagram.data(),datagram.size(),&remoteIPv6Addr,&remotePort);
        QString receiveData = QString::fromUtf8(datagram.data());
        remoteIPv4Addr = QHostAddress(remoteIPv6Addr.toIPv4Address()).toString();
        if(!receiveData.isEmpty()){
            parseFileMessage(datagram.data());
        }
    }
}

/**
 * 接收远程主机发送的文件消息
 * @brief UDPTrans::parseMessage
 */
void UDPTrans::parseFileMessage(QByteArray data)
{
    int first = data[0];            //第一个字节
    //qDebug() << first;
    QByteArray content;
    content.append(data.data() + 1, data.size() - 1);   //去掉data字节流的第一个字节
    if(MessageType::fileInfo == first){
        QString receiveData = QString::fromUtf8(content);
        //弹出模态对话框
        QString fileName = receiveData.split("##")[0];
        QString fileSize = receiveData.split("##")[1];
        receiveFile* rFile = new receiveFile(this);
        rFile->setIPv4(remoteIPv4Addr);
        rFile->setFileName(fileName);
        rFile->setFileSize(fileSize);
        saveFileName = fileName;
        saveDirPath = QCoreApplication::applicationDirPath() + "/receiveFiles";
        rFile->setSaveFilePath(saveDirPath);
        saveFilePath = saveDirPath + "/" + saveFileName;

        //rFile->exec();
        rFile->show();
    } else if(MessageType::acceptFile == first){
        //打开传输进度窗口 读取文件并发送
        //qDebug() << "接收方已同意,开始发送文件";
        quint64 unitBytes = 0;      //每次发送字节数
        quint64 totalBytes = 0;     //总发送字节数
        do {
            char buff[4096] = {0};
            unitBytes = file.read(buff,sizeof(buff));
            totalBytes += unitBytes;
            if(unitBytes > 0){
                unitBytes = udpSocketFile->writeDatagram(QByteArray(buff).insert(0,MessageType::fileContent),QHostAddress(remoteIPv4Addr),remotePort);
            }
        } while (unitBytes > 0);
        qDebug() << "文件传输完毕";   //文件发送完毕向接收方发送通知消息
        QByteArray msg;
        msg.append(MessageType::sentFile);
        udpSocketFile->writeDatagram(msg,QHostAddress(remoteIPv4Addr),remotePort);

        file.close();
//        progress* ps = new progress(this);
//        ps->exec();
    } else if(MessageType::fileContent == first){
        //接收文件内容 接收完毕必须要关闭文件
        qDebug() << content;
        receiveFileHandle.write(content);
    } else if(MessageType::rejectFile == first){

    } else if(MessageType::sentFile == first){
        //文件发送完毕 关闭文件句柄
        receiveFileHandle.close();
    }

}

void UDPTrans::on_remoteDevice_clicked(const QModelIndex &index)
{
    qDebug() << index;
}

/**
 * 同意接收文件并打开文件句柄
 * 如果打开成功则向文件发送方回馈一个通知消息
 * 如果打开失败则提示消息
 * @brief UDPTrans::acceptFile
 */
void UDPTrans::acceptFile()
{
    //打开接收文件句柄
    receiveFileHandle.setFileName(saveFilePath);
    bool succ = receiveFileHandle.open(QIODevice::ReadWrite);
    if(succ){
        QByteArray msg;
        msg.append(MessageType::acceptFile);
        udpSocketFile->writeDatagram(msg,QHostAddress(remoteIPv4Addr),remotePort);
    } else {
        QMessageBox::warning(this, tr("提示"),tr("打开文件句柄失败,无法保存文件"),QMessageBox::Ok,QMessageBox::Ok);
        exit(0);
    }

}

void UDPTrans::rejectFile()
{
    qDebug() << "拒绝接收文件";
}
