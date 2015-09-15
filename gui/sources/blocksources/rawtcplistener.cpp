#include "rawtcplistener.h"
#include <QDebug>
#include "../networkclientwidget.h"

const QString RawTcpListener::ID = QString("TCP Client");

RawTcpListener::RawTcpListener(QObject *parent) :
    BlocksSource(parent),
    remotePort(0)
{
    ssocket = NULL;
    type = CLIENT;
}

RawTcpListener::RawTcpListener(QHostAddress remoteAddress, quint16 remotePort, QObject *parent) :
    BlocksSource(parent),
    remotePeerAddress(remoteAddress),
    remotePort(remotePort)
{
    ssocket = NULL;
    type = CLIENT;
}

RawTcpListener::~RawTcpListener()
{
    delete ssocket;
    qDebug() << "Destroyed" << this;
}

void RawTcpListener::sendBlock(Block *block)
{
    if (ssocket != NULL && ssocket->isWritable()) {
        QByteArray data = block->getData();
        qint64 size = data.size();
        qint64 byteWritten = ssocket->write(data);
        while (size > byteWritten && ssocket->isWritable()) {
            byteWritten += ssocket->write(&(data.data()[byteWritten - 1]),size - byteWritten);
        }
    }
    delete block;
}

bool RawTcpListener::startListening()
{
    if (ssocket != NULL) {
         qCritical() << metaObject()->className() << tr("socket already exist, ignoring");
         return false;
    }
     ssocket = new(std::nothrow) QSslSocket();
     if (ssocket == NULL) {
         qFatal("Cannot allocate memory for QSslSocket X{");
     }
     connect(ssocket, SIGNAL(encrypted()), this, SLOT(ready()));
     connect(ssocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)));
     connect(ssocket, SIGNAL(disconnected()), this, SLOT(onClientDeconnection()));
     connect(ssocket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onError(QAbstractSocket::SocketError)));
//     connect(ssocket, SIGNAL(peerVerifyError(const QSslError &)),this, SLOT(onPeerVerificationError (const QSslError &)));
     connect(ssocket, SIGNAL(modeChanged(QSslSocket::SslMode)),this, SLOT(onSSLModeChange(QSslSocket::SslMode)));
     connect(ssocket, SIGNAL(readyRead()), SLOT(onDataReceived()));

     if (remotePort == 0) {
         qCritical() << metaObject()->className() << tr("TCP port is null");
         delete ssocket;
         ssocket = NULL;
         return false;
     } else {
        ssocket->connectToHost(remotePeerAddress, remotePort);
     }

     qDebug() << "socket opened" << remotePeerAddress.toString() << ":" << remotePort;

     ssocket->startClientEncryption();

     return true;
}

void RawTcpListener::stopListening()
{
    if (ssocket != NULL) {
        ssocket->disconnectFromHost();
        delete ssocket;
        ssocket = NULL;
    }

    qDebug() << tr("End of client processing %1:%2").arg(remotePeerAddress.toString()).arg(remotePort);
    if (!tempData.isEmpty()) {
        processBlock(tempData);
        tempData.clear();
    }
    emit stopped();
}

void RawTcpListener::onDataReceived()
{
    qDebug() << "Data received";
    processBlock(ssocket->readAll());
}

void RawTcpListener::ready()
{
    QSslSocket * sobject = static_cast<QSslSocket *>(sender());
    qDebug()<< "Client encrypted: " << sobject->peerAddress().toString() << "/" << sobject->peerPort();
}

void RawTcpListener::onSslErrors(const QList<QSslError> &errors)
{
    for (int i = 0; i < errors.size(); i++) {
        if (errors.at(i).error() == QSslError::SelfSignedCertificate) {
            emit log(tr("SSL: ").append(errors.at(i).errorString()), metaObject()->className(), Pip3lineConst::LWARNING);
        }
    }
}

void RawTcpListener::onError(QAbstractSocket::SocketError error)
{
    if (error != QAbstractSocket::RemoteHostClosedError) {
        QSslSocket * sobject = static_cast<QSslSocket *>(sender());
        emit log(sobject->errorString(), metaObject()->className(), Pip3lineConst::LERROR);
        emit connectionclosed();
    }
}

void RawTcpListener::onPeerVerificationError(const QSslError &error)
{
    QSslSocket * sobject = static_cast<QSslSocket *>(sender());
    qDebug()<< "SSL Peer error " << sobject->peerAddress().toString() << "/" << sobject->peerPort() << " : "
            << error.errorString();
}

void RawTcpListener::onSSLModeChange(QSslSocket::SslMode mode)
{
    QSslSocket * sobject = static_cast<QSslSocket *>(sender());
    qDebug() << "Mode changed for " << sobject->peerAddress().toString() << "/" << sobject->peerPort() << mode;
}

void RawTcpListener::processBlock(QByteArray data)
{
    if (data.size() > 0) {
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == NULL) qFatal("Cannot allocate memory for Block X{");
        emit blockReceived(datab);
    }
}

void RawTcpListener::onClientDeconnection()
{
    QSslSocket * sobject = static_cast<QSslSocket *>(sender());
    emit log(tr("Disconnection for %1/%2").arg(sobject->peerAddress().toString()).arg(sobject->peerPort()), metaObject()->className(), Pip3lineConst::LWARNING);
    emit connectionclosed();
}

QWidget *RawTcpListener::requestGui(QWidget *parent)
{
    NetworkClientWidget * ret = new(std::nothrow) NetworkClientWidget(this, parent);
    if (ret == NULL) {
        qFatal("Cannot allocate memory for NetworkClientWidget X{");
    }

    return ret;
}
quint16 RawTcpListener::getRemotePort() const
{
    return remotePort;
}

void RawTcpListener::setRemotePort(const quint16 &value)
{
    remotePort = value;
}

QString RawTcpListener::getName()
{
    return ID;
}

bool RawTcpListener::isReflexive()
{
    return false;
}

QHostAddress RawTcpListener::getRemotePeerAddress() const
{
    return remotePeerAddress;
}

void RawTcpListener::setRemotePeerAddress(const QHostAddress &value)
{
    remotePeerAddress = value;
}

