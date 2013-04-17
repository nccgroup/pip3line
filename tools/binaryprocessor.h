/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BINARYPROCESSOR_H
#define BINARYPROCESSOR_H

#include "processor.h"

class BinaryProcessor: public Processor
{
    public:
        static const int BLOCKSIZE = 4096;
        BinaryProcessor(TransformMgmt *tFactory, QObject * parent = 0);
        ~BinaryProcessor();
        void run();
    private:
        BinaryProcessor() {}
};

#endif // BINARYPROCESSOR_H
