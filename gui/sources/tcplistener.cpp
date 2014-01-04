/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tcplistener.h"
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QTime>
#include <QCryptographicHash>
#include <QDebug>

const int TcpListener::BLOCK_MAX_SIZE = 0x8000000;

TcpListener::TcpListener(
#if QT_VERSION >= 0x050000
    qintptr nsocketDescriptor,
#else
    int nsocketDescriptor,
#endif
        QObject *parent) :
    BlocksSource(parent)
{
    separator = '\x0a';
    socket = NULL;
    remotePort = 0;
    socketDescriptor = nsocketDescriptor;
    qDebug() << "Created" << this;
}

TcpListener::TcpListener(QHostAddress remoteAddress, quint16 nport, QObject *parent) :
    BlocksSource(parent)
{
    separator = '\x0a';
    socket = NULL;
    socketDescriptor = 0;
    remotePeerAddress = remoteAddress;
    remotePort = nport;
}

TcpListener::~TcpListener()
{
    delete socket;
    qDebug() << "Destroyed" << this;
}

void TcpListener::startListening()
{
   // qDebug() << "Socket starts processing";

    socket = new(std::nothrow) QTcpSocket();
    if (socket == NULL) {
        qFatal("Cannot allocate memory for QTcpSocket X{");
    }

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(readyRead()), SLOT(onDataReceived()));

    if (remotePort == 0) {
        if (!socket->setSocketDescriptor(socketDescriptor)) {
            qCritical() << metaObject()->className() << tr("Invalid socket descriptor");
            delete socket;
            socket = NULL;
            return;
        }
        remotePeerAddress = socket->peerAddress();
        remotePort = socket->peerPort();
    } else {
        socket->connectToHost(remotePeerAddress, remotePort);
    }

    qDebug() << "socket opened" << remotePeerAddress.toString() << ":" << remotePort;

}

void TcpListener::stopListening()
{
    delete socket;
    socket = NULL;
    qDebug() << tr("End of client processing %1:%2").arg(remotePeerAddress.toString()).arg(remotePort);
    emit finished();
}

void TcpListener::onDataReceived()
{
//    qDebug() << "socket received data";
//    qDebug() << "Socket" << this << "performing long operation";
//    for (int i = 0; i < 1000000; i++)
//        QCryptographicHash::hash(QByteArray("vhsfdiuhfpishfpeirhjpfoerjre"), QCryptographicHash::Sha1);

    QByteArray data;
    QList<QByteArray> dataList;

    data = socket->readAll();

    if (data.size() > 0) {
        int count = data.count(separator);

        if (count > 0) {
            dataList = data.split(separator);
            tempData.append(dataList.takeFirst());
            processBlock(tempData);
            tempData.clear();
            count--;

            for (int i = 0 ; i < count ; i++) {
                processBlock(dataList.at(i));
            }

            if (count < dataList.size())
                tempData = dataList.last();
            else
                tempData.clear();

        } else {
            tempData.append(data);
            if (tempData.size() > BLOCK_MAX_SIZE) {
                tempData.resize(BLOCK_MAX_SIZE);
                qWarning() << this->metaObject()->className() << "Data received from the stream source  is too large, the block has been truncated." ;
                processBlock(tempData);
                tempData.clear();
            }
        }
    }
}

void TcpListener::onSocketError(QAbstractSocket::SocketError error)
{
    switch (error) {
        case QAbstractSocket::RemoteHostClosedError:
            qDebug() << "Disconnected socket" << remotePeerAddress.toString() << ":" << remotePort << QThread::currentThread();
            QTimer::singleShot(0,this,SLOT(stopListening()));
            break;
        case QAbstractSocket::SocketTimeoutError:
            break;
        default:
            qCritical() << metaObject()->className() << error << socket->errorString();
    }
}

void TcpListener::processBlock(QByteArray data)
{
    if (data.isEmpty()){
        QString mess = tr("Received data block is empty, ignoring.");
        qWarning() << metaObject()->className() << mess;
        emit error(mess,metaObject()->className());
        return;
    }

    if (base64Applied) {
        data = QByteArray::fromBase64(data);
        if (data.isEmpty()){
            QString mess = tr("Base64 decoded received data block is empty, ignoring.");
            qWarning() << metaObject()->className() << mess;
            emit error(mess,metaObject()->className());
            return;
        }
    }

    emit newBlock(data);
}

void TcpListener::sendBlock(const QByteArray &block)
{
    if (socket != NULL && socket->isWritable()) {
        qint64 size = block.size();
        qint64 byteWritten = socket->write(block);
        while (size > byteWritten) {
            byteWritten += socket->write(&(block.data()[byteWritten - 1]),size - byteWritten);
        }
    }
}
