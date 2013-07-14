/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QColor>
#include <QHash>
#include "../transformsgui.h"
#include "../sources/bytesourceabstract.h"
#include "byteitemmodel.h"
#include "bytetableview.h"
#include "../loggerwidget.h"
#include "../guihelper.h"

namespace Ui {
    class HexView;
}

class HexView : public QWidget
{
        Q_OBJECT
    public:
        explicit HexView(ByteSourceAbstract *dataModel,GuiHelper *guiHelper, QWidget *parent = 0);
        ~HexView();
    signals:
        void markingStatus(bool hasMarking);
        void askForFileLoad();
    public slots:
        void onClearAllMArkings();
        bool goTo(qint64 offset, bool absolute, bool select = false);
        bool search(QByteArray item, bool flag = false);
    private slots:
        void updateStats();
        void onRightClick(QPoint pos);
        void updateMarkMenu();
        void updateImportExportMenus();
        void updateSendToMenu();

        void onCopy(QAction * action);
        void onLoad(QAction * action);
        void onReplace(QAction * action);
        void onInsertAfter(QAction * action);
        void onInsertBefore(QAction * action);
        void onSendToTab(QAction * action);
        void onMarkMenu(QAction * action);
        void onSelectFromSizeMenu(QAction * action);
        void onGotoFromOffsetMenu(QAction * action);
        void onClearSelectionMarkings();
        void onLoadFile();
        void onSaveToFile();
        void onSelectAll();
        void onKeepOnlySelection();
        void onNewByteArray();
        void onDeleteSelection();
        void onSelectionChanged();

    private:
        Q_DISABLE_COPY(HexView)
        void buildContextMenus();
        quint64 normalizeSelectedInt(bool bigEndian);
        Ui::HexView *ui;
        QMenu * globalContextMenu;
        QMenu * sendToMenu;
        QMenu * markMenu;
        QMenu * selectFromSizeMenu;
        QMenu * gotoFromOffsetMenu;
        QMenu * copyMenu;
        QMenu * loadMenu;
        QMenu * replaceMenu;
        QMenu * insertAfterMenu;
        QMenu * insertBeforeMenu;
        QHash<QAction *, TransformsGui *> sendToTabMapping;
        QAction * newMarkingAction;
        ByteItemModel * dataModel;
        ByteTableView * hexTableView;
        ByteSourceAbstract *byteSource;
        GuiHelper * guiHelper;
        LoggerWidget *logger;
        static const QString NEW_BYTE_ACTION;
        static const QString BYTES_LE_ACTION;
        static const QString BYTES_BE_ACTION;
};

#endif // HEXVIEW_H
