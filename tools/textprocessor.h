/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include "processor.h"

class TextProcessor: public Processor
{
    public:
        explicit TextProcessor(TransformMgmt *tFactory, QObject * parent = 0);
        ~TextProcessor();
        void run();
    protected:
        TextProcessor() {}
    private:
        Q_DISABLE_COPY(TextProcessor)
};

#endif // TEXTPROCESSOR_H
