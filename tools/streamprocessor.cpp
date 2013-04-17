/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "streamprocessor.h"

StreamProcessor::StreamProcessor(TransformMgmt *tFactory, QObject *parent) :
    Processor(tFactory,parent)
{
}

void StreamProcessor::run()
{
    QByteArray data;
    QByteArray buffer;
    QList<QByteArray> dataList;

    while (true) {
        bool dataPresent = in->waitForReadyRead(3000);
        if (dataPresent) {
            data = in->readAll();
            if (data.size() == 0) {
                if (runSem.tryAcquire())
                    break;
                continue;
            }

            int count = data.count(separator);

            if (count > 0) {
                dataList = data.split(separator);
                buffer.append(dataList.takeFirst());

                writeBlock(buffer);

                count--;

                for (int i = 0 ; i < count ; i++) {
                    writeBlock(dataList.at(i));
                }

                if (count < dataList.size())
                    buffer = dataList.last();
                else
                    buffer.clear();

            } else {
                buffer.append(data);
            }
        }

        if (runSem.tryAcquire())
            break;

    }
    if (!buffer.isEmpty())
        writeBlock(buffer);

}

void StreamProcessor::stop()
{
    runSem.release();
}
