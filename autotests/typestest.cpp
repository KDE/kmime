/*
  SPDX-FileCopyrightText: 2015 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_types.h"

#include <QTest>
#include <QObject>

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

    void testListToString()
    {
        QVector<Types::Mailbox> mboxes;
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
