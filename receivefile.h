#ifndef RECEIVEFILE_H
#define RECEIVEFILE_H

#include <QDialog>

namespace Ui {
class Dialog;
}

class receiveFile : public QDialog
{
    Q_OBJECT
    public:
        explicit receiveFile(QWidget *parent = nullptr);
        void setIPv4(QString ipv4);
        void setFileName(QString fileName);
        void setFileSize(quint64 fileSize);

    signals:

    private:
        Ui::Dialog *ui;

    private slots:

};

#endif // RECEIVEFILE_H
