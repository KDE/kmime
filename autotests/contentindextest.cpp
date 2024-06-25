/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contentindextest.h"

#include "content.h"
#include "contentindex.h"

using namespace KMime;

#include <QTest>

QTEST_MAIN(ContentIndexTest)

void ContentIndexTest::testToString()
{
    KMime::ContentIndex ci;
    QCOMPARE(ci.toString(), QString());
    ci.push(1);
    QCOMPARE(ci.toString(), QLatin1StringView("1"));
    ci.push(2);
    QCOMPARE(ci.toString(), QLatin1StringView("2.1"));
}

void ContentIndexTest::testFromString()
{
    ContentIndex ci1;
    QVERIFY(!ci1.isValid());

    ContentIndex ci2(QStringLiteral("1.2.bla"));
    QVERIFY(!ci2.isValid());

    ContentIndex ci3(QStringLiteral("1"));
    QVERIFY(ci3.isValid());
    QCOMPARE(ci3.pop(), 1u);
    QVERIFY(!ci3.isValid());

    ContentIndex ci4(QStringLiteral("3.2"));
    QVERIFY(ci4.isValid());
    QCOMPARE(ci4.pop(), 3u);
    QCOMPARE(ci4.pop(), 2u);
    QVERIFY(!ci4.isValid());
}

void ContentIndexTest::testContent()
{
    auto c1 = new Content();
    QCOMPARE(c1->content(ContentIndex()), c1);
    QCOMPARE(c1->content(ContentIndex(u"1")), (Content *)nullptr);
    QCOMPARE(c1->indexForContent(c1), ContentIndex());

    auto c11 = new Content();
    c1->appendContent(c11);
    QCOMPARE(c1->content(ContentIndex(u"1")), c11);
    QCOMPARE(c1->content(ContentIndex(u"3")), (Content *)nullptr);
    QCOMPARE(c1->content(ContentIndex(u"2.1")), (Content *)nullptr);
    QCOMPARE(c1->indexForContent(c1), ContentIndex());
    QCOMPARE(c1->indexForContent(c11), ContentIndex(u"1"));
    QCOMPARE(c1->indexForContent(c1->contents().first()), ContentIndex(u"1"));

    auto c12 = new Content();
    c1->appendContent(c12);
    auto c121 = new Content();
    c12->appendContent(c121);
    QCOMPARE(c1->content(ContentIndex(u"2")), c12);
    QCOMPARE(c1->content(ContentIndex(u"2.1")), c121);
    QCOMPARE(c1->indexForContent(c121), ContentIndex(u"2.1"));
    QCOMPARE(c121->index(), ContentIndex(u"2.1"));

    QCOMPARE(c1->indexForContent((Content *)nullptr), ContentIndex());
    delete c1;
}

void ContentIndexTest::testNavigation()
{
    ContentIndex ci;
    QCOMPARE(ci.toString(), QString());
    ci.push(1);
    ci.push(2);
    ci.push(3);
    QCOMPARE(ci.toString(), QLatin1StringView("3.2.1"));
    QCOMPARE(ci.pop(), 3u);
    QCOMPARE(ci.up(), 1u);
    QCOMPARE(ci.toString(), QLatin1StringView("2"));
}


#include "moc_contentindextest.cpp"
