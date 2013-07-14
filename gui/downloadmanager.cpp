/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "downloadmanager.h"
#include <QMutexLocker>
#include <QDebug>

const QString DownloadManager::ID = "DownloadManager";

DownloadManager::DownloadManager(QUrl &url,QNetworkAccessManager * nnetworkManager, ByteSourceAbstract *destModel, QObject *parent) :
    QObject(parent)
{
    resource = url;
    networkManager = nnetworkManager;
    model = destModel;
}

DownloadManager::~DownloadManager()
{
    qDebug() << "Destroying " << this;
}

bool DownloadManager::launch()
{
    if (networkManager != NULL) {
        reply = networkManager->get(QNetworkRequest(resource));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(networkSSLError(QList<QSslError>)));
        connect(reply,SIGNAL(finished()), this, SLOT(requestFinished()));
    } else {
        emit error(tr("No network manager, ignoring download request"),ID);
        return false;
    }
    return true;
}

QByteArray DownloadManager::getData()
{
    QMutexLocker lock(&dataMutex);
    if (data.size() == 0) {
        emit warning(tr("No data for collection"), ID);
    }
    return data;
}

void DownloadManager::requestFinished()
{
    if (reply->error()) {
        emit error(tr("[Failed to load \"%1\"] %2\n").arg(resource.toString()).arg(reply->errorString()),ID);
    } else {
        qDebug() << tr("%1 successfully loaded (%2 bytes) ").arg(resource.toEncoded().constData()).arg(reply->size());
        if (model != NULL) {
            model->setData(reply->readAll());
        } else {
            dataMutex.lock();
            data = reply->readAll();
            dataMutex.unlock();
        }
    }
    reply->deleteLater();
    emit finished(this);
}

void DownloadManager::networkSSLError(QList<QSslError> sslErrors)
{
    for (int i = 0; i < sslErrors.size(); i++) {
        emit error(sslErrors.at(i).errorString(),ID);
    }
}
