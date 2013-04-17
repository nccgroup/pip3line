/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "binaryprocessor.h"

BinaryProcessor::BinaryProcessor(TransformMgmt * tFactory, QObject *parent) :
    Processor(tFactory, parent)
{
}

BinaryProcessor::~BinaryProcessor()
{

}

void BinaryProcessor::run()
{
    QByteArray temp;

    char buffer[BLOCKSIZE];
    qint64 whatHasBeenDone = 0;

    if (tlist.isEmpty()) {
        emit error(tr("Error: No transformation configured, exiting the process."), "Binary Processor");
        return;
    }

    while (!in->atEnd()) {
        whatHasBeenDone = in->read(buffer,BLOCKSIZE);

        if (whatHasBeenDone == -1) {
            emit error(in->errorString(), "Binary Processor");
            break;
        }
        temp.append(buffer,whatHasBeenDone);
    }

    writeBlock(temp);

}
