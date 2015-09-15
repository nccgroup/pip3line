#include "statestatuswidget.h"
#include "ui_statestatuswidget.h"

StateStatusWidget::StateStatusWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StateStatusWidget)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(50);
    ui->progressBar->setValue(0);
}

StateStatusWidget::~StateStatusWidget()
{
    delete ui;
}

void StateStatusWidget::log(QString , QString , Pip3lineConst::LOGLEVEL )
{
    ui->progressBar->setValue((ui->progressBar->value() + 1) % 50);
}
