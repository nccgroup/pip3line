/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RANDOMACCESSTAB_H
#define RANDOMACCESSTAB_H

#include "tababstract.h"
#include <commonstrings.h>
#include <QBitArray>

namespace Ui {
class RandomAccessTab;
}

class OffsetGotoWidget;
class SearchWidget;
class ByteSourceAbstract;
class HexView;
class GuiHelper;
class ReadOnlyButton;
class ClearAllMarkingsButton;
class ByteSourceGuiButton;
class DetachTabButton;
class ByteTableView;

class RandomAccessTab : public TabAbstract
{
        Q_OBJECT
        
    public:
        explicit RandomAccessTab(ByteSourceAbstract *bytesource, GuiHelper *guiHelper, QWidget *parent = 0);
        ~RandomAccessTab();
        int getBlockCount() const;
        ByteSourceAbstract *getSource(int blockIndex);
        ByteTableView *getHexTableView(int blockIndex);
        void loadFromFile(QString);
        void setData(const QByteArray &);
        bool canReceiveData();

    private slots:
        void fileLoadRequest();
        void onSearch(QByteArray item, QBitArray mask,  bool);
        void onGotoOffset(quint64 offset, bool absolute, bool negative, bool select);
        void log(QString mess,QString source,Pip3lineConst::LOGLEVEL level);
        void onCloseLogView();

    private:
        Q_DISABLE_COPY(RandomAccessTab)
        static const QString LOGID;
        void integrateByteSource();
        ByteSourceAbstract *byteSource;
        HexView *hexView;
        OffsetGotoWidget * gotoWidget;
        SearchWidget *searchWidget;
        Ui::RandomAccessTab *ui;
        ReadOnlyButton * roButton;
        ClearAllMarkingsButton * clearAllMarksButton;
        ByteSourceGuiButton * guiButton;
        DetachTabButton *detachButton;
        bool eventFilter(QObject *obj, QEvent *event);
};

#endif // RANDOMACCESSTAB_H
