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

class GuiHelper;
class ByteSourceAbstract;
class QFocusEvent;
class QPushButton;
class QEvent;
class SearchAbstract;

class SearchLine : public QLineEdit
{
        Q_OBJECT
    public:
        explicit SearchLine(ByteSourceAbstract *source, QWidget *parent = 0);
        ~SearchLine();
    public slots:
        void setError(bool val);
        void updateProgress(double val);
        void onSearchStarted();
        void onSearchEnded();
    private slots:
        void onSourceUpdated(quintptr);
    signals:
        void newSearch(QString val, int modifiers);
    private:
        void focusInEvent(QFocusEvent * event);
        void paintEvent(QPaintEvent * event);
        void keyPressEvent(QKeyEvent * event);
        double progress;
        quint64 sourceSize;
        static const QString FIND_PLACEHOLDER_TEXT;
        static const QString TOOLTIP_TEXT;
        static const QString PLACEHOLDER_DISABLED_TEXT;
        static const QColor LOADING_COLOR;
        SearchAbstract *sObject;
};

class SearchWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit SearchWidget(ByteSourceAbstract *source, QWidget *parent = 0);
        ~SearchWidget();
        QString text();
        void setStopVisible(bool val);
    public slots:
        void setError(bool val);
        void onSearchStarted();
        void onSearchEnded();
        void updateStatusProgress(double val);
        void clearSearch();

    private slots:
        void onSearch(QString val, int modifiers);

    signals:
        void searchRequest(QByteArray data,QBitArray mask, bool couldBeText);
        void stopSearch();
    private:
        SearchLine * lineEdit;
        QPushButton *stopPushButton;
};

#endif // SEARCHWIDGET_H
