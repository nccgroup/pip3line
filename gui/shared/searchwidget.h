/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <QColor>
#include <QBitArray>
#include <QAbstractListModel>
#include <QTime>
#include "../sources/bytesourceabstract.h"

class GuiHelper;
class ByteSourceAbstract;
class QFocusEvent;
class QPushButton;
class QEvent;
class SearchAbstract;
class SearchResultsWidget;
class BytesRange;
class FloatingDialog;

class FoundOffsetsModel : public QAbstractListModel
{
        Q_OBJECT
    public:
        explicit FoundOffsetsModel(QObject * parent = 0);
        ~FoundOffsetsModel();
        void setSearchDelegate(SearchAbstract * searchDelegate);
        SearchAbstract *getSearchObject() const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        quint64 getStartingOffset(const QModelIndex & index);
        quint64 getStartingOffset(const int index);
        quint64 getEndOffset(const QModelIndex & index);
        static bool lessThanFoundOffset(BytesRange* i1, BytesRange* i2);
        void startSearch();
        int elapsed();
        BytesRangeList *getRanges() const;
    signals:
        void updated();
    public slots:
        void clear();
        void setNewList(BytesRangeList *list);
        void onRangeDestroyed();
    private:
        BytesRangeList * ranges;
        SearchAbstract * searchDelegate;
        QTime searchTimer;
};

class SearchLine : public QLineEdit
{
        Q_OBJECT
    public:
        explicit SearchLine(ByteSourceAbstract *source, QWidget *parent = 0);
        ~SearchLine();
        ByteSourceAbstract *getBytesource() const;
        void setBytesource(ByteSourceAbstract *source);

    public slots:
        void setError(bool val);
        void updateProgress(double val);
        void onSearchStarted();
        void onSearchEnded();
    private slots:
        void onSourceUpdated(quintptr);
    signals:
        void newSearch(QString val, int modifiers);
        void requestJumpToNext();
    private:
        static const int MAX_TEXT_SIZE;
        void focusInEvent(QFocusEvent * event);
        void paintEvent(QPaintEvent * event);
        void keyPressEvent(QKeyEvent * event);
        double progress;
        bool searching;
        quint64 sourceSize;
        QTime timer;
        ByteSourceAbstract *bytesource;
        static const QColor LOADING_COLOR;
};

class SearchWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit SearchWidget(ByteSourceAbstract *source, GuiHelper *nguiHelper, QWidget *parent = 0);
        ~SearchWidget();
        QString text();
        void setText(QString data);
        void setStopVisible(bool val);
        FoundOffsetsModel *getModel() const;
        ByteSourceAbstract *getBytesource() const;

        SearchAbstract *getSearchDelegate() const;
        void setSearchDelegate(SearchAbstract *delegate);

    public slots:
        void setError(bool val);
        void onSearchStarted();
        void onSearchEnded();
        void updateStatusProgress(double val);
        void clearSearch();
        void nextFind(quint64 pos = 0);
    private slots:
        void onSearch(QString val, int modifiers);
        void onAdvanced();
        void processJump(quint64 start, quint64 end);
        void onRequestNext();
    signals:
        void searchRequest(QByteArray data,QBitArray mask, bool couldBeText);
        void stopSearch();
        void jumpTo(quint64 start, quint64 end);
    private:
        static const QString FIND_PLACEHOLDER_TEXT;
        static const QString TOOLTIP_TEXT;
        static const QString PLACEHOLDER_DISABLED_TEXT;
        SearchLine * lineEdit;
        QPushButton *stopPushButton;
        QPushButton *advancedPushButton;
        FoundOffsetsModel *model;
        SearchResultsWidget * resultWidget;
        ByteSourceAbstract *bytesource;
        SearchAbstract * searchDelegate;
        GuiHelper *guiHelper;
        FloatingDialog *advancedSearchDialog;
        quint64 lastJumpStart;
};

#endif // SEARCHWIDGET_H
