/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QHash>
#include "../transformsgui.h"
#include "../sources/bytesourceabstract.h"
#include "../guihelper.h"
#include "../loggerwidget.h"
#include <QTextCodec>
#include <QPlainTextEdit>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QThread>

class RenderTextView : public QThread
{
        Q_OBJECT
    public:
        explicit RenderTextView(QObject *parent = 0);
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

namespace Ui {
    class TextView;
}

class TextView : public QWidget
{
        Q_OBJECT
        
    public:
        explicit TextView(ByteSourceAbstract *byteSource, GuiHelper *guiHelper, QWidget *parent = 0);
        ~TextView();
        void setModel(ByteSourceAbstract *byteSource);
        bool search(QString item);

    signals:
        void invalidText();
        void askForFileLoad();

    private slots:
        void updateSendToMenu();
        void updateImportExportMenu();
        void onTextChanged();
        void updateText(quintptr source);
        void textRenderingStarted();
        void textRenderingFinished();
        void reveceivingTextChunk(const QString &chunk);
        void updateStats();
        void onRightClick(QPoint pos);
        void onLoad(QAction *action);
        void onCopy(QAction *action);
        void onSaveToFile();
        void onLoadFile();
        void onSendToTab(QAction *action);
        void onSelectAll();
        void onKeepOnlySelection();
        void onCodecChange(QString codecName);

    private:
        Q_DISABLE_COPY(TextView)
        void buildContextMenu();
        void checkForEncodingError();
        bool eventFilter(QObject *obj, QEvent *event);
        QByteArray encode(QString text);
        static const int MAX_TEXT_VIEW;
        static const QString DEFAULT_CODEC;
        static const QString LOGID;
        QMenu * globalContextMenu;
        QMenu * sendToMenu;
        QMenu * copyMenu;
        QMenu * loadMenu;
        QAction * loadFileAction;
        QAction * saveToFileAction;
        QAction * sendToNewTabAction;
        QAction * selectAllAction;
        QAction * keepOnlySelectedAction;
        QHash<QAction *, TransformsGui *> sendToTabMapping;
        ByteSourceAbstract *byteSource;
        RenderTextView *renderThread;
        Ui::TextView *ui;
        GuiHelper *guiHelper;
        LoggerWidget *logger;

        QTextCodec *currentCodec;
};

#endif // TEXTVIEW_H
