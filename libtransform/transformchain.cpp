/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "transformchain.h"

TransformChain::TransformChain()
{
    preferredFormat = TEXTFORMAT;
}

TransformChain::TransformChain(const TransformChain &other) : QList<TransformAbstract *>(other)
{
    name = other.name;
    description = other.description;
    help = other.help;
    preferredFormat = other.preferredFormat;
}

TransformChain &TransformChain::operator =(const TransformChain &other)
{
    QList<TransformAbstract *>::operator =(other);
    name = other.name;
    description = other.description;
    help = other.help;
    preferredFormat = other.preferredFormat;
    return *this;
}

TransformChain::~TransformChain()
{
}

void TransformChain::setName(const QString &nname)
{
    name = nname;
}

QString TransformChain::getName() const
{
    return name;
}

void TransformChain::setDescription(const QString &desc)
{
    description = desc;
}

QString TransformChain::getDescription() const
{
    return description;
}

void TransformChain::setHelp(const QString &helpString)
{
    help = helpString;
}

QString TransformChain::getHelp() const
{
    return help;
}

void TransformChain::setFormat(OutputFormat val)
{
    preferredFormat = val;
}

OutputFormat TransformChain::getFormat() const
{
    return preferredFormat;
}
