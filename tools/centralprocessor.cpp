/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "centralprocessor.h"
#include <QMutexLocker>
#include <QHashIterator>
#include <QDebug>

CentralProcessor::CentralProcessor(QObject *parent) :
    QThread(parent)
{
    running = false;
    qDebug() << "created " << this;
}

CentralProcessor::~CentralProcessor()
{
    QMutexLocker lock(&globalMutex);
    if (!children.isEmpty()) {
        qWarning("[CentralProcessor] children still running ... waiting");
        QHashIterator<TransformRequest * , quintptr> i(children);
         while (i.hasNext()) {
             i.next();
             bool eop = ((TransformRequest *) i.key())->wait(100);
             if (!eop)
                 qWarning("[CentralProcessor] children still running ... now ignoring T_T");
         }
    }

    foreach (QMutex * value, current)
         delete value;

    current.clear();
    qDebug() << "Destroying " << this;

}

void CentralProcessor::addToProcessingQueue(TransformRequest *request)
{
    QMutexLocker lock(&globalMutex);

    if (children.contains(request)) {
        qWarning("[CentralProcessor::addToProcessingQueue] request already there T_T");
        return;
    }

    queue.enqueue(request);
    queueSem.release(1);
}

void CentralProcessor::run()
{
    running = true;
    QMutex * mutex = NULL;
    quintptr source = 0;
    runMutex.lock();
    while (running) {
        runMutex.unlock();

        queueSem.acquire(1);
        globalMutex.lock();
        if (!queue.isEmpty()) {
            TransformRequest * request = queue.dequeue();
            source = request->getptid();
            if (current.contains(source)) {
                mutex = current.value(source, NULL);
            } else {
                mutex = new(std::nothrow) QMutex();
                if (mutex == NULL)
                    qFatal("Cannot allocate memory for QMutex in CentralProcessor X{");

                current.insert(source, mutex);
            }

            request->setMutex(mutex);
            children.insert(request, source);
            connect(request, SIGNAL(destroyed()), this, SLOT(onChildThreadDestroyed()));
            request->start();
        }
        globalMutex.unlock();
        runMutex.lock();
    }
    runMutex.unlock();
}

void CentralProcessor::stop()
{
    runMutex.lock();
    running = false;
    globalMutex.lock();
    queueSem.release(1);
    globalMutex.unlock();
    runMutex.unlock();
}

void CentralProcessor::onChildThreadDestroyed()
{
    TransformRequest * tr = (TransformRequest *) sender();
    quintptr source = tr->getptid();
    QMutexLocker lock(&globalMutex);

    if (children.remove(tr) != 1)
        qWarning("[CentralProcessor::onChildThreadDelete] value not found when removing child T_T");

    QList<quintptr> list = children.values();
    if (!list.contains(source)) {
        delete current.value(source); // deleting the mutex
        if (current.remove(source) != 1)
            qWarning("[CentralProcessor::onChildThreadDelete] value not found when removing source T_T");
    }
}
