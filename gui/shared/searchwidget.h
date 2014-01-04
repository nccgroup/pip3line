#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>
#include <QWidget>

class GuiHelper;
class ByteSourceAbstract;
class QFocusEvent;
class QPushButton;

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
        void updateProgress(quint64 val);
        void clearSearch();
    private slots:
        void onStopClicked();
        void onSearch(QString val, int modifiers);

    signals:
        void searchRequest(QByteArray data, bool couldBeText);
        void stopSearch();
    private:
        SearchLine * lineEdit;
        QPushButton *stopPushButton;
};

#endif // SEARCHWIDGET_H
