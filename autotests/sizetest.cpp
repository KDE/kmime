/*
  SPDX-FileCopyrightText: 2011 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "message.h"
#include "content_p.h"
#include "headers_p.h"

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
        QVERIFY(sizeof(ContentPrivate) <=
                (sizeof(QByteArray) * 5 + sizeof(QList<Content *>) * 2 + 32));
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
        VERIFYSIZE(BasePrivate, sizeof(QByteArray));
        VERIFYSIZE(UnstructuredPrivate, sizeof(BasePrivate) + sizeof(QString));
        VERIFYSIZE(StructuredPrivate, sizeof(BasePrivate));     // empty
        VERIFYSIZE(AddressPrivate, sizeof(StructuredPrivate));
        VERIFYSIZE(MailboxListPrivate,
                   sizeof(BasePrivate) + sizeof(QList<Types::Mailbox>));
        VERIFYSIZE(SingleMailboxPrivate, sizeof(MailboxListPrivate));
        VERIFYSIZE(AddressListPrivate, sizeof(BasePrivate) + sizeof(KMime::Types::AddressList));
        VERIFYSIZE(IdentPrivate, sizeof(AddressListPrivate) + sizeof(KMime::Types::AddrSpecList) + sizeof(QByteArray));
        VERIFYSIZE(SingleIdentPrivate, sizeof(IdentPrivate));
        VERIFYSIZE(TokenPrivate, sizeof(StructuredPrivate) + sizeof(QByteArray));
        VERIFYSIZE(PhraseListPrivate, sizeof(StructuredPrivate) + sizeof(QStringList));
        VERIFYSIZE(DotAtomPrivate, sizeof(StructuredPrivate) + sizeof(QByteArray));
        VERIFYSIZE(ParametrizedPrivate, sizeof(StructuredPrivate) + sizeof(std::map<QByteArray, QString>));
        VERIFYSIZE(ReturnPathPrivate, sizeof(AddressPrivate) + sizeof(Types::Mailbox));
        VERIFYSIZE(MailCopiesToPrivate, sizeof(AddressListPrivate) + 8);
        VERIFYSIZE(ContentTransferEncodingPrivate, sizeof(TokenPrivate) + 8);
        VERIFYSIZE(ContentIDPrivate, sizeof(SingleIdentPrivate));
        VERIFYSIZE(ContentTypePrivate, sizeof(ParametrizedPrivate) + sizeof(QByteArray) + 8);
        VERIFYSIZE(GenericPrivate, sizeof(UnstructuredPrivate) + 8);
        VERIFYSIZE(ControlPrivate, sizeof(StructuredPrivate) + 2*sizeof(QByteArray));
        VERIFYSIZE(DatePrivate, sizeof(StructuredPrivate) + 8);
        VERIFYSIZE(NewsgroupsPrivate,
                   sizeof(StructuredPrivate) + sizeof(QList<QByteArray>));
        VERIFYSIZE(LinesPrivate, sizeof(StructuredPrivate) + 8);
    }
};

QTEST_MAIN(SizeTest)

#include "sizetest.moc"
