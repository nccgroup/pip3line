/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef CENTRALPROCESSOR_H
#define CENTRALPROCESSOR_H

#include <QThread>
#include <transformabstract.h>
#include <QByteArray>
#include <QQueue>
#include <QSet>
#include <QMutex>
#include <QSemaphore>

#include "transformrequest.h"

class CentralProcessor : public QThread
{
        Q_OBJECT
    public:
        explicit CentralProcessor(QObject *parent = 0);
        ~CentralProcessor();
        void addToProcessingQueue(TransformRequest * request);
        void run();
        void stop();
    private slots:
        void onChildThreadDestroyed();
    private:
        QHash<quintptr, QMutex * > current;
        QHash<TransformRequest * , quintptr> children;
        QMutex globalMutex;
        QQueue<TransformRequest *> queue;
        QSemaphore queueSem;
        bool running;
        QMutex runMutex;
};

#endif // CENTRALPROCESSOR_H
