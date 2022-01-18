#ifndef PROGRESS_H
#define PROGRESS_H

#include <QDialog>

namespace Ui {
class progress;
}

class progress : public QDialog
{
    Q_OBJECT

public:
    explicit progress(QWidget *parent = nullptr);
    ~progress();

private:
    Ui::progress *ui;
};

#endif // PROGRESS_H
