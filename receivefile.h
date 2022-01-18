#ifndef RECEIVEFILE_H
#define RECEIVEFILE_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

namespace Ui {
class Dialog;
}



class receiveFile : public QDialog
{
    Q_OBJECT
    public:
        explicit receiveFile(QWidget *parent = nullptr);
        void setIPv4(QString);
        void setFileName(QString);
        void setFileSize(QString);
        QString formateFileSize(QString);
        void setSaveFilePath(QString);
    signals:
        void acceptFile();
        void rejectFile();
    private:
        Ui::Dialog *ui;
        QString saveFilePath;

    private slots:

        void on_modifySaveFilePath_clicked();
        void on_pushButton_2_clicked();
        void on_pushButton_3_clicked();
};

#endif // RECEIVEFILE_H
