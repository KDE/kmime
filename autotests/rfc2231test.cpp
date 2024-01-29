/*
    SPDX-FileCopyrightText: 2010 Leo Franchi <lfranchi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QTest>

#include "rfc2231test.h"

#include "kmime_util.h"
#include <kmime_codecs.cpp>
#include <QDebug>
using namespace KMime;

QTEST_MAIN(RFC2231Test)

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
    QCOMPARE(KMime::encodeRFC2047String(u"bla", "utf-8"), QByteArray("bla"));
    QCOMPARE(KMime::encodeRFC2231String(QString::fromUtf8("with accents Ã²Ã³Ã¨Ã©Ã¤Ã¯Ã±"), "utf-8").constData(),
             "utf-8''with%20accents%20%C3%83%C2%B2%C3%83%C2%B3%C3%83%C2%A8%C3%83%C2%A9%C3%83%C2%A4%C3%83%C2%AF%C3%83%C2%B1");
}

#include "moc_rfc2231test.cpp"
