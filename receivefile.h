#ifndef RECEIVEFILE_H
#define RECEIVEFILE_H

#include <QWidget>

namespace Ui {
class Dialog;
}

class receiveFile : public QWidget
{
    Q_OBJECT
    public:
        explicit receiveFile(QWidget *parent = nullptr);

    signals:

    private:
        Ui::Dialog *ui;

    private slots:

};

#endif // RECEIVEFILE_H
