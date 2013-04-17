/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "rendertextview.h"
#include <QTimer>

RenderTextView::RenderTextView(QObject *parent) :
    QThread(parent)
{
    textRenderingChunkSize = 500;
    running = false;
}

RenderTextView::~RenderTextView()
{
}

void RenderTextView::run()
{
    running = true;
    int numBlock = 0;
    QString value;

    runMutex.lock();
    while (running) {
        runMutex.unlock();

        dataSem.acquire(1);
        runMutex.lock();
        if (!running)
            break;
        runMutex.unlock();
        dataMutex.lock();
        if (textViewDisplayQueue.isEmpty()) {
            dataMutex.unlock();
            runMutex.lock();
            continue;
        } else {
            value = textViewDisplayQueue.dequeue();
            dataMutex.unlock();
            numBlock = (value.size() / textRenderingChunkSize) + 1;
            emit startingRendering();
            for (int i = 0; i < numBlock; i++) {
                emit dataChunk(value.mid(i*textRenderingChunkSize,textRenderingChunkSize));
            }
            emit finishedRendering();
        }
        runMutex.lock();
    }
    runMutex.unlock();
}

void RenderTextView::stop()
{
    runMutex.lock();
    running = false;
    runMutex.unlock();
    dataSem.release(1);
}

void RenderTextView::setDataForRendering(const QString &text)
{
    dataMutex.lock();
    textViewDisplayQueue.enqueue(text);
    dataMutex.unlock();
    dataSem.release(1);
}

