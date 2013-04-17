/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NEWBYTEDIALOG_H
#define NEWBYTEDIALOG_H

#include <QDialog>

namespace Ui {
class NewByteDialog;
}

class NewByteDialog : public QDialog
{
        Q_OBJECT
        
    public:
        explicit NewByteDialog(QWidget *parent = 0, bool isSimple = false);
        ~NewByteDialog();
        int byteCount();
        char getChar();
        void closeEvent(QCloseEvent *event);
        
    private:
        Ui::NewByteDialog *ui;
};

#endif // NEWBYTEDIALOG_H
