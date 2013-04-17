/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TRANSFORMABSTRACT_H
#define TRANSFORMABSTRACT_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QHash>
#include "commonstrings.h"
#include "libtransform_global.h"
using namespace Pip3lineConst;

class LIBTRANSFORMSHARED_EXPORT TransformAbstract : public QObject
{
    Q_OBJECT
    public:
        // enumeration to define the direction of the transformation
        // note: not all transformations are bi-directional
        enum Way {INBOUND, OUTBOUND};
        TransformAbstract();
        virtual ~TransformAbstract();
        // mandatory methods to implement
        virtual QString name() const = 0;
        virtual QString description() const = 0;
        virtual void transform(const QByteArray &input, QByteArray &output) = 0;

        // optional configuration methods
        virtual QHash<QString, QString> getConfiguration();
        virtual bool setConfiguration(QHash<QString, QString> propertiesList);

        // methods only used for gui coolness (optionals)
        virtual bool isTwoWays();
        QWidget * getGui(QWidget * parent);
        virtual QString inboundString() const;
        virtual QString outboundString() const;
        // Optional Help string. Will be accessible from the Gui
        virtual QString help() const;
        // Optional Credits. Will be accessible from the Gui
        virtual QString credits() const;
        // Base functions setting the current direction of the transformation process
        void setWay(Way nway);
        TransformAbstract::Way way();
        // Utilities functions used for conversion
        QByteArray fromHex(QByteArray in);
        static QByteArray fromEscapedHex(const QByteArray &val);
        static QByteArray fromCStyleArray(const QByteArray &val);
        static QByteArray toEscapedHex(const QByteArray &val);
        static QByteArray toCStyleArray(const QByteArray &val);
        static QByteArray toCSV(const QByteArray &val);
        static QByteArray toPrintableString(const QByteArray &val);
        // Convenience function which just calls "void transform(const QByteArray &input, QByteArray &output)"
        QByteArray transform(const QByteArray &input);
    public Q_SLOTS:
        // Convenience functions used to manage errors and warnings internally
        virtual void logError(const QString message, const QString source);
        virtual void logWarning(const QString message, const QString source);

    Q_SIGNALS:
        void confUpdated(); // to be emitted when the internal configuration has been modified, so that the gui get notified
        void error(const QString, const QString); // signal for an error message (message, source)
        void warning(const QString, const QString); // signal for warning message (message, source)
        void resetMessages(); // for gui purposes, tell the default gui to clean all previous error/warning messages

    protected:
        virtual QWidget * requestGui(QWidget * parent);
        Way wayValue; // store the "direction" of the Transformation when needed

    private Q_SLOTS:
        void onGuiDelete();
    private:
        static const QByteArray HEXCHAR;
        QWidget *confGui; // this is the configuration widget, if needed. if the pointer is null, the gui will just ignore the setting tab.
};

#endif // TRANSFORMABSTRACT_H
