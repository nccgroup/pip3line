/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "downloadmanager.h"
#include <QMutexLocker>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QVariant>
#include <QUrl>
#include <QDebug>
#include "loggerwidget.h"

const QString DownloadManager::ID = "DownloadManager";

DownloadManager::DownloadManager(QUrl &url, GuiHelper *guiHelper,QObject *parent) :
    QObject(parent),
    guiHelper(guiHelper)
{
    resource = url;
    networkManager = guiHelper->getNetworkManager();
}

DownloadManager::~DownloadManager()
{
    qDebug() << "Destroying " << this;
}

bool DownloadManager::launch()
{
    if (networkManager != NULL) {

        createRequest(resource);

    } else {
        guiHelper->getLogger()->logError(tr("No network manager, ignoring download request"),ID);
        return false;
    }
    return true;
}

QByteArray DownloadManager::getData()
{
    // there should not be any need for a mutex here, as getData and requestFisnihed should never execute concurrently
    if (data.size() == 0) {
        guiHelper->getLogger()->logWarning(tr("No data for collection"),ID);
    }
    return data;
}

void DownloadManager::requestFinished()
{
    QNetworkReply * reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == NULL) {
        qFatal("DownloadManager::requestFinished reply is NULL");
    }

    if (reply->error()) {
        guiHelper->getLogger()->logError(tr("[Failed to load \"%1\"] %2").arg(resource.toString()).arg(reply->errorString()),ID);
    } else {
        QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (possibleRedirectUrl.isEmpty() || (!previousRedirect.isEmpty() && previousRedirect == possibleRedirectUrl)) {
            // there should not be any need for a mutex here, as getData and requestFisnihed should never execute concurrently
            data = reply->readAll();
            qDebug() << tr("%1 successfully loaded (%2 bytes) ").arg(resource.toEncoded().constData()).arg(data.size());
            emit finished(data);
            deleteLater();
        } else {
            createRequest(possibleRedirectUrl);
        }
    }
    reply->deleteLater();
}

void DownloadManager::networkSSLError(QList<QSslError> sslErrors)
{
    for (int i = 0; i < sslErrors.size(); i++) {
        guiHelper->getLogger()->logError(sslErrors.at(i).errorString(),ID);
    }
}

void DownloadManager::createRequest(QUrl url)
{
    QNetworkRequest request(url);
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    qDebug() << "ssl verify" << QSslConfiguration::defaultConfiguration().peerVerifyMode();
    QNetworkReply * reply = networkManager->get(request);
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(networkSSLError(QList<QSslError>)));
    connect(reply,SIGNAL(finished()), this, SLOT(requestFinished()));
    if (QSslConfiguration::defaultConfiguration().peerVerifyMode() == QSslSocket::VerifyNone)
        reply->ignoreSslErrors();
}
