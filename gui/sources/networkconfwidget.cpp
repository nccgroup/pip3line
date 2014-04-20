#include "networkconfwidget.h"
#include "ui_networkconfwidget.h"
#include <QHostAddress>
/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QNetworkInterface>

const QString NetworkConfWidget::TCP_CLIENT_LABEL = "TCP Client";
const QString NetworkConfWidget::TCP_SERVER_LABEL = "TCP Server";
const QString NetworkConfWidget::UDP_CLIENT_LABEL = "UDP Client";
const QString NetworkConfWidget::UDP_SERVER_LABEL = "UDP Server";
const QString NetworkConfWidget::REFRESH_INTERFACES = "Refresh interfaces";

NetworkConfWidget::NetworkConfWidget(NetworkConfWidget::CONNECTYPE type,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkConfWidget)
{
    ui->setupUi(this);
    configureType(type);
    connect (ui->ipComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(refreshIPs(QString)));
    connect(ui->ipComboBox, SIGNAL(editTextChanged(QString)), SLOT(onIPEdited(QString)));
}

NetworkConfWidget::~NetworkConfWidget()
{
    delete ui;
}

void NetworkConfWidget::configureType(NetworkConfWidget::CONNECTYPE type)
{

    switch (type) {
    case TCP_CLIENT:
        ui->protoLabel->setText(TCP_CLIENT_LABEL);
        break;
    case TCP_SERVER:
        ui->protoLabel->setText(TCP_SERVER_LABEL);
        ui->ipComboBox->addItem(REFRESH_INTERFACES);
        break;
    case UDP_CLIENT:
        ui->protoLabel->setText(UDP_CLIENT_LABEL);
        break;
    case UDP_SERVER:
        ui->protoLabel->setText(UDP_SERVER_LABEL);
        ui->ipComboBox->addItem(REFRESH_INTERFACES);
        break;
    default:
        qFatal("[NetworkConfWidget] Invalid proto type");
    }
}
QString NetworkConfWidget::getIP() const
{
    return IP;
}

void NetworkConfWidget::setIP(const QString &value)
{
    IP = value;
    if (!historicIPs.contains(value)) {
        historicIPs.append(value);
    }
}

quint16 NetworkConfWidget::getPort() const
{
    return port;
}

void NetworkConfWidget::setPort(const quint16 &value)
{
    port = value;
}

void NetworkConfWidget::refreshIPs()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QList<QString> slist;

    for (int i = 0; i < list.size(); i++) {
        slist.append(list.at(i).toString());
    }
    int index = slist.indexOf("127.0.0.1");

    ui->ipComboBox->clear();
    ui->ipComboBox->addItems(slist);
    if (index != -1)
        ui->ipComboBox->setCurrentIndex(index);

    ui->ipComboBox->addItem(REFRESH_INTERFACES);
}

void NetworkConfWidget::onIPselected(QString ips)
{
    if (ips == REFRESH_INTERFACES) {
        ui->ipComboBox->blockSignals(true);
        refreshIPs();
        ui->ipComboBox->blockSignals(false);
    } else {
        IP = ips;
    }

}

void NetworkConfWidget::onIPEdited(QString ips)
{
    setIP(ips);
}

