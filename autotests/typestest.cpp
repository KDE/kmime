/*
  Copyright (c) 2015 Volker Krause <vkrause@kde.org>

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

#include "kmime_types.h"

#include <QTest>
#include <QObject>
#include <QDebug>

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
