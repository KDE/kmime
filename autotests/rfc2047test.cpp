/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "rfc2047test.h"

#include <KCodecs>
#include <kmime_codecs.cpp>
using namespace KMime;

QTEST_MAIN(RFC2047Test)

void RFC2047Test::testRFC2047encode()
{
    // empty
    QCOMPARE(KMime::encodeRFC2047String(QString(), "utf-8"), QByteArray());
    // identity
    QCOMPARE(KMime::encodeRFC2047String(u"bla", "utf-8"), QByteArray("bla"));
    // utf-8
    // expected value is probably wrong, libkmime will chose 'B' instead of 'Q' encoding
    QEXPECT_FAIL("", "libkmime will chose 'B' instead of 'Q' encoding", Continue);
    QCOMPARE(KMime::encodeRFC2047String(QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"), "utf-8").constData(),
             "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>");

    // Fallback to UTF-8 for encoding since the given charset can't encode the string
    const QString input = QStringLiteral("æſðđŋħł");
    const QByteArray result = KMime::encodeRFC2047String(input, "latin1");
    QCOMPARE(KCodecs::decodeRFC2047String(QString::fromUtf8(result)), input);
    QVERIFY(result.contains("utf-8"));
}

#include "moc_rfc2047test.cpp"
