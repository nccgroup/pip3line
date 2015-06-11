/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef GENERICTAB_H
#define GENERICTAB_H

#include "tababstract.h"
#include <QBitArray>
#include <QUrl>

namespace Ui {
class GenericTab;
}

class OffsetGotoWidget;
class SearchWidget;
class ByteSourceAbstract;
class HexView;
class ReadOnlyButton;
class ClearAllMarkingsButton;
class ByteSourceGuiButton;
class DetachTabButton;
class ByteTableView;
class MessagePanelWidget;
class QDragEnterEvent;
class QDropEvent;
class DownloadManager;
class TransformAbstract;

class GenericTab : public TabAbstract
{
        Q_OBJECT
    public:

        explicit GenericTab(ByteSourceAbstract *bytesource, GuiHelper *guiHelper, QWidget *parent = 0);
        ~GenericTab();
        void loadFromFile(QString fileName);
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int);
        ByteTableView *getHexTableView(int blockIndex);
        void setData(const QByteArray &data);
        bool canReceiveData();
        BaseStateAbstract *getStateMngtObj();

        OffsetGotoWidget *getGotoWidget() const;
        SearchWidget *getSearchWidget() const;
        MessagePanelWidget *getMessagePanel() const;

    private slots:
        void onSearch(QByteArray item, QBitArray mask, bool);
        void fileLoadRequest();
        void onGotoOffset(quint64 offset, bool absolute,bool negative, bool select);
        void onHistoryBackward();
        void onHistoryForward();
        void onNewViewTab(QAction * action);
        void onDeleteTab(int index);

    private:
        Q_DISABLE_COPY(GenericTab)
        enum ViewType {UNDEFINED = 0, HEXVIEW = 1, TEXTVIEW = 2, DEFAULTTEXT = 3};
        struct ViewTab {
                ViewTab() : transform(NULL), type(UNDEFINED),tabName(GuiConst::UNDEFINED_TEXT) {}
                TransformAbstract * transform;
                ViewType type;
                QString tabName;
        };
        static const QString TEXT_TEXT;
        bool eventFilter(QObject *obj, QEvent *event);
        void dragEnterEvent ( QDragEnterEvent * event );
        void dropEvent(QDropEvent *event);
        void integrateByteSource();
        void addViewTab(ViewTab data);

        Ui::GenericTab *ui;
        ByteSourceAbstract *bytesource;
        HexView *hexView;
        OffsetGotoWidget * gotoWidget;
        SearchWidget *searchWidget;
        ReadOnlyButton * roButton;
        ClearAllMarkingsButton * clearAllMarksButton;
        ByteSourceGuiButton * guiButton;
        DetachTabButton *detachButton;
        MessagePanelWidget * messagePanel;
        QAction * newHexViewAction;
        QAction * newTextViewAction;
        QAction * newDefaultTextViewAction;
        bool ableToReceiveData;
        QList<ViewTab> tabData;

        friend class GenericTabStateObj;
        friend class GenericTabClosingStateObj;
};

class GenericTabStateObj : public TabStateObj
{
        Q_OBJECT
    public:
        explicit GenericTabStateObj(GenericTab *tab);
        ~GenericTabStateObj();
        void run();
};

class GenericTabClosingStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit GenericTabClosingStateObj(GenericTab *tab);
        ~GenericTabClosingStateObj();
        void run();
        void setScrollIndex(int value);
        void setCurrentIndex(int value);

    protected:
        GenericTab *tab;
        int scrollIndex;
        int currentIndex;
};

#endif // GENERICTAB_H
