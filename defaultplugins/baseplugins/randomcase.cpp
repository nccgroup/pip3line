/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include "randomcase.h"
#include "confgui/randomcasewidget.h"

RandomCase::RandomCase()
{
}

const QString RandomCase::id = "Random Case";

QString RandomCase::name() const {
    return id;
}

QString RandomCase::description() const {
    return tr("Randomize the characters cases");
}

bool RandomCase::isTwoWays() {
    return false;
}

QWidget *RandomCase::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) RandomCaseWidget(this, parent);
    if (widget == NULL) {
        qFatal("Cannot allocate memory for RandomCaseWidget X{");
    }
    return widget;
}

void RandomCase::reRandomize()
{
    emit confUpdated();
}

void RandomCase::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    QByteArray temp;
    for (int i = 0; i < input.size(); i++) {
        temp.append(input.at(i));
        output.append((qrand() % 2 == 0) ? temp.toUpper() : temp.toLower());
        temp.clear();
    }
}

QString RandomCase::help() const
{
    QString help;
    help.append("<p>Randomize the characters case of the input (UTF-8)</p><p>i.e. rAndomIze thE cHaRActErS cAsE oF thE Input (UTf-8)</p>");
    return help;
}
