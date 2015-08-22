/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include <qtest.h>

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
    QCOMPARE(KMime::encodeRFC2047String(QLatin1String("bla"), "utf-8"), QByteArray("bla"));
    // utf-8
    // expected value is probably wrong, libkmime will chose 'B' instead of 'Q' encoding
    QEXPECT_FAIL("", "libkmime will chose 'B' instead of 'Q' encoding", Continue);
    QCOMPARE(KMime::encodeRFC2047String(QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"), "utf-8").constData(),
             "=?utf-8?q?Ingo=20Kl=C3=B6cker?= <kloecker@kde.org>");

    // Fallback to UTF-8 for encoding since the given charset can't encode the string
    const QString input = QString::fromUtf8("æſðđŋħł");
    const QByteArray result = KMime::encodeRFC2047String(input, "latin1");
    QCOMPARE(KCodecs::decodeRFC2047String(QString::fromUtf8(result)), input);
    QVERIFY(result.contains("utf-8"));
}
