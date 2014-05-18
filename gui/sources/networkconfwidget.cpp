/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "networkconfwidget.h"
#include "ui_networkconfwidget.h"
#include <QHostAddress>
#include <QDebug>
#include <QNetworkInterface>

const QString NetworkConfWidget::TCP_CLIENT_LABEL = "TCP Target";
const QString NetworkConfWidget::TCP_SERVER_LABEL = "TCP Listener";
const QString NetworkConfWidget::UDP_CLIENT_LABEL = "UDP Target";
const QString NetworkConfWidget::UDP_SERVER_LABEL = "UDP Listener";
const QString NetworkConfWidget::START_TEXT = "Start";
const QString NetworkConfWidget::STOP_TEXT = "Stop";

NetworkConfWidget::NetworkConfWidget(NetworkConfWidget::CONNECTYPE type,QWidget *parent) :
    QWidget(parent)
{
    ui = new(std::nothrow) Ui::NetworkConfWidget;
    if (ui == NULL) {
        qFatal("Cannot allocate memory for Ui::NetworkConfWidget X{");
    }
    ui->setupUi(this);
    refreshIPs();
    ui->startStopPushButton->setText(START_TEXT);
    configureType(type);
    connect (ui->ipComboBox, SIGNAL(currentIndexChanged(QString)), SLOT(onIPselected(QString)));
    connect(ui->ipComboBox, SIGNAL(editTextChanged(QString)), SLOT(onIPEdited(QString)));
    connect(ui->refreshPushButton, SIGNAL(clicked()), SLOT(refreshIPs()));
    connect(ui->startStopPushButton, SIGNAL(clicked()), this, SLOT(onStartStop()));
    connect(ui->restartPushButton, SIGNAL(clicked()), this, SIGNAL(restart()));
    connect(ui->portSpinBox, SIGNAL(valueChanged(int)), SLOT(onPortChanged(int)));
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
        break;
    case UDP_CLIENT:
        ui->protoLabel->setText(UDP_CLIENT_LABEL);
        break;
    case UDP_SERVER:
        ui->protoLabel->setText(UDP_SERVER_LABEL);
        break;
    default:
        qFatal("[NetworkConfWidget] Invalid proto type");
    }
}

quint16 NetworkConfWidget::getPort() const
{
    return (quint16)ui->portSpinBox->value();
}

void NetworkConfWidget::setPort(const quint16 &value)
{
    ui->portSpinBox->setValue((int)value);
}

QHostAddress NetworkConfWidget::getIP() const
{
    return QHostAddress(ui->ipComboBox->currentText());
}

void NetworkConfWidget::setIP(const QHostAddress &value)
{
    int index = 0;
    QString IP = value.toString();
    if (IP.isEmpty())
        return;
    index = historicIPs.indexOf(IP);
    if (index == -1) {
        index = historicIPs.size();
        historicIPs.append(IP);
        ui->ipComboBox->clear();
        ui->ipComboBox->addItems(historicIPs);
    }

    ui->ipComboBox->setCurrentIndex(index);
}

void NetworkConfWidget::enableDecodeEncodeOption(bool enable)
{
    ui->encodeCheckBox->setVisible(enable);
    ui->decodeCheckBox->setVisible(enable);
}

void NetworkConfWidget::onServerStarted()
{
    ui->startStopPushButton->setText(STOP_TEXT);
    ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-stop-7.png"));
}

void NetworkConfWidget::onServerStopped()
{
    ui->startStopPushButton->setText(START_TEXT);
    ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-start.png"));
}

void NetworkConfWidget::refreshIPs()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QList<QString> slist;

    for (int i = 0; i < list.size(); i++) {
        slist.append(list.at(i).toString());
    }
    int index = slist.indexOf("127.0.0.1");
    historicIPs = slist;
    ui->ipComboBox->clear();
    ui->ipComboBox->addItems(slist);
    if (index != -1)
        ui->ipComboBox->setCurrentIndex(index);
    else
        ui->ipComboBox->setCurrentIndex(0);
}

void NetworkConfWidget::onIPselected(QString ips)
{
    if (!historicIPs.contains(ips)) {
        historicIPs.append(ips);
    }
    emit newIp(QHostAddress(ips));
}

void NetworkConfWidget::onIPEdited(QString ips)
{
    if (!historicIPs.contains(ips)) {
        historicIPs.append(ips);
    }
    emit newIp(QHostAddress(ips));
}

void NetworkConfWidget::onPortChanged(int val)
{
    qDebug() << "port " << val;
    if (val > 0 && val < 65535) {
        emit newPort((quint16) val);
    }
}

void NetworkConfWidget::onStartStop()
{
    if (ui->startStopPushButton->text().compare(START_TEXT) == 0) {
        emit start();
    } else {
        emit stop();
    }
}

