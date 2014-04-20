/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/


#include "intercepsource.h"
#include <QDebug>

IntercepSource::IntercepSource(QObject *parent) :
    BasicSource(parent)
{
    capabilities = CAP_RESIZE | CAP_HISTORY | CAP_SEARCH | CAP_WRITE | CAP_COMPARE;
}

IntercepSource::~IntercepSource()
{

}

QString IntercepSource::name()
{
    return tr("Intercept source");
}

void IntercepSource::setData(QByteArray, quintptr)
{
    qWarning() << "Cannot reset data";
}

void IntercepSource::clear(quintptr )
{
    qWarning() << "Cannot clear an IntercepSource";
}

int IntercepSource::preferredTabType()
{
    return TAB_GENERIC;
}

bool IntercepSource::isReadableText()
{
    return false;
}
