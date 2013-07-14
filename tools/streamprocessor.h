/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef STREAMPROCESSOR_H
#define STREAMPROCESSOR_H

#include "processor.h"
#include <QTcpSocket>
#include <QSemaphore>

class StreamProcessor : public Processor
{
        Q_OBJECT
    public:
        explicit StreamProcessor(TransformMgmt * tFactory,QObject *parent = 0);
        virtual void run();
    public slots:
        void stop();
    protected:
        QSemaphore runSem;
    private:
        Q_DISABLE_COPY(StreamProcessor)
        
};

#endif // STREAMPROCESSOR_H
