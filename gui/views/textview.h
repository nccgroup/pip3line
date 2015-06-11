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
#include <commonstrings.h>

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
        explicit TextView(ByteSourceAbstract *byteSource, GuiHelper *guiHelper, QWidget *parent = 0, bool takeByteSourceOwnership = false);
        ~TextView();
        void setModel(ByteSourceAbstract *byteSource);
        void search(QByteArray item, QBitArray mask = QBitArray());
        void copyToClipboard();
        bool isAutoCopyToClipboard() const;
    public slots:
        void setAutoCopyToClipboard(bool value);

    signals:
        void invalidText();
        void searchStatus(bool);
    private slots:
        void updateSendToMenu();
        void updateImportExportMenu();
        void onTextChanged();
        void updateText(quintptr source);
        void updateStats();
        void onRightClick(QPoint pos);
        void onLoad(QAction *action);
        void onCopy(QAction *action);
        void onLoadFile();
        void onSendToTab(QAction *action);
        void onSelectAll();
        void onKeepOnlySelection();
        void onCodecChange(QString codecName);
        void onReadOnlyChanged(bool viewIsReadonly);
        void onSaveToFile(QAction* action);
    private:
        Q_DISABLE_COPY(TextView)
        void buildContextMenu();
        void checkForEncodingError();
        bool eventFilter(QObject *obj, QEvent *event);
        QByteArray encode(QString text);
        static const int MAX_TEXT_VIEW;
        static const QString DEFAULT_CODEC;
        static const QString LOGID;
        static const QString COPY_AS_TEXT;
        QMenu * globalContextMenu;
        QMenu * sendToMenu;
        QMenu * copyMenu;
        QMenu * loadMenu;
        QAction * loadFileAction;
        QAction * sendToNewTabAction;
        QAction * selectAllAction;
        QAction * keepOnlySelectedAction;
        QAction * copyAsTextAction;
        QMenu * saveToFileMenu;
        QHash<QAction *, TabAbstract *> sendToTabMapping;
        Ui::TextView *ui;
        QTextCodec *currentCodec;
        bool errorNotReported;
        bool autoCopyToClipboard;
};

#endif // TEXTVIEW_H
