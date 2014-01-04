/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef GENERICTAB_H
#define GENERICTAB_H

#include "tababstract.h"

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
    private slots:
        void onSearch(QByteArray item, bool);
        void fileLoadRequest();
        void onGotoOffset(quint64 offset, bool absolute,bool negative, bool select);
        void onHistoryBackward();
        void onHistoryForward();

    private:
        Q_DISABLE_COPY(GenericTab)
        static const QString LOGID;
        bool eventFilter(QObject *obj, QEvent *event);
        void integrateByteSource();
        ByteSourceAbstract *byteSource;
        Ui::GenericTab *ui;
        HexView *hexView;
        OffsetGotoWidget * gotoWidget;
        SearchWidget *searchWidget;
        ReadOnlyButton * roButton;
        ClearAllMarkingsButton * clearAllMarksButton;
        ByteSourceGuiButton * guiButton;
        DetachTabButton *detachButton;
};

#endif // GENERICTAB_H
