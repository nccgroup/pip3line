/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMREQUEST_H
#define TRANSFORMREQUEST_H

#include <QThread>
#include <QMutex>
#include <transformabstract.h>

class TransformRequest : public QThread
{
        Q_OBJECT
    public:
        // takes ownership of the TransformAbstract object
        explicit TransformRequest(TransformAbstract *transform, const QByteArray &in, quintptr nptid);
        ~TransformRequest();
        void run();
        void setMutex(QMutex *mutex);
        quintptr getptid() const;
    signals:
        void finishedProcessing(QByteArray output, Messages messages);
    private slots:
        void logError(QString message, QString source = QString());
        void logWarning(QString message, QString source = QString());
        void logStatus(QString message, QString source = QString());
    private:
        void logMessage(QString message, QString source = QString(), LOGLEVEL level = LSTATUS);
        const QByteArray inputData;
        QByteArray outputData;
        TransformAbstract *transform;
        QList<Message> messagesList;
        QMutex *mutex;
        quintptr ptid;
        
};

#endif // TRANSFORMREQUEST_H
