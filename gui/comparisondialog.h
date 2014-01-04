/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef COMPARISONDIALOG_H
#define COMPARISONDIALOG_H

#include "appdialog.h"
#include <QList>
#include <QColor>

namespace Ui {
class ComparisonDialog;
}

class QComboBox;
class TabAbstract;
class ByteSourceAbstract;
class QThread;

class CompareWorker : public QObject
{
        Q_OBJECT
    public:
        explicit CompareWorker(ByteSourceAbstract * sA, ByteSourceAbstract * sB, QObject *parent = 0);
        ~CompareWorker();
        quint64 getStartA() const;
        void setARange(const quint64 start, const quint64 end);
        quint64 getStartB() const;
        void setBRange(const quint64 start, const quint64 end);
        quint64 getSizeA() const;
        quint64 getSizeB() const;
        bool getMarkA() const;
        void setMarkA(bool value);
        bool getMarkB() const;
        void setMarkB(bool value);
        bool getMarkSame() const;
        void setMarkSame(bool value);
        quint64 getEndA() const;
        quint64 getEndB() const;
        QString getNameA() const;
        void setNameA(const QString &value);
        QString getNameB() const;
        void setNameB(const QString &value);
        QColor getMarkColor() const;
        void setMarkColor(const QColor &value);

    public slots:
        void compare();
        void stop();
    signals:
        void markingA(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor, QString toolTip);
        void markingB(quint64 start, quint64 end, const QColor &bgcolor,const QColor &fgColor, QString toolTip);
        void finishComparing(bool different);
        void progress(int percent);
    private:
        ByteSourceAbstract * sourceA;
        ByteSourceAbstract * sourceB;
        quint64 startA;
        quint64 startB;
        quint64 endA;
        quint64 endB;
        quint64 sizeA;
        quint64 sizeB;
        bool markA;
        bool markB;
        QString nameA;
        QString nameB;
        bool markSame;
        bool stopped;
        QColor marksColor;
};

class ComparisonDialog : public AppDialog
{
        Q_OBJECT
        
    public:
        explicit ComparisonDialog(GuiHelper *guiHelper ,QWidget *parent = 0);
        ~ComparisonDialog();
    private slots:
        void onTabSelection(int index);
        void onTabEntriesChanged();
        void onCompare();
        void loadTabs();
        void oncolorChange();
        void onAdvancedClicked(bool status);
        void onEntrySelected(int index);
        void endOfComparison(bool equals);
    private:
        static const QColor DEFAULT_MARKING_COLOR;
        void refreshEntries(QComboBox *entryBox, int count);
        void refreshTabs(QComboBox *tabBox);
        void changeIconColor(QColor color);
        Ui::ComparisonDialog *ui;
        QList<TabAbstract *> tabs;
        QColor marksColor;
        QThread * workerThread;
};

#endif // COMPARISONDIALOG_H
