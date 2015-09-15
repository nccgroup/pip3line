/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef INTERCEPTORCHESTRATORABSTRACT_H
#define INTERCEPTORCHESTRATORABSTRACT_H

#include <QObject>
#include "../blocksources/blockssource.h"
#include "payloadmodel.h"

class InterceptOrchestratorAbstract : public QObject
{
        Q_OBJECT
    public:
        explicit InterceptOrchestratorAbstract(QObject *parent = 0);
        virtual ~InterceptOrchestratorAbstract();
        bool isForwarding() const;
        PayloadModel *getModel() const;
        void setModel(PayloadModel *value);
        bool hasStandbyPayload() const;
        virtual void forwardStandbyPayload() = 0;
        virtual int requiredNumOfSources() = 0;
        virtual quint16 acceptablesSourcesTypes() = 0;
        Payload *getStandbyPayload() const;
        void setStandbyPayload(Payload *value);
        void clearStandbyPayload();
        QString getName() const;
        QString getDescription() const;
    public slots:
        virtual void receiveBlock(Block *block) = 0;
        virtual void setForwarding(bool value);
    signals:
        void standbyPayloadUpdated();
        void queueUpdated(int size);
    protected:
        bool forwarding;
        PayloadModel *model;
        Payload *standbyPayload;
        QString name;
        QString description;
};

#endif // INTERCEPTORCHESTRATORABSTRACT_H
