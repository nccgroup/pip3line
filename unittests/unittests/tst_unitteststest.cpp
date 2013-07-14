/**
Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Gabriel Caudrelier, gabriel dot caudrelier at nccgroup dot com

https://github.com/nccgroup/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QString>
#include <QtTest>
#include <transformmgmt.h>
#include <QTextStream>
#include <QtCore/QCoreApplication>
#include <QHash>

class UnittestsTest : public QObject
{
        Q_OBJECT
        
    public:
        UnittestsTest();
        
    private slots:
        void logError(const QString mess);
        void logStatus(const QString mess);
    private Q_SLOTS:
        void initTestCase();
        void cleanupTestCase();
        void testBase32();
        void testBase64();
        void testBase64UrlEncode();
        void testBinary();
        void testCharEncoding();
        void testCisco7();
        void testCut();
        void testHexencode();
        void testHieroglyphy();
        void testBytesToInteger();

        void randomFuzzing(TransformAbstract * tr, QString text = QString());
        QByteArray randomByteArray(int length = -1);
    private:
        void printConf(QHash<QString, QString> configuration);
        QTextStream *messlog;
        TransformMgmt *transformFactory;

};

UnittestsTest::UnittestsTest()
{
    messlog = new QTextStream(stdout);
    transformFactory = NULL;
}

void UnittestsTest::logError(const QString mess)
{
    *messlog << "Error: " <<  mess << endl;
}

void UnittestsTest::logStatus(const QString mess)
{
    *messlog << "Status: " <<  mess << endl;
}

void UnittestsTest::initTestCase()
{
    transformFactory = new TransformMgmt();
    connect(transformFactory, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transformFactory, SIGNAL(status(QString,QString)), this, SLOT(logStatus(QString)));
    transformFactory->initialize(qApp->applicationDirPath());

}

void UnittestsTest::cleanupTestCase()
{
    delete transformFactory;
}

void UnittestsTest::testBase32()
{
    QString name = "Base32";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    randomFuzzing(transf, "default");

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("un test 1234")), QByteArray("OVXCA5DFON2CAMJSGM2A===="));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("ME======"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));



    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 5);
    QCOMPARE(configuration.value(PROP_NAME) , name);

    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(configuration.value(XMLVARIANT) , QString::number(0));
    QCOMPARE(configuration.value(XMLPADDINGCHAR) , QString("3d"));
    QCOMPARE(configuration.value(XMLINCLUDEPADDING) , QString::number(true));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLVARIANT,QString::number(1));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("un test 1234")), QByteArray("ENQ20X35EDT20C9J6CT0===="));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("C4======"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLVARIANT,QString::number(2));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("un test 1234")), QByteArray("ELN20T35EDQ20C9I6CQ0===="));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("C4======"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(0));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("OVXCA5DFON2CAMJSGM2A====")), QByteArray("un test 1234"));
    QCOMPARE(transf->transform(QByteArray("ME======")),QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 0");

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(1));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("ENQ20X35EDT20C9J6CT0====")), QByteArray("un test 1234"));
    QCOMPARE(transf->transform(QByteArray("C4======")),QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 1");

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(2));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("ELN20T35EDQ20C9I6CQ0====")), QByteArray("un test 1234"));
    QCOMPARE(transf->transform(QByteArray("C4======")),QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 2");

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(1));
    configuration.insert(XMLPADDINGCHAR, "2d");
    configuration.insert(XMLINCLUDEPADDING, QString::number(false));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 5);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));
    QCOMPARE(configuration.value(XMLVARIANT) , QString::number(1));
    QCOMPARE(configuration.value(XMLPADDINGCHAR) , QString("2d"));
    QCOMPARE(configuration.value(XMLINCLUDEPADDING) , QString::number(false));
    QCOMPARE(transf->transform(QByteArray("ENQ20X35EDT20C9J6CT0")) , QByteArray("un test 1234"));
    QCOMPARE(transf->transform(QByteArray("c4")) , QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")) , QByteArray(""));

    failconfiguration = configuration;
    failconfiguration.remove(XMLVARIANT);
    failconfiguration.remove(XMLPADDINGCHAR);
    failconfiguration.remove(XMLINCLUDEPADDING);

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLVARIANT,QString::number(3));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLVARIANT,QString::number(-1));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLPADDINGCHAR, "zzzz");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLINCLUDEPADDING,"zzzz");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLINCLUDEPADDING,"2W2vds");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    delete transf;
}

void UnittestsTest::testBase64()
{
    QString name = "Base64";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    randomFuzzing(transf, "default");

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("YQ=="));
    QCOMPARE(transf->transform(QByteArray(QString("un test 132*$<>?>?<").toUtf8())), QByteArray("dW4gdGVzdCAxMzIqJDw+Pz4/PA=="));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(configuration.value(XMLVARIANT) , QString::number(0));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLVARIANT,QString::number(1));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(transf->transform(QByteArray(QString("un test 132*$<>?>?<").toUtf8())), QByteArray("dW4gdGVzdCAxMzIqJDw-Pz4_PA"));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("YQ"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 1");

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLVARIANT,QString::number(2));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(transf->transform(QByteArray(QString("un test 132*$<>?>?<").toUtf8())), QByteArray("dW4gdGVzdCAxMzIqJDw-Pz4_PA2"));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("YQ2"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 2");

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(0));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(transf->transform(QByteArray("dW4gdGVzdCAxMzIqJDw+Pz4/PA==")), QByteArray(QString("un test 132*$<>?>?<").toUtf8()));
    QCOMPARE(transf->transform(QByteArray("YQ")), QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(1));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(transf->transform(QByteArray("dW4gdGVzdCAxMzIqJDw-Pz4_PA")), QByteArray(QString("un test 132*$<>?>?<").toUtf8()));
    QCOMPARE(transf->transform(QByteArray("YQ")), QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(2));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(transf->transform(QByteArray("dW4gdGVzdCAxMzIqJDw-Pz4_PA2")), QByteArray(QString("un test 132*$<>?>?<").toUtf8()));
    QCOMPARE(transf->transform(QByteArray("YQ2")), QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLVARIANT,QString::number(3));
    configuration.insert(XMLPADDINGCHAR, QString("25"));
    configuration.insert(XMLCHAR62, QString("24"));
    configuration.insert(XMLCHAR63, QString("21"));
    configuration.insert(XMLPADDINGTYPE, QString::number(0));



    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size(),7);
    QCOMPARE(configuration.value(PROP_NAME), name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));
    QCOMPARE(configuration.value(XMLVARIANT), QString::number(3));
    QCOMPARE(configuration.value(XMLPADDINGCHAR), QString("25"));
    QCOMPARE(configuration.value(XMLCHAR62), QString("24"));
    QCOMPARE(configuration.value(XMLCHAR63), QString("21"));
    QCOMPARE(configuration.value(XMLPADDINGTYPE), QString::number(0));
    QCOMPARE(transf->transform(QByteArray("dW4gdGVzdCAxMzIqJDw$Pz4!PA%%")), QByteArray("un test 132*$<>?>?<"));
    QCOMPARE(transf->transform(QByteArray("YQ--")), QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    randomFuzzing(transf, "Variant 3");

    failconfiguration = configuration;
    failconfiguration.remove(XMLVARIANT);

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLVARIANT,QString::number(-1));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLVARIANT,QString::number(4));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLPADDINGCHAR,"");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLPADDINGCHAR,"QStr");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLCHAR62,"");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLCHAR62,"ZXZXZX");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLCHAR63,"");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLCHAR63,"ZXZX");

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLPADDINGTYPE,QString::number(-1));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLPADDINGTYPE,QString::number(100));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    delete transf;
}

void UnittestsTest::testBase64UrlEncode()
{
    QString name = "Base64 && Url Encode";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("un test 123s4")), QByteArray("dW4gdGVzdCAxMjNzNA%3D%3D"));
    QCOMPARE(transf->transform(QByteArray("a")), QByteArray("YQ%3D%3D"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 2);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    QVERIFY(transf->setConfiguration(configuration));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size(),2);
    QCOMPARE(configuration.value(PROP_NAME), name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));
    QCOMPARE(transf->transform(QByteArray("dW4gdGVzdCAxMjNzNA%3D%3D")), QByteArray("un test 123s4"));
    QCOMPARE(transf->transform(QByteArray("YQ%3D%3D")), QByteArray("a"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);

    QVERIFY(!transf->setConfiguration(failconfiguration));

}

void UnittestsTest::testBinary()
{
    QString name = "Binary encoding";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("un test 123s4")), QByteArray("01110101011011100010000001110100011001010111001101110100001000000011000100110010001100110111001100110100"));
    QCOMPARE(transf->transform(QByteArray("b")), QByteArray("01100010"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(configuration.value(XMLBLOCKSIZE), QString::number(0));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLBLOCKSIZE,QString::number(8));
    QVERIFY(transf->setConfiguration(configuration));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size(),3);
    QCOMPARE(configuration.value(PROP_NAME), name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));
    QCOMPARE(transf->transform(QByteArray("01100010 01101000 01101001 01101111 01100111 01110010 01100101 01101111 01101001 01110010 01110110 01100101 ")), QByteArray("bhiogreoirve"));
    QCOMPARE(transf->transform(QByteArray("01100011")), QByteArray("c"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLBLOCKSIZE);

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLBLOCKSIZE,QString::number(-1));

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLBLOCKSIZE,QString::number(1025));

    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testCharEncoding()
{
    QString name = "Char encoding";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("un test 123s4")), QByteArray("un test 123s4"));
    QCOMPARE(transf->transform(QByteArray("\xc2\xa3")), QByteArray("\x3f"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(configuration.value(XMLCODECNAME), QString("ASMO-708"));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLCODECNAME,QString("UTF-16"));
    QVERIFY(transf->setConfiguration(configuration));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size(),3);
    QCOMPARE(configuration.value(PROP_NAME), name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));
    QCOMPARE(transf->transform(QByteArray("\xc2\xa3")), QByteArray("\xea\x8f\x82"));
    QCOMPARE(transf->transform(QByteArray("")), QByteArray(""));
    QCOMPARE(configuration.value(XMLCODECNAME), QString("UTF-16"));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);

    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLCODECNAME);

    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testCisco7()
{
    QString name = "Cisco secret 7";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::OUTBOUND);
    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("Cisco"));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 2);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);

    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testCut()
{
    QString name = "Cut";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("0"));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 5);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLFROM,QString("2"));
    configuration.insert(XMLLENGTH,QString("3"));
    QVERIFY(transf->setConfiguration(configuration));

    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("250"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLEVERYTHING,QString("1"));
    QVERIFY(transf->setConfiguration(configuration));

    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("2506324F41"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLFROM);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLLENGTH);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLEVERYTHING);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLFROM,QString::number(-1));
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLLENGTH,QString::number(0));
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLEVERYTHING,QString::number(-1));
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.insert(XMLEVERYTHING,QString::number(2));
    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testHexencode()
{
    QString name = "Hexadecimal";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("303632353036333234463431"));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLTYPE,QString("0"));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::OUTBOUND));
    configuration.insert(XMLTYPE,QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("\\x30\\x36\\x32\\x35\\x30\\x36\\x33\\x32\\x34\\x46\\x34\\x31")), QByteArray("062506324F41"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));

    configuration.insert(XMLTYPE,QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("{ 0x30, 0x36, 0x32, 0x35, 0x30, 0x36, 0x33, 0x32, 0x34, 0x46, 0x34, 0x31 }")), QByteArray("062506324F41"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));

    configuration.insert(XMLTYPE,QString("3"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("30,36,32,35,30,36,33,32,34,46,34,31,")), QByteArray("062506324F41"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::OUTBOUND));

    configuration.insert(PROP_WAY,QString::number((int)TransformAbstract::INBOUND));
    configuration.insert(XMLTYPE,QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("\\x30\\x36\\x32\\x35\\x30\\x36\\x33\\x32\\x34\\x46\\x34\\x31"));

    configuration.insert(XMLTYPE,QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(transf->transform(QByteArray("062506324F41")),QByteArray("{ 0x30, 0x36, 0x32, 0x35, 0x30, 0x36, 0x33, 0x32, 0x34, 0x46, 0x34, 0x31 }"));
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));

    configuration.insert(XMLTYPE,QString("3"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(transf->transform(QByteArray("062506324F41")), QByteArray("30,36,32,35,30,36,33,32,34,46,34,31,"));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove(XMLTYPE);
    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testHieroglyphy()
{
    QString name = "Hieroglyphy";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("alert(1);fdsTH")), QByteArray("[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((+{}+[])[+!![]]+(![]+[])[!+[]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+[])+(!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+[]))+(+!![]+[])+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+[])+(!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+[]))+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+!![]+[])+([]+{})[!+[]+!![]])+([][[]]+[])[!+[]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]]+(![]+[])[!+[]+!![]+!![]]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([]+{})[!+[]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(+{}+[])[+!![]])()(+{}+[])[+[]]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([]+{})[!+[]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(+{}+[])[+!![]])()(!![]+[])[+!![]])()"));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(configuration.value("btoaInUse"), QString("1"));

    configuration.insert("btoaInUse",QString("0"));
    configuration.insert(XMLTYPE,QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 3);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value(PROP_WAY), QString::number((int)TransformAbstract::INBOUND));
    QCOMPARE(transf->transform(QByteArray("alert(1);fdsTH")), QByteArray("[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((+{}+[])[+!![]]+(![]+[])[!+[]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+[])+(!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+[]))+(+!![]+[])+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+[])+(!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+[]))+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+!![]+[])+([]+{})[!+[]+!![]])+([][[]]+[])[!+[]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]]+(![]+[])[!+[]+!![]+!![]]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+!![]+!![]+!![]+[])+(!+[]+!![]+!![]+!![]+[]))+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[+[]]+([][[]]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()([][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(![]+[])[!+[]+!![]+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+([]+[][(![]+[])[!+[]+!![]+!![]]+([]+{})[+!![]]+(!![]+[])[+!![]]+(!![]+[])[+[]]][([]+{})[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]]+(![]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+[]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(!![]+[])[+[]]+([]+{})[+!![]]+(!![]+[])[+!![]]]((!![]+[])[+!![]]+([][[]]+[])[!+[]+!![]+!![]]+(!![]+[])[+[]]+([][[]]+[])[+[]]+(!![]+[])[+!![]]+([][[]]+[])[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]+!![]+!![]]+(![]+[])[!+[]+!![]]+([]+{})[+!![]]+([]+{})[!+[]+!![]+!![]+!![]+!![]]+(+{}+[])[+!![]]+(!![]+[])[+[]]+([][[]]+[])[!+[]+!![]+!![]+!![]+!![]]+([]+{})[+!![]]+([][[]]+[])[+!![]])())[!+[]+!![]+!![]]+([][[]]+[])[!+[]+!![]+!![]])()(([]+{})[+[]])[+[]]+(!+[]+!![]+!![]+!![]+[])+(!+[]+!![]+!![]+!![]+!![]+!![]+!![]+!![]+[])))()"));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove("btoaInUse");
    QVERIFY(!transf->setConfiguration(failconfiguration));
}

void UnittestsTest::testBytesToInteger()
{
    QString name = "Byte(s) to Integer";
    QHash<QString, QString> configuration;
    QHash<QString, QString> failconfiguration;

    TransformAbstract *transf = transformFactory->getTransform(name);
    if (transf == 0)
        QSKIP("Cannot find the transformation, skipping test",SkipSingle);

    connect(transf, SIGNAL(error(QString, QString)), this, SLOT(logError(QString)));
    connect(transf, SIGNAL(warning(QString,QString)), this, SLOT(logError(QString)));

    transf->setWay(TransformAbstract::INBOUND);
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("875770545"));

    configuration = transf->getConfiguration();
    QCOMPARE(configuration.size() , 5);
    QCOMPARE(configuration.value(PROP_NAME) , name);
    QCOMPARE(configuration.value("Endianness") ,  QString::number(1));
    QCOMPARE(configuration.value("IntegerSize") ,  QString::number(4));
    QCOMPARE(configuration.value("SignedInteger") ,  QString::number(1));

    configuration.insert("SignedInteger",QString("1"));
    configuration.insert("Endianness",QString("1"));
    configuration.insert("IntegerSize",QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("-79"));
    configuration.insert("IntegerSize",QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("12977"));
    configuration.insert("IntegerSize",QString("4"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("875770545"));
    configuration.insert("IntegerSize",QString("8"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("875770545"));

    configuration.insert("Endianness",QString("0"));
    configuration.insert("IntegerSize",QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("-79"));
    configuration.insert("IntegerSize",QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("-20174"));
    configuration.insert("IntegerSize",QString("4"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("-1322110156"));
    configuration.insert("IntegerSize",QString("8"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("-5678419881729458176"));

    configuration.insert("SignedInteger",QString("0"));
    configuration.insert("Endianness",QString("1"));
    configuration.insert("IntegerSize",QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("177"));
    configuration.insert("IntegerSize",QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("12977"));
    configuration.insert("IntegerSize",QString("4"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("875770545"));
    configuration.insert("IntegerSize",QString("8"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("875770545"));

    configuration.insert("Endianness",QString("0"));
    configuration.insert("IntegerSize",QString("1"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("177"));
    configuration.insert("IntegerSize",QString("2"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("45362"));
    configuration.insert("IntegerSize",QString("4"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("2972857140"));
    configuration.insert("IntegerSize",QString("8"));
    QVERIFY(transf->setConfiguration(configuration));
    QCOMPARE(transf->transform(QByteArray("\xb1\x32\x33\x34")),QByteArray("12768324191980093440"));

    failconfiguration = configuration;
    failconfiguration.remove(PROP_WAY);
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove("SignedInteger");
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove("Endianness");
    QVERIFY(!transf->setConfiguration(failconfiguration));

    failconfiguration = configuration;
    failconfiguration.remove("IntegerSize");
    QVERIFY(!transf->setConfiguration(failconfiguration));

}

void UnittestsTest::randomFuzzing(TransformAbstract *transf, QString text)
{
    for (int i = 0; i < 100 ;i++ ) {
        transf->setWay(TransformAbstract::INBOUND);

        QByteArray val1 = randomByteArray();
        QByteArray val2;
        QByteArray val3;
        transf->transform(val1,val2);

        transf->setWay(TransformAbstract::OUTBOUND);

        transf->transform(val2,val3);
        QVERIFY2( (val1 == val3) , QString("[%1] Failed fuzzing %2 <> %3").arg(text).arg(QString::fromUtf8(val1.toHex())).arg(QString::fromUtf8(val3.toHex())).toUtf8());
    }
}

QByteArray UnittestsTest::randomByteArray(int length)
{
    QByteArray ret;
    qsrand(QDateTime::currentDateTime().toTime_t());

    if (length == -1) {
        int val = (qrand() % 10);
        length = ( val == 0 ? 1 : val ) * 10 ;
    }

    for (int i = 0; i < length; i++) {
        ret.append(QByteArray::number(qrand() % 256, 16));
    }

    ret = QByteArray::fromHex(ret);

    return ret;
}

void UnittestsTest::printConf(QHash<QString, QString> configuration)
{
    QHashIterator<QString, QString> i(configuration);
     while (i.hasNext()) {
         i.next();
         *messlog << i.key() << ": " << i.value() << endl;
     }
}

QTEST_MAIN(UnittestsTest)

#include "tst_unitteststest.moc"
