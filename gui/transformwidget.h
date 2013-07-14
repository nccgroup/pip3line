/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QTableWidgetItem>
#include <QMenu>
#include <QColor>
#include <transformmgmt.h>
#include <transformabstract.h>
#include <QNetworkAccessManager>
#include "infodialog.h"
#include <QMutex>
#include <QTime>
#include <QUrl>
#include <QLineEdit>
#include "views/textview.h"
#include "sources/bytesourceabstract.h"
#include "downloadmanager.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include "views/hexview.h"

#include <QValidator>

class OffsetValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit OffsetValidator(QObject *parent = 0);
        State validate(QString & input, int & pos) const;

};

class HexView;
class TextView;

namespace Ui {
class TransformWidget;
}

class TransformWidget : public QWidget
{
    Q_OBJECT
    
    public:
        explicit TransformWidget(GuiHelper *guiHelper ,QWidget *parent = 0);
        ~TransformWidget();
        QByteArray output();
        TransformAbstract *getTransform();
        void forceUpdating();
        bool setTransform(TransformAbstract *transf);
        ByteSourceAbstract * getBytes() const;

    signals:
        void updated();
        void transfoRequest(TransformWidget *);
        void transformChanged();
        void confErrors(QString,QString);
        void error(QString, QString);
        void warning(const QString, const QString);
        void status(const QString, const QString);
        void deletionRequest(TransformWidget *);
        void tryNewName(QString name);
    public slots:
        void input(QByteArray inputdata);
        void updatingFrom();
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void logStatus(const QString message, const QString source = QString());
        void reset();
        void fromLocalFile(QString fileName);
    private slots:
        void refreshOutput();
        void onFileLoadRequest();
        void processingFinished(QByteArray output, Messages messages);
        void buildSelectionArea();
        void downloadFinished(DownloadManager *downloadManager);
        void updateView(quintptr source);
        void onInvalidText();
        void onTransformSelected(QString name);
        void updatingFromTransform();
        void onHistoryBackward();
        void onHistoryForward();
        void on_encodeRadioButton_toggled(bool checked);
        void on_decodeRadioButton_toggled(bool checked);
        void on_deleteButton_clicked();
        void on_infoPushButton_clicked();
        void on_clearMarkingsPushButton_clicked();
        void on_clearDataPushButton_clicked();
        void onGoToOffset(bool select = false);
        void onSearch(int modifiers);

    private:
        Q_DISABLE_COPY(TransformWidget)
        static const int MAX_DIRECTION_TEXT;
        static const QString NEW_BYTE_ACTION;
        void integrateTransform();
        void configureViewArea();
        void clearCurrentTransform();
        void addMessage(const QString &message, QColor color);
        void setDownload(QUrl url);
        void configureDirectionBox();
        bool eventFilter(QObject *obj, QEvent *event);
        void dragEnterEvent ( QDragEnterEvent * event );
        void dropEvent(QDropEvent *event);
        bool firstView;
        QNetworkAccessManager *manager;
        Ui::TransformWidget *ui;

        TransformAbstract * currentTransform;
        TransformMgmt *transformFactory;

        InfoDialog * infoDialog;

        GuiHelper * guiHelper;
        LoggerWidget *logger;
        QByteArray outputData;

        ByteSourceAbstract *byteSource;
        HexView *hexView;
        TextView *textView;
        QWidget *settingsTab;
};


#endif // TRANSFORMWIDGET_H
