/*
  SPDX-FileCopyrightText: 2011 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_message.h"
#include "kmime_content_p.h"
#include "kmime_headers_p.h"

#include <QTest>
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
        VERIFYSIZE(BasePrivate, 8);
        VERIFYSIZE(UnstructuredPrivate, 16);
        VERIFYSIZE(StructuredPrivate, sizeof(BasePrivate));     // empty
        VERIFYSIZE(AddressPrivate, sizeof(StructuredPrivate));
        VERIFYSIZE(MailboxListPrivate, 16);
        VERIFYSIZE(SingleMailboxPrivate, sizeof(MailboxListPrivate));
        VERIFYSIZE(AddressListPrivate, 16);
        VERIFYSIZE(IdentPrivate, 32);
        VERIFYSIZE(SingleIdentPrivate, sizeof(IdentPrivate));
        VERIFYSIZE(TokenPrivate, 16);
        VERIFYSIZE(PhraseListPrivate, 16);
        VERIFYSIZE(DotAtomPrivate, 16);
        VERIFYSIZE(ParametrizedPrivate, 16);
        VERIFYSIZE(ReturnPathPrivate, 32);
        VERIFYSIZE(MailCopiesToPrivate, 24);
        VERIFYSIZE(ContentTransferEncodingPrivate, 24);
        VERIFYSIZE(ContentIDPrivate, 24);
        VERIFYSIZE(ContentTypePrivate, 32);
        VERIFYSIZE(GenericPrivate, 24);
        VERIFYSIZE(ControlPrivate, 24);
        VERIFYSIZE(DatePrivate, 16);
        VERIFYSIZE(NewsgroupsPrivate, 16);
        VERIFYSIZE(LinesPrivate, 16);
    }
};

QTEST_MAIN(SizeTest)

#include "sizetest.moc"
