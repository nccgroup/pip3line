/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INTERCEPTORPROXY_H
#define INTERCEPTORPROXY_H

#include <QHash>
#include <QQueue>
#include <QTimerEvent>
#include "payload.h"
#include "interceptorchestratorabstract.h"

class InterceptorProxy : public InterceptOrchestratorAbstract
{
        Q_OBJECT
    public:
        explicit InterceptorProxy(QObject *parent = 0);
        ~InterceptorProxy();
        BlocksSource *getLeft() const;
        void setLeft(BlocksSource *value);
        BlocksSource *getRight() const;
        void setRight(BlocksSource *value);
        void forwardStandbyPayload();
        int requiredNumOfSources();
        quint16 acceptablesSourcesTypes();
    public slots:
        void receiveBlock(Block *block);
    private:
        bool verifySanity();
        void registerBlockSource(BlocksSource *bs);
        void timerEvent(QTimerEvent *event);
        void flushQueue();
        BlocksSource *left;
        BlocksSource *right;
        QHash<quint32, quint32> translationTable;
        QQueue<Payload *> packetQueue;
        int timer;
};

#endif // INTERCEPTORPROXY_H
