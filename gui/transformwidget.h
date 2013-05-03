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
#include "byteitemmodel.h"
#include "bytetableview.h"
#include "infodialog.h"
#include "ui_tabname.h"
#include <QMutex>
#include <QTime>
#include <QUrl>
#include "downloadmanager.h"
#include "guihelper.h"
#include "loggerwidget.h"
#include "rendertextview.h"

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
    signals:
        void updated();
        void transfoRequest(TransformWidget *);
        void transformChanged();
        void confErrors(QString,QString);
        void error(QString, QString);
        void warning(const QString, const QString);
        void status(const QString, const QString);
        void deletionRequest(TransformWidget *);
    public slots:
        void input(QByteArray);
        void updatingFrom();
        void logWarning(const QString message, const QString source = QString());
        void logError(const QString message, const QString source = QString());
        void logStatus(const QString message, const QString source = QString());
        void clearMessages();
        void reset();
    private slots:
        void updateImportExportMenus();
        void updateSendToMenu();
        void updateMarkMenu();
        void refreshOutput();
        void processingFinished(QByteArray output, Messages messages);
        void buildSelectionArea();
        void downloadFinished(DownloadManager *downloadManager);
        void updateView(ByteItemModel::UpdateSource source);
        void updateStats();
        void onSelectionChanged();
        void onInvalidText();
        void dragEnterEvent ( QDragEnterEvent * event );
        void dropEvent(QDropEvent *event);
        void onTransformConfError(const QString mess, const QString source = QString());
        void onTransformSelected(QString name);
        void onRightClick(QPoint pos);
        void updatingFromTransform();
        void onCopy(QAction * action);
        void onImport(QAction * action);
        void onReplace(QAction * action);
        void onInsertAfter(QAction * action);
        void onInsertBefore(QAction * action);
        void onHistoryBackward();
        void onHistoryForward();
        void on_encodeRadioButton_toggled(bool checked);
        void on_decodeRadioButton_toggled(bool checked);
        void on_actionImport_From_File_triggered();
        void on_actionSave_to_file_triggered();
        void on_deleteButton_clicked();
        void on_actionDelete_selected_bytes_triggered();
        void addNullBytes(char byteSample = '\00', int pos = 0, int count = 1);
        void on_infoPushButton_clicked();
        void on_actionSelect_all_triggered();
        void on_actionKeep_only_Selected_triggered();
        void onMenuSendToTriggered(QAction *);
        void onMenuMarkTriggered(QAction *action);
        void on_clearMarkingsPushButton_clicked();
        void on_actionClear_marking_triggered();
        void on_clearDataPushButton_clicked();
        void on_actionNew_byte_array_triggered();

    private:
        static const int MAX_DIRECTION_TEXT;
        static const QString NEW_BYTE_ACTION;
        void integrateTransform();
        QByteArray getSelectedBytes();
        void buildContextMenus();
        void configureViewArea();
        void clearCurrentTransform();
        void addMessage(const QString &message, QColor color);
        void setDownload(QUrl url);
        void configureDirectionBox();
        bool firstView;
        QNetworkAccessManager *manager;
        Ui::TransformWidget *ui;
        QMenu * plainTextContextMenu;
        QMenu * globalContextMenu;
        QMenu * sendToMenu;
        QMenu * markMenu;
        QMenu * copyMenu;
        QMenu * importMenu;
        QMenu * replaceMenu;
        QMenu * insertAfterMenu;
        QMenu * insertBeforeMenu;
        QHash<QAction *, TransformsGui *> sendToActions;
        TransformAbstract * currentTransform;
        TransformMgmt *transformFactory;
        ByteItemModel * dataModel;
        ByteTableView * hexTableView;
        InfoDialog * infoDialog;
        Ui::TabName nameUi;
        QDialog * nameDialog;
        GuiHelper * guiHelper;
        LoggerWidget *logger;
        QByteArray inputData;
        QByteArray outputData;
        QMutex processingMutex;
};

#endif // TRANSFORMWIDGET_H
