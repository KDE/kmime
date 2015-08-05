/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "headerfactorytest.h"

//#include <typeinfo>

#include <QDebug>
#include <qtest.h>

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
