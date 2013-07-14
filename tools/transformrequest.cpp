/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformrequest.h"
#include <QMutexLocker>
#include <QTime>
#include <QDebug>

TransformRequest::TransformRequest(TransformAbstract *ntransform, const QByteArray &in, quintptr nptid, bool takeOwnerShip) :
    QThread(0),
    inputData(in)
{
    transform = ntransform;
    connect(transform, SIGNAL(error(QString,QString)), this, SLOT(logError(QString,QString)),Qt::DirectConnection);
    connect(transform, SIGNAL(warning(QString,QString)), this, SLOT(logWarning(QString,QString)),Qt::DirectConnection);
    deleteObject = takeOwnerShip;
    mutex = NULL;
    ptid = nptid;

}

TransformRequest::~TransformRequest()
{
    if (deleteObject)
        delete transform;
}

void TransformRequest::run()
{
    QMutexLocker lock(mutex);

    QTime timer;
    timer.start();
    if (transform != NULL) {
        transform->transform(inputData,outputData);
    } else {
        qWarning() << "[TransformRequest] transform is null, ignoring";
    }

 //   qDebug() << "Processor fisnished: " << this <<  " exec time: " << timer.elapsed() << "ms";

    emit finishedProcessing(outputData, messagesList);

    deleteLater();
}

void TransformRequest::setMutex(QMutex *nmutex)
{
    mutex = nmutex;
}

quintptr TransformRequest::getptid() const
{
    return ptid;
}

void TransformRequest::logError(QString message, QString source)
{
    logMessage(message, source, LERROR);
}

void TransformRequest::logWarning(QString message, QString source)
{
    logMessage(message, source, LWARNING);
}

void TransformRequest::logStatus(QString message, QString source)
{
    logMessage(message, source, LSTATUS);
}


void TransformRequest::logMessage(QString message, QString source, LOGLEVEL level)
{
    Message m;
    m.level = level;
    m.message = message;
    m.source = source;
    messagesList.append(m);
}
