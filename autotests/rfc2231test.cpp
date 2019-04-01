/*
    Copyright (c) 2010 Leo Franchi <lfranchi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QTest>

#include "rfc2231test.h"

#include <kmime_util.h>
#include <kmime_codecs.cpp>
#include <QDebug>
using namespace KMime;

QTEST_MAIN(RFC2231Test)

void RFC2231Test::testRFC2231decode()
{
    QByteArray encCharset;

    // empty
    QCOMPARE(KMime::decodeRFC2231String(QByteArray(), encCharset, "utf-8", false), QString());
    // identity
    QCOMPARE(KMime::decodeRFC2231String("bla", encCharset, "utf-8", false), QLatin1String("bla"));
    // utf-8
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker <kloecker@kde.org>", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    qDebug() << "Charset:" << encCharset;
    QCOMPARE(KMime::decodeRFC2231String("iso8859-1''Ingo%20Kl%C3%B6cker <kloecker@kde.org>", encCharset, "iso8859-1", false),
             QString::fromUtf8("Ingo KlÃ¶cker <kloecker@kde.org>"));
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker"));
    QCOMPARE(encCharset, QByteArray("UTF-8"));

    // whitespaces between two encoded words
    QCOMPARE(KMime::decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker       Ingo%20Kl%C3%B6cker", encCharset, "utf-8", false),
             QString::fromUtf8("Ingo Klöcker       Ingo Klöcker"));
    QCOMPARE(decodeRFC2231String("utf-8''Ingo%20Kl%C3%B6cker  foo  Ingo%20Kl%C3%B6cker", encCharset),
             QString::fromUtf8("Ingo Klöcker  foo  Ingo Klöcker"));

    // iso-8859-x
    QCOMPARE(KMime::decodeRFC2231String("ISO-8859-1'Andr%E9s Ot%F3n", encCharset, "utf-8", false),
             QString::fromUtf8("Andrés Otón"));
    QCOMPARE(encCharset, QByteArray("ISO-8859-1"));

    QCOMPARE(KMime::decodeRFC2231String( "iso-8859-2''Rafa%B3 Rzepecki", encCharset, "utf-8", false),
             QString::fromUtf8( "Rafał Rzepecki" ));
    QCOMPARE(encCharset, QByteArray( "ISO-8859-2" ));
    QCOMPARE(KMime::decodeRFC2231String( "iso-8859-9''S%2E%C7a%F0lar Onur", encCharset, "utf-8", false),
             QString::fromUtf8( "S.Çağlar Onur" ));
    QCOMPARE(encCharset, QByteArray( "ISO-8859-9" ));
    QCOMPARE(KMime::decodeRFC2231String( "iso-8859-15''Rafael Rodr%EDguez", encCharset, "utf-8", false),
             QString::fromUtf8( "Rafael Rodríguez" ));
    QCOMPARE(encCharset, QByteArray( "ISO-8859-15" ));

    // wrong charset + charset overwrite
    QCOMPARE(KMime::decodeRFC2231String( "iso8859-1''Ingo%20Kl%C3%B6cker", encCharset, "utf-8", true),
             QString::fromUtf8( "Ingo Klöcker" ));

    // Small data
    QCOMPARE(decodeRFC2231String( "iso-8859-1''c", encCharset ), QString::fromUtf8("c") );
}

void RFC2231Test::testInvalidDecode()
{
    /* QByteArray encCharset;

     // invalid / incomplete encoded data
     QCOMPARE( decodeRFC2231String( "=", encCharset ), QString::fromUtf8("=") );
     QCOMPARE( decodeRFC2231String( "=?", encCharset ), QString::fromUtf8("=?") );
     QCOMPARE( decodeRFC2231String( "=?a?b?=", encCharset ), QString::fromUtf8("=?a?b?=") );
     QCOMPARE( decodeRFC2231String( "=?a?b?c?", encCharset ), QString::fromUtf8("=?a?b?c?") );
     QCOMPARE( decodeRFC2231String( "=?a??c?=", encCharset ), QString::fromUtf8("=?a??c?=") ); */
}

void RFC2231Test::testRFC2231encode()
{
    // empty
    QCOMPARE(KMime::encodeRFC2047String(QString(), "utf-8"), QByteArray());
    // identity
    QCOMPARE(KMime::encodeRFC2047String(QLatin1String("bla"), "utf-8"), QByteArray("bla"));
    QCOMPARE(KMime::encodeRFC2231String(QString::fromUtf8("with accents Ã²Ã³Ã¨Ã©Ã¤Ã¯Ã±"), "utf-8").constData(),
             "utf-8''with%20accents%20%C3%83%C2%B2%C3%83%C2%B3%C3%83%C2%A8%C3%83%C2%A9%C3%83%C2%A4%C3%83%C2%AF%C3%83%C2%B1");
}
