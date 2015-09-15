#ifndef NETWORKCLIENTWIDGET_H
#define NETWORKCLIENTWIDGET_H

#include <QHostAddress>
#include <QWidget>
#include "blocksources/rawtcplistener.h"

namespace Ui {
    class NetworkClientWidget;
}

class NetworkClientWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit NetworkClientWidget(RawTcpListener * listener, QWidget *parent = 0);
        ~NetworkClientWidget();
        QHostAddress getIPAddress();
        quint16 getPort();
    public slots:
        void reset();
    private slots:
        void onStartStop();
        void onRestart();
    signals:
        void start();
        void stop();
        void restart();

    private:
        static const QString START_STRING;
        static const QString STOP_STRING;
        Ui::NetworkClientWidget *ui;
        RawTcpListener * listener;
};

#endif // NETWORKCLIENTWIDGET_H
