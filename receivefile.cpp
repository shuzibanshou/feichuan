#include "receivefile.h"
#include "ui_receivefile.h"

receiveFile::receiveFile(QWidget *parent) : QDialog(parent),ui(new Ui::Dialog)
{
    //ui一定要记得初始化
    ui->setupUi(this);
}

void receiveFile::setIPv4(QString ipv4)
{
    ui->senderIPv4->setText(ipv4);
}

void receiveFile::setFileName(QString fileName)
{
    ui->receiveFileName->setText(fileName);
}

void receiveFile::setFileSize(quint64 fileSize)
{
    ui->receiveFileSize->setText(QString::number(fileSize));
}


