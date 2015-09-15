#include "interceptsetupwidget.h"
#include "ui_interceptsetupwidget.h"
#include "../blocksources/blockssource.h"
#include "../blocksources/rawtcplistener.h"
#include "../blocksources/tcplistener.h"
#include "../blocksources/tcpserverlistener.h"
#include "../blocksources/udplistener.h"

const QStringList InterceptSetUpWidget::TypeList = QStringList() << "One side (sniffing , client, server, external)" << "Intercepting proxy";

InterceptSetUpWidget::InterceptSetUpWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterceptSetUpWidget)
{
    ui->setupUi(this);
    // top level set up
    ui->typeComboBox->addItems(TypeList);
    ui->typeComboBox->setCurrentIndex(0);
    ui->stackedWidget->setCurrentIndex(ui->typeComboBox->currentIndex());
    connect(ui->typeComboBox, SIGNAL(currentIndexChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));

    // unary page set-up
    ui->uSourceComboBox->addItems(getUnaryList());

    ui->plSourceComboBox->addItems(getUnaryList(false));
    ui->prSourceComboBox->addItems(getUnaryList(false));


}

InterceptSetUpWidget::~InterceptSetUpWidget()
{
    delete ui;
}

QStringList InterceptSetUpWidget::getUnaryList(bool includeReflexive)
{
    QStringList list;
    list << RawTcpListener::ID;
    if (includeReflexive) {
        list << TcpListener::ID;
        list << TcpServerListener::ID;
        list << UdpListener::ID;
    }
    return list;
}
