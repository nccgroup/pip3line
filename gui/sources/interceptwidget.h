#ifndef INTERCEPTWIDGET_H
#define INTERCEPTWIDGET_H

#include <QWidget>
#include <QStringList>
#include <QFont>
#include <QModelIndex>
#include <QBitArray>

namespace Ui {
class InterceptWidget;
}

class IntercepSource;
class BlocksSource;
class PayloadModel;
class QTableView;

class InterceptWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit InterceptWidget(IntercepSource * source, QWidget *parent = 0);
        ~InterceptWidget();
    private slots:
        void onSourceChanged(QString selected);
        void onCurrentGuiDeleted();
        void updateColumns();
        void onCurrentSelectedChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
        void onColumnChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    private:
        static const QString CHOOSE_TEXT;
        static const QString UDP_EXTERNAL_SOURCE_TEXT;
        static const QString TCP_EXTERNAL_SOURCE_TEXT;
        static const QFont RegularFont;
        static const int TIMESTAMP_COLUMN_WIDTH;
        Ui::InterceptWidget *ui;
        IntercepSource *source;
        QStringList sourceChoices;
        BlocksSource *currentBlockSource;
        QWidget * currentGui;
        PayloadModel *model;
        QTableView * packetsTable;
};

#endif // INTERCEPTWIDGET_H
