/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RENDERTEXTVIEW_H
#define RENDERTEXTVIEW_H

#include <QPlainTextEdit>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QThread>

class RenderTextView : public QThread
{
        Q_OBJECT
    public:
        RenderTextView(QObject *parent = 0);
        ~RenderTextView();
        void run();
        void stop();
    signals:
        void startingRendering();
        void finishedRendering();
        void dataChunk(QString chunk);
    public slots:
        void setDataForRendering(const QString &text);
    private :
        QQueue<QString> textViewDisplayQueue;
        int textRenderingChunkSize;
        QSemaphore dataSem;
        bool running;
        QMutex runMutex;
        QMutex dataMutex;
};

#endif // RENDERTEXTVIEW_H
