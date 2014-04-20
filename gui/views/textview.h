/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QHash>
#include <QMutex>
#include <QQueue>
#include <QSemaphore>
#include <QThread>

#include "singleviewabstract.h"

class QTextCodec;
class TabAbstract;
class QMenu;
class QAction;

namespace Ui {
    class TextView;
}

class TextView : public SingleViewAbstract
{
        Q_OBJECT
        
    public:
        explicit TextView(ByteSourceAbstract *byteSource, GuiHelper *guiHelper, QWidget *parent = 0);
        ~TextView();
        void setModel(ByteSourceAbstract *byteSource);
        void search(QByteArray item, QBitArray mask = QBitArray());

    signals:
        void invalidText();
        void searchStatus(bool);

    private slots:
        void updateSendToMenu();
        void updateImportExportMenu();
        void onTextChanged();
        void updateText(quintptr source);
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
        QHash<QAction *, TabAbstract *> sendToTabMapping;

        Ui::TextView *ui;


        QTextCodec *currentCodec;
        bool errorNotReported;
};

#endif // TEXTVIEW_H
