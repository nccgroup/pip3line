/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "interceptorproxy.h"

InterceptorProxy::InterceptorProxy(QObject *parent) :
    InterceptOrchestratorAbstract(parent)
{
    left = NULL;
    right = NULL;
    timer = 0;
    name = "Intercepting Proxy";
    description = "Proxy used to perform man-in-the-middle accross different protocols";
}

InterceptorProxy::~InterceptorProxy()
{
    if (timer != 0)
        killTimer(timer);
    delete left;
    delete right;
}

void InterceptorProxy::receiveBlock(Block *block)
{

    if (!verifySanity()) {
        delete block;
        return;
    }

    BlocksSource * bs = BlocksSource::getSourceObject(block->getSourceid());

    Payload * payl = new(std::nothrow) Payload(block);
    if (payl == NULL) {
        qFatal("Cannot allocate Payload X{");
    }

    if (bs == left) {
        payl->setDirection(Payload::LEFTRIGHT);
        bs = right;
    } else if (bs == right){
        payl->setDirection(Payload::RIGHTLEFT);
        bs = left;
    } else {
        qCritical("[InterceptorProxy::receiveBlock] Cannot determine blocks source id, not transferring the block T_T");
        bs = NULL;
    }

    if (forwarding) {
        flushQueue();
        if (bs != NULL) bs->postBlockForSending(block);
        model->addPayload(payl);
    } else {
        packetQueue.enqueue(payl);
        emit queueUpdated(packetQueue.size());
    }
}

bool InterceptorProxy::verifySanity()
{
    if (left == NULL ) {
        qCritical("[InterceptorProxy::verifySanity] Left source is NULL T_T");
        return false;
    }

    if (right == NULL) {
        qCritical("[InterceptorProxy::verifySanity] Right source is NULL T_T");
        return false;
    }
    return true;
}

void InterceptorProxy::registerBlockSource(BlocksSource *bs)
{
    connect(bs, SIGNAL(blockReceived(Block *)), this, SLOT(receiveBlock(Block*)));
    if (timer == 0)
        timer = startTimer(200);
}

void InterceptorProxy::timerEvent(QTimerEvent *)
{
    if (!packetQueue.isEmpty()) {
        if (forwarding) {
            flushQueue();
            standbyPayload = NULL;
        } else if (standbyPayload == NULL) {
            Payload * payl = packetQueue.dequeue();
            model->addPayload(payl);
            standbyPayload = payl;
            emit queueUpdated(packetQueue.size());
        }
        emit standbyPayloadUpdated();
    }
}

void InterceptorProxy::flushQueue()
{
    if (!verifySanity()) {
        return;
    }

    while (!packetQueue.isEmpty()) {
        Payload * payl = packetQueue.dequeue();
        BlocksSource * bt = BlocksSource::getSourceObject(payl->getSourceid());
        if (bt == left) {
            right->postBlockForSending(payl->toBlock());
        } else if (bt == right) {
            left->postBlockForSending(payl->toBlock());
        } else {
            qCritical("[InterceptorProxy::flushQueue] Cannot determine blocks source, not sending packet T_T");
        }
        model->addPayload(payl);
    }
    emit queueUpdated(0);
}

BlocksSource *InterceptorProxy::getRight() const
{
    return right;
}

void InterceptorProxy::setRight(BlocksSource *value)
{
    delete right; // no need to check for NULL value
    right = value;
    registerBlockSource(right);
}

void InterceptorProxy::forwardStandbyPayload()
{
    if (!verifySanity()) {
        return;
    }

    if (standbyPayload != NULL) {
        BlocksSource * bt = BlocksSource::getSourceObject(standbyPayload->getSourceid());
        if (bt == left) {
            right->postBlockForSending(standbyPayload->toBlock());
        } else if (bt == right) {
            left->postBlockForSending(standbyPayload->toBlock());
        } else {
            qCritical("[InterceptorProxy::forwardStandbyPayload] Cannot determine blocks source, not sending payload T_T");
            return;
        }

        if (forwarding) {
            flushQueue();
            standbyPayload = NULL; // payloads own by the model , don't touch
        } else {
            if (!packetQueue.isEmpty()) {
                Payload * payl = packetQueue.dequeue();
                model->addPayload(payl);
                standbyPayload = payl;
                emit queueUpdated(packetQueue.size());
            } else {
                standbyPayload = NULL;
            }
        }

        emit standbyPayloadUpdated();
    }
}

int InterceptorProxy::requiredNumOfSources()
{
    return 2;
}

quint16 InterceptorProxy::acceptablesSourcesTypes()
{
    return BlocksSource::CLIENT | BlocksSource::SERVER;
}

BlocksSource *InterceptorProxy::getLeft() const
{
    return left;
}

void InterceptorProxy::setLeft(BlocksSource *value)
{
    delete left; // no need to check for NULL value
    left = value;
    registerBlockSource(left);
}

