/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "textprocessor.h"
#include <QDebug>

TextProcessor::TextProcessor(TransformMgmt * tFactory, QObject *parent) :
                             Processor(tFactory, parent)
{
}

TextProcessor::~TextProcessor()
{
}

void TextProcessor::run()
{
    if (in == NULL) {
        logError(tr("Input is not defined, TextProcessor will not run"), "TextProcessor");
        return;
    }

    if (out == NULL) {
        logError(tr("output is not defined, TextProcessor will not run"), "TextProcessor");
        return;
    }

    QByteArray temp;
    QTextStream ins(in);
    stats.reset();

    while (!ins.atEnd()) {
        temp = ins.readLine().toUtf8();

        writeBlock(temp);

    }
}
