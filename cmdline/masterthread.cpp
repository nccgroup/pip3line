/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "masterthread.h"
#include <QFile>
#include "../tools/textprocessor.h"
#include "../tools/binaryprocessor.h"
#include <QtCore/QCoreApplication>

MasterThread::MasterThread(bool nbinaryInput, bool hideErrorsFlag, bool verboseFlag, QObject *parent) :
    QThread(parent)
{
    verbose = verboseFlag;
    hideErrors = hideErrorsFlag;
    binaryInput = nbinaryInput;
    singleWay = TransformAbstract::INBOUND;
    errlog = new QTextStream(stderr);
    messlog = new QTextStream(stdout);
    connect(&transformFactory, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(&transformFactory, SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString)));
    transformFactory.initialize(QCoreApplication::applicationDirPath());
}

MasterThread::~MasterThread()
{
    delete errlog;
    delete messlog;
}

void MasterThread::run()
{
    QFile fin;
    QFile fout;
    Processor * pt = 0;

    fin.open(stdin, QFile::ReadOnly | QIODevice::Unbuffered);
    fout.open(stdout,QFile::WriteOnly | QIODevice::Unbuffered);

    if (binaryInput) {
        pt = new BinaryProcessor(&transformFactory);

    } else {
        pt = new TextProcessor(&transformFactory);
    }
    pt->setInput(&fin);
    pt->setOutput(&fout);
    if (!hideErrors)
        connect(pt,SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));

    if (verbose)
        connect(pt,SIGNAL(status(QString, QString)), this, SLOT(logStatus(QString)));

    if (!transformName.isEmpty() && pt->configureFromName(transformName, singleWay)) {
        pt->start();
        pt->wait();
    } else if (!confFileName.isEmpty() && pt->configureFromFile(confFileName)) {
        pt->start();
        pt->wait();
    }

    delete pt;

    QCoreApplication::exit(0);
}

void MasterThread::setConfigurationFile(const QString &fileName, bool inbound)
{
    confFileName = fileName;
    if (inbound) {
        singleWay = TransformAbstract::INBOUND;
    } else {
        singleWay = TransformAbstract::OUTBOUND;
    }
}

void MasterThread::setTransformName(const QString &ntransformName, bool inbound)
{
    transformName = ntransformName;
    if (inbound) {
        singleWay = TransformAbstract::INBOUND;
    } else {
        singleWay = TransformAbstract::OUTBOUND;
    }
}

void MasterThread::logError(const QString mess, const QString &source)
{
    if (!hideErrors) {
        if (!source.isEmpty()) {
            *errlog << source << ": ";
        }
        *errlog << mess << endl;
    }
}

void MasterThread::logStatus(const QString mess, const QString &source)
{
    if (verbose) {
        if (!source.isEmpty()) {
            *messlog << source << ": ";
        }
        *messlog << mess << endl;
    }
}
