/*
  SPDX-FileCopyrightText: 2015 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "types.h"

#include <QTest>
#include <QObject>

using namespace Qt::Literals::StringLiterals;
using namespace KMime;

class TypesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testListParsing()
    {
        auto list = Types::Mailbox::listFrom7BitString("");
        QCOMPARE(list.size(), 0);
        list = Types::Mailbox::listFromUnicodeString(QString());
        QCOMPARE(list.size(), 0);

        list = Types::Mailbox::listFrom7BitString("Name <name@example.local>, Name 2 <name2@example.local>");
        QCOMPARE(list.size(), 2);
        QCOMPARE(list.at(0).name(), QStringLiteral("Name"));
        QCOMPARE(list.at(1).address(), QByteArray("name2@example.local"));

        list = Types::Mailbox::listFromUnicodeString(QStringLiteral("Name <name@example.local>, Name 2 <name2@example.local>"));
        QCOMPARE(list.size(), 2);
        QCOMPARE(list.at(0).name(), QStringLiteral("Name"));
        QCOMPARE(list.at(1).address(), QByteArray("name2@example.local"));
    }

    void testEaiPrettyAddress()
    {
        // prettyAddress() must not Latin-1-decode UTF-8 address bytes.
        Types::Mailbox mb;
        mb.fromUnicodeString(u"Grå katt <grå@grå.org>"_s);
        QCOMPARE(mb.prettyAddress(), u"Grå katt <grå@grå.org>"_s);

        Types::Mailbox mb2;
        mb2.fromUnicodeString(u"grå@example.com"_s);
        QCOMPARE(mb2.prettyAddress(), u"grå@example.com"_s);
    }

    void testEaiFromUnicodeString()
    {
        // Bare EAI addr-spec with non-ASCII localpart: must NOT be
        // RFC2047-encoded.
        Types::Mailbox mb;
        mb.fromUnicodeString(u"grå@example.com"_s);
        QCOMPARE(mb.address(), QByteArray("gr\xC3\xA5@example.com"));
        QCOMPARE(mb.as7BitString(QByteArray("utf-8")),
                 QByteArray("gr\xC3\xA5@example.com"));

        // EAI addr-spec with display name: display name may be RFC2047-encoded,
        // addr-spec must be literal UTF-8.
        Types::Mailbox mb2;
        mb2.fromUnicodeString(u"Grå katt <grå@example.com>"_s);
        QCOMPARE(mb2.name(), u"Grå katt"_s);
        QCOMPARE(mb2.address(), QByteArray("gr\xC3\xA5@example.com"));
        const QByteArray s2 = mb2.as7BitString(QByteArray("utf-8"));
        QVERIFY2(s2.contains("gr\xC3\xA5@example.com"), s2.constData());
        // ... although the display name can also be UTF8... maybe later.
    }

    void testListToString()
    {
        QList<Types::Mailbox> mboxes;
        QCOMPARE(Types::Mailbox::listToUnicodeString(mboxes), QString());

        Types::Mailbox mbox;
        mbox.setAddress("name@example.local");
        mboxes.push_back(mbox);
        QCOMPARE(Types::Mailbox::listToUnicodeString(mboxes), QStringLiteral("name@example.local"));

        mbox.setName(QStringLiteral("First Last"));
        mboxes.push_back(mbox);
        QCOMPARE(Types::Mailbox::listToUnicodeString(mboxes), QStringLiteral("name@example.local, First Last <name@example.local>"));
    }
};

QTEST_MAIN(TypesTest)

#include "typestest.moc"
