/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

#include <QWidget>
#include "byteitemmodel.h"
#include "rendertextview.h"
#include "guihelper.h"
#include "loggerwidget.h"

namespace Ui {
class TextWidget;
}

class TextWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit TextWidget(QWidget *parent = 0);
        ~TextWidget();
        void setModel(ByteItemModel *dataModel);
        QByteArray getSelection();
        bool hasSelection();
        void selectAll();
        void reset();
    public slots:
        void dropEvent(QDropEvent *event);
        void dragEnterEvent ( QDragEnterEvent * event );
        void keyPressEvent(QKeyEvent *event);
    signals:
        void invalidText();
        void textSelectionChanged();
        void sendDropEvent(QDropEvent *event);
    protected:
        bool eventFilter(QObject *obj, QEvent *event);
    private slots:
        void onTextChanged();
        void onSelectionChanged();
        void updateText(ByteItemModel::UpdateSource source);
        void textRenderingStarted();
        void textRenderingFinished();
        void reveceivingTextChunk(const QString &chunk);
        void updateStats();
        
    private:
        static int MAX_TEXT_VIEW;
        Ui::TextWidget *ui;
        ByteItemModel *currentModel;
        RenderTextView *renderThread;
};

#endif // TEXTWIDGET_H
