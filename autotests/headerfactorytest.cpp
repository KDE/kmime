/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "headerfactorytest.h"

//#include <typeinfo>

#include <QTest>

#include <kmime_headerfactory_p.h>
#include <kmime_headers.h>

using namespace KMime;
using namespace KMime::Headers;
//using namespace KMime::Headers::Generics;

QTEST_MAIN(HeaderFactoryTest)

template <typename T>
bool isHeaderRegistered()
{
    T dummy;
    Base *h = Headers::createHeader(dummy.type());
    if (h) {
        delete h;
        return true;
    }
    return false;
}

void HeaderFactoryTest::testBuiltInHeaders()
{
    // Abstract headers have pure virtual methods.
    // Generic headers have an empty type().
    // All other built-in headers are supposed to be registered.

    //QVERIFY( isHeaderRegistered<Base>() ); // Abstract.
    //QVERIFY( isHeaderRegistered<Unstructured>() ); // Abstract.
    //QVERIFY( isHeaderRegistered<Structured>() ); // Abstract.
    //QVERIFY( isHeaderRegistered<Address>() ); // Abstract.
    //QVERIFY( isHeaderRegistered<MailboxList>() ); // Generic.
    //QVERIFY( isHeaderRegistered<SingleMailbox>() ); // Generic.
    //QVERIFY( isHeaderRegistered<AddressList>() ); // Generic.
    //QVERIFY( isHeaderRegistered<Ident>() ); // Generic.
    //QVERIFY( isHeaderRegistered<SingleIdent>() ); // Generic.
    //QVERIFY( isHeaderRegistered<Token>() ); // Generic.
    //QVERIFY( isHeaderRegistered<PhraseList>() ); // Generic.
    //QVERIFY( isHeaderRegistered<DotAtom>() ); // Generic.
    //QVERIFY( isHeaderRegistered<Parametrized>() ); // Generic.
    QVERIFY(isHeaderRegistered<ReturnPath>());
    QVERIFY(isHeaderRegistered<From>());
    QVERIFY(isHeaderRegistered<Sender>());
    QVERIFY(isHeaderRegistered<To>());
    QVERIFY(isHeaderRegistered<Cc>());
    QVERIFY(isHeaderRegistered<Bcc>());
    QVERIFY(isHeaderRegistered<ReplyTo>());
    QVERIFY(isHeaderRegistered<MailCopiesTo>());
    QVERIFY(isHeaderRegistered<ContentTransferEncoding>());
    QVERIFY(isHeaderRegistered<Keywords>());
    QVERIFY(isHeaderRegistered<MIMEVersion>());
    QVERIFY(isHeaderRegistered<MessageID>());
    QVERIFY(isHeaderRegistered<ContentID>());
    QVERIFY(isHeaderRegistered<Supersedes>());
    QVERIFY(isHeaderRegistered<InReplyTo>());
    QVERIFY(isHeaderRegistered<References>());
    QVERIFY(isHeaderRegistered<ContentType>());
    QVERIFY(isHeaderRegistered<ContentDisposition>());
    //QVERIFY( isHeaderRegistered<Generic>() ); // Changeable type().
    QVERIFY(isHeaderRegistered<Subject>());
    QVERIFY(isHeaderRegistered<Organization>());
    QVERIFY(isHeaderRegistered<ContentDescription>());
    QVERIFY(isHeaderRegistered<ContentLocation>());
    QVERIFY(isHeaderRegistered<Control>());
    QVERIFY(isHeaderRegistered<Date>());
    QVERIFY(isHeaderRegistered<Newsgroups>());
    QVERIFY(isHeaderRegistered<FollowUpTo>());
    QVERIFY(isHeaderRegistered<Lines>());
    QVERIFY(isHeaderRegistered<UserAgent>());
}

#include "moc_headerfactorytest.cpp"
