/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QHash>

#include "singleviewabstract.h"

namespace Ui {
    class HexView;
}

class QAction;
class GuiHelper;
class LoggerWidget;
class ByteSourceAbstract;
class TabAbstract;
class ByteItemModel;
class ByteTableView;
class QMenu;

class HexView : public SingleViewAbstract
{
        Q_OBJECT
    public:
        explicit HexView(ByteSourceAbstract *dataModel,GuiHelper *guiHelper, QWidget *parent = 0);
        ~HexView();
        void addCustomMenuActions(QAction * action);
        ByteTableView *getHexTableView();
    public slots:
        void onClearAllMArkings();
        bool goTo(quint64 offset, bool absolute,bool negative, bool select = false);
        void search(QByteArray item);
        void searchAgain();
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
        void onCopySelectedSize(QAction * action);
        void onCopyCurrentOffset(QAction *action);
        void onClearSelectionMarkings();
        void onLoadFile();
        void onSaveToFile(QAction *action);
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
        QMenu * copySelectedSizeMenu;
        QMenu * copyCurrentOffsetMenu;
        QMenu * loadMenu;
        QMenu * replaceMenu;
        QMenu * insertAfterMenu;
        QMenu * insertBeforeMenu;
        QMenu * saveToFileMenu;
        QHash<QAction *, TabAbstract *> sendToTabMapping;
        QAction * newMarkingAction;
        ByteItemModel * dataModel;
        ByteTableView * hexTableView;

        qint64 selectedSize;
        quint64 startOffset;
        static const QString NEW_BYTE_ACTION;
        static const QString BYTES_LE_ACTION;
        static const QString BYTES_BE_ACTION;
        static const QString OFFSET_LE_ACTION;
        static const QString OFFSET_BE_ACTION;
        static const QString OFFSET_REL_LE_ACTION;
        static const QString OFFSET_REL_BE_ACTION;
        static const QString SIZE_OCTAL_ACTION;
        static const QString SIZE_DECIMAL_ACTION;
        static const QString SIZE_HEXADECIMAL_ACTION;
};

#endif // HEXVIEW_H
