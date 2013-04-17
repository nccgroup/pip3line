/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QUrl>
#include <QMutex>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QUrl>
#include "byteitemmodel.h"

class DownloadManager : public QObject
{
        Q_OBJECT
    public:
        explicit DownloadManager(QUrl &url,QNetworkAccessManager * networkManager, ByteItemModel *destModel = 0, QObject *parent = 0);
        ~DownloadManager();
        bool launch();
        QByteArray getData();
    signals:
        void error(QString, QString);
        void warning(QString, QString);
        void finished(DownloadManager *);
    public slots:
        void requestFinished();
        void networkSSLError(QList<QSslError> sslError);
    private:
        static const QString ID;

        QNetworkAccessManager * networkManager;
        QMutex dataMutex;
        QNetworkReply * reply;

        ByteItemModel *model;
        QByteArray data;
        QUrl resource;
};

#endif // DOWNLOADMANAGER_H
