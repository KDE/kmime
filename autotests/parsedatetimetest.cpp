/*
    Copyright (c) 2015 Albert Astals Cid <aacid@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "parsedatetimetest.h"

#include <qtest.h>

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

    KMime::HeaderParsing::parseDateTime(scursor, send, result, false);

    QCOMPARE(result, expResult);
}

