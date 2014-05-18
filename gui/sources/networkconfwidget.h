/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NETWORKCONFWIDGET_H
#define NETWORKCONFWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QHostAddress>

namespace Ui {
class NetworkConfWidget;
}

class NetworkConfWidget : public QWidget
{
        Q_OBJECT

    public:
        enum CONNECTYPE {TCP_CLIENT, TCP_SERVER, UDP_CLIENT, UDP_SERVER};
        explicit NetworkConfWidget(NetworkConfWidget::CONNECTYPE type,QWidget *parent = 0);
        ~NetworkConfWidget();

        quint16 getPort() const;
        void setPort(const quint16 &value);
        QHostAddress getIP() const;
        void setIP(const QHostAddress &value);
        void enableDecodeEncodeOption( bool enable);
    public slots:
        void onServerStarted();
        void onServerStopped();
    signals:
        void newIp(QHostAddress);
        void newPort(quint16);
        void start();
        void stop();
        void restart();
    private slots:
        void refreshIPs();
        void onIPselected(QString ips);
        void onIPEdited(QString ips);
        void onPortChanged(int val);
        void onStartStop();
    private:
        void configureType(CONNECTYPE type);
        Ui::NetworkConfWidget *ui;
        QStringList historicIPs;
        static const QString START_TEXT;
        static const QString STOP_TEXT;
        static const QString TCP_CLIENT_LABEL;
        static const QString TCP_SERVER_LABEL;
        static const QString UDP_CLIENT_LABEL;
        static const QString UDP_SERVER_LABEL;
};

#endif // NETWORKCONFWIDGET_H
