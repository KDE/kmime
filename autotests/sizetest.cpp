/*
  Copyright (c) 2011 Volker Krause <vkrause@kde.org>

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

#include "kmime_message.h"
#include "kmime_content_p.h"
#include "kmime_headers_p.h"

#include <qtest.h>
#include <QObject>
#include <QDebug>

using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Headers::Generics;

// this is to ensure we don't accidentally increase the size of memory hotspots
// and to help with optimizing memory use of these structures
class SizeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testContent()
    {
        qDebug() << sizeof(Content);
        QVERIFY(sizeof(Content) <= 16);
        qDebug() << sizeof(ContentPrivate);
        QVERIFY(sizeof(ContentPrivate) <= 88);
        qDebug() << sizeof(Message);
        QCOMPARE(sizeof(Message), sizeof(Content));
    }

    void testHeaders()
    {
        qDebug() << sizeof(Headers::Base);
        QVERIFY(sizeof(Base) <= 16);
        QCOMPARE(sizeof(Unstructured), sizeof(Base));
        QCOMPARE(sizeof(Structured), sizeof(Base));
        QCOMPARE(sizeof(Address), sizeof(Base));
        QCOMPARE(sizeof(MailboxList), sizeof(Base));
        QCOMPARE(sizeof(SingleMailbox), sizeof(Base));
        QCOMPARE(sizeof(AddressList), sizeof(Base));
        QCOMPARE(sizeof(Ident), sizeof(Base));
        QCOMPARE(sizeof(SingleIdent), sizeof(Base));
        QCOMPARE(sizeof(Token), sizeof(Base));
        QCOMPARE(sizeof(PhraseList), sizeof(Base));
        QCOMPARE(sizeof(DotAtom), sizeof(Base));
        QCOMPARE(sizeof(Parametrized), sizeof(Base));
        QCOMPARE(sizeof(ReturnPath), sizeof(Base));
        QCOMPARE(sizeof(MailCopiesTo), sizeof(Base));
        QCOMPARE(sizeof(ContentTransferEncoding), sizeof(Base));
        QCOMPARE(sizeof(ContentID), sizeof(Base));
        QCOMPARE(sizeof(ContentType), sizeof(Base));
        QCOMPARE(sizeof(Generic), sizeof(Base));
        QCOMPARE(sizeof(Control), sizeof(Base));
        QCOMPARE(sizeof(Date), sizeof(Base));
        QCOMPARE(sizeof(Newsgroups), sizeof(Base));
        QCOMPARE(sizeof(Lines), sizeof(Base));
    }

#define VERIFYSIZE( class, limit ) \
    qDebug() << #class << sizeof( class ); \
    QVERIFY( sizeof( class ) <= limit );

    void testHeadersPrivate()
    {
        VERIFYSIZE(BasePrivate, 16);
        VERIFYSIZE(UnstructuredPrivate, 24);
        VERIFYSIZE(StructuredPrivate, sizeof(BasePrivate));     // empty
        VERIFYSIZE(AddressPrivate, sizeof(StructuredPrivate));
        VERIFYSIZE(MailboxListPrivate, 24);
        VERIFYSIZE(SingleMailboxPrivate, sizeof(MailboxListPrivate));
        VERIFYSIZE(AddressListPrivate, 24);
        VERIFYSIZE(IdentPrivate, 40);
        VERIFYSIZE(SingleIdentPrivate, sizeof(IdentPrivate));
        VERIFYSIZE(TokenPrivate, 24);
        VERIFYSIZE(PhraseListPrivate, 24);
        VERIFYSIZE(DotAtomPrivate, 24);
        VERIFYSIZE(ParametrizedPrivate, 24);
        VERIFYSIZE(ReturnPathPrivate, 40);
        VERIFYSIZE(MailCopiesToPrivate, 32);
        VERIFYSIZE(ContentTransferEncodingPrivate, 32);
        VERIFYSIZE(ContentIDPrivate, 32);
        VERIFYSIZE(ContentTypePrivate, 40);
        VERIFYSIZE(GenericPrivate, 32);
        VERIFYSIZE(ControlPrivate, 32);
        VERIFYSIZE(DatePrivate, 24);
        VERIFYSIZE(NewsgroupsPrivate, 24);
        VERIFYSIZE(LinesPrivate, 24);
    }
};

QTEST_MAIN(SizeTest)

#include "sizetest.moc"
