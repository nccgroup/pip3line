#ifndef MEMORYWIDGET_H
#define MEMORYWIDGET_H

#include <QWidget>
#include <QSize>
#include <QModelIndex>

namespace Ui {
    class MemoryWidget;
}

class CurrentMemorysource;
class QMenu;
class QAction;

class MemoryWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit MemoryWidget(CurrentMemorysource *source, QWidget *parent = 0);
        ~MemoryWidget();
        void setProcSelection(bool val);
    private slots:
        void onDoubleClick(QModelIndex index);
        void onRefreshToggled(bool val);
        void onRightClick(QPoint pos);
        void contextMenuAction(QAction * action);
        void onSearch(QByteArray item,bool maybetext);
    private:
        static const QString GOTOSTART;
        static const QString GOTOEND;
        void initContextMenu();
        Ui::MemoryWidget *ui;
        CurrentMemorysource * msource;
        QMenu * contextMenu;
};

#endif // MEMORYWIDGET_H
