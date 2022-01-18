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
