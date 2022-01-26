#include "progress.h"
#include "ui_progress.h"

progress::progress(QWidget *parent) :QDialog(parent),ui(new Ui::progress)
{
    ui->setupUi(this);
}

progress::~progress()
{
    delete ui;
}

void progress::setRange(qint32 min, qint32 max)
{
    ui->progressBar->setRange(min,max);
}

void progress::setValue(qint32 val)
{
    ui->progressBar->setValue(val);
}

