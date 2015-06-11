#include "networkclientwidget.h"
#include "ui_networkclientwidget.h"

const QString NetworkClientWidget::START_STRING = "Start";
const QString NetworkClientWidget::STOP_STRING = "Stop";

NetworkClientWidget::NetworkClientWidget(RawTcpListener *nlistener, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkClientWidget)
{
    listener = nlistener;
    ui->setupUi(this);
    ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-start.png"));
    ui->startStopPushButton->setText(START_STRING);

    connect(ui->startStopPushButton, SIGNAL(clicked()), SLOT(onStartStop()));
    connect(ui->restartPushButton, SIGNAL(clicked()), SLOT(onRestart()));

    if (listener != NULL) {
        connect(this, SIGNAL(start()), listener, SLOT(startListening()));
        connect(this, SIGNAL(stop()), listener, SLOT(stopListening()));
        connect(this, SIGNAL(restart()), listener, SLOT(restart()));
        connect(listener, SIGNAL(connectionclosed()), this, SLOT(reset()));
    }

}

NetworkClientWidget::~NetworkClientWidget()
{
    delete ui;
}

QHostAddress NetworkClientWidget::getIPAddress()
{
    return QHostAddress(ui->peerAddrLineEdit->text());
}

quint16 NetworkClientWidget::getPort()
{
    return (quint16) ui->portSpinBox->value();
}

void NetworkClientWidget::reset()
{
    ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-start.png"));
    ui->startStopPushButton->setText(START_STRING);
}

void NetworkClientWidget::onStartStop()
{
    if (ui->startStopPushButton->text() == START_STRING) {
        ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-stop-7.png"));
        ui->startStopPushButton->setText(STOP_STRING);
        if (listener != NULL) {
            listener->setRemotePeerAddress(QHostAddress(ui->peerAddrLineEdit->text()));
            listener->setRemotePort((quint16) ui->portSpinBox->value());
        }
        emit start();
    }
    else {
        ui->startStopPushButton->setIcon(QIcon(":/Images/icons/media-playback-start.png"));
        ui->startStopPushButton->setText(START_STRING);
        emit stop();
    }
}

void NetworkClientWidget::onRestart()
{
    if (listener != NULL) {
        listener->setRemotePeerAddress(QHostAddress(ui->peerAddrLineEdit->text()));
        listener->setRemotePort((quint16) ui->portSpinBox->value());
    }
    emit restart();
}
