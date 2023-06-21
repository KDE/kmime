/*
    SPDX-FileCopyrightText: 2015 Albert Astals Cid <aacid@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parsedatetimetest.h"

#include <QTest>

#include <kmime_header_parsing.h>

using namespace KMime;

QTEST_MAIN(ParseDateTimeTest)

void ParseDateTimeTest::testParseDateTime_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QDateTime>("expResult");

    QTest::newRow("1")
        << QByteArray("Sat, 25 Apr 2015 12:10:48 +0000")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-25T12:10:48+00:00"), Qt::ISODate));
    QTest::newRow("2")
        << QByteArray("Fri, 24 Apr 2015 10:22:42 +0200")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-24T10:22:42+02:00"), Qt::ISODate));
    QTest::newRow("3")
        << QByteArray("Thu, 23 Apr 2015 21:43:32 -0300")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-23T21:43:32-03:00"), Qt::ISODate));
    QTest::newRow("4")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +0200")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-24T10:39:15+02:00"), Qt::ISODate));
    QTest::newRow("5")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +02:00")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-24T10:39:15+02:00"), Qt::ISODate));
    QTest::newRow("6")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +02:23")
        << QDateTime(QDateTime::fromString(QStringLiteral("2015-04-24T10:39:15+02:23"), Qt::ISODate));
    QTest::newRow("7")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +02a")
        << QDateTime();
    QTest::newRow("8")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +02:")
        << QDateTime();
    QTest::newRow("9")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +02:af")
        << QDateTime();
    QTest::newRow("10")
        << QByteArray("Fri 24 Apr 2015 10:39:15 +in:af")
        << QDateTime();
}

void ParseDateTimeTest::testParseDateTime()
{
    QFETCH(QByteArray, input);
    QFETCH(QDateTime, expResult);

    QDateTime result;
    const char *scursor = input.constData();
    const char *send = input.constData() + input.length();

    const bool success = KMime::HeaderParsing::parseDateTime(scursor, send, result, false);
    QCOMPARE(success, !result.isNull());
    QCOMPARE(result, expResult);
}


#include "moc_parsedatetimetest.cpp"
