/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "interceptorchestratorabstract.h"

InterceptOrchestratorAbstract::InterceptOrchestratorAbstract(QObject *parent) :
    QObject(parent)
{
    forwarding = false;
    name = "NoNAME";
    description = "NoDescription";
    standbyPayload = NULL;
    model = new(std::nothrow) PayloadModel(this);
    if (model == NULL) qFatal("Cannot alocate memory for PayloadModel X{");
}

InterceptOrchestratorAbstract::~InterceptOrchestratorAbstract()
{

}

bool InterceptOrchestratorAbstract::isForwarding() const
{
    return forwarding;
}

void InterceptOrchestratorAbstract::setForwarding(bool value)
{
    forwarding = value;
}
QString InterceptOrchestratorAbstract::getDescription() const
{
    return description;
}

QString InterceptOrchestratorAbstract::getName() const
{
    return name;
}

Payload *InterceptOrchestratorAbstract::getStandbyPayload() const
{
    return standbyPayload;
}

void InterceptOrchestratorAbstract::setStandbyPayload(Payload *value)
{
    standbyPayload = value;
}

void InterceptOrchestratorAbstract::clearStandbyPayload()
{
    standbyPayload = NULL;
}

PayloadModel *InterceptOrchestratorAbstract::getModel() const
{
    return model;
}

void InterceptOrchestratorAbstract::setModel(PayloadModel *value)
{
    model = value;
}

bool InterceptOrchestratorAbstract::hasStandbyPayload() const
{
    return standbyPayload != NULL;
}



