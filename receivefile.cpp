#include "receivefile.h"
#include "ui_receivefile.h"

receiveFile::receiveFile(QWidget *parent) : QDialog(parent),ui(new Ui::Dialog)
{
    //ui一定要记得初始化
    ui->setupUi(this);
    connect(this, SIGNAL(acceptFile()), parent, SLOT(acceptFile()));
    connect(this, SIGNAL(rejectFile()), parent, SLOT(rejectFile()));
}

void receiveFile::setIPv4(QString ipv4)
{
    ui->senderIPv4->setText(ipv4);
}

void receiveFile::setFileName(QString fileName)
{
    ui->receiveFileName->setText(fileName);
}

void receiveFile::setFileSize(QString fileSize)
{
    ui->receiveFileSize->setText(formateFileSize(fileSize));
}

/**
 * 格式化输出文件大小
 * @brief receiveFile::formateFileSize
 * @param fileSize
 * @return
 */
QString receiveFile::formateFileSize(QString fileSize)
{
    float num = fileSize.toFloat();
    QStringList list;
    list << "KB" << "MB" << "GB" << "TB";

    QStringListIterator i(list);
    QString unit("bytes");

    while(num >= 1024.0 && i.hasNext())
    {
     unit = i.next();
     num /= 1024.0;
    }
    return QString().setNum(num,'f',2)+" "+unit;
}

/**
 * 设置UI控件显示的文件默认保存目录
 * @brief receiveFile::setSaveFilePath
 * @return
 */
void receiveFile::setSaveFilePath(QString m_saveFilePath)
{
    saveFilePath = m_saveFilePath;
    ui->saveFilePath->setText(saveFilePath);
}

/**
 * 选择接收文件存储目录
 * @brief receiveFile::on_modifySaveFilePath_clicked
 */
void receiveFile::on_modifySaveFilePath_clicked()
{
    //文件夹路径
    saveFilePath = QFileDialog::getExistingDirectory(this, "更改接收文件存储路径","/");
    ui->saveFilePath->setText(saveFilePath);
}

/**
 * 接收新文件
 * @brief receiveFile::on_pushButton_2_clicked
 */
void receiveFile::on_pushButton_2_clicked()
{
    //先检测目录是否存在 不存在则尝试创建目录
    QDir dir(saveFilePath);
    if(!dir.exists() && dir.mkpath(saveFilePath)){
        QMessageBox::warning(this, tr("提示"),tr("该目录不存在,且无法创建\n请检查权限并手动创建"),QMessageBox::Ok,QMessageBox::Ok);
        return;
    }
    //向父窗口发送接收文件消息
    emit acceptFile();
    //关闭弹窗
    this->close();
}

/**
 * 拒收新文件
 * @brief receiveFile::on_pushButton_2_clicked
 */
void receiveFile::on_pushButton_3_clicked()
{
    //向父窗口发送拒收文件消息
    emit rejectFile();
}
