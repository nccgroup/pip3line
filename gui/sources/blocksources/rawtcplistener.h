#ifndef RAWTCPLISTENER_H
#define RAWTCPLISTENER_H

#include "blockssource.h"
#include <QHostAddress>
#include <QSslSocket>

class RawTcpListener : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit RawTcpListener(QObject *parent = 0);
        explicit RawTcpListener(QHostAddress remoteAddress, quint16 remotePort, QObject *parent = 0);
        ~RawTcpListener();
        QHostAddress getRemotePeerAddress() const;
        void setRemotePeerAddress(const QHostAddress &value);
        quint16 getRemotePort() const;
        void setRemotePort(const quint16 &value);
        QString getName();
        bool isReflexive();
    signals:
        void connectionclosed();
    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();

    private slots:
        void onDataReceived();
        void ready();
        void onSslErrors(const QList<QSslError> & errors);
        void onError(QAbstractSocket::SocketError error);
        void onPeerVerificationError(const QSslError &error);
        void onSSLModeChange(QSslSocket::SslMode mode);
        void processBlock(QByteArray data);
        void onClientDeconnection();
    private:
        QWidget *requestGui(QWidget * parent);
        QSslSocket *ssocket;
        QByteArray tempData;
        QHostAddress remotePeerAddress;
        quint16 remotePort;
};

#endif // RAWTCPLISTENER_H
