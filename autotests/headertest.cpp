/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "headertest.h"

#include <QTest>

#include "headers.h"

using namespace Qt::Literals;
using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Headers::Generics;

// the following test cases are taken from KDE mailinglists, bug reports, RFC 2045,
// RFC 2183 and RFC 2822, Appendix A

QTEST_MAIN(HeaderTest)

void HeaderTest::testIdentHeader()
{
    // empty header
    auto h = new Headers::Generics::Ident();
    QVERIFY(h->isEmpty());

    // parse single identifier
    h->from7BitString(QByteArray("<1162746587.784559.5038.nullmailer@svn.kde.org>"));
    QCOMPARE(h->identifiers().count(), 1);
    QCOMPARE(h->identifiers().first(), QByteArray("1162746587.784559.5038.nullmailer@svn.kde.org"));
    QCOMPARE(h->asUnicodeString(), QString::fromLatin1("<1162746587.784559.5038.nullmailer@svn.kde.org>"));
    QVERIFY(!h->isEmpty());
    delete h;

    // parse multiple identifiers
    h = new Headers::Generics::Ident();
    h->from7BitString(QByteArray("<1234@local.machine.example> <3456@example.net>"));
    QCOMPARE(h->identifiers().count(), 2);
    auto ids = h->identifiers();
    QCOMPARE(ids.takeFirst(), QByteArray("1234@local.machine.example"));
    QCOMPARE(ids.first(), QByteArray("3456@example.net"));
    delete h;

    // parse multiple identifiers with folded headers
    h = new Headers::Generics::Ident();
    h->from7BitString(QByteArray("<1234@local.machine.example>\n  <3456@example.net>"));
    QCOMPARE(h->identifiers().count(), 2);
    ids = h->identifiers();
    QCOMPARE(ids.takeFirst(), QByteArray("1234@local.machine.example"));
    QCOMPARE(ids.first(), QByteArray("3456@example.net"));

    // appending of new identifiers (with and without angle-brackets)
    h->appendIdentifier("<abcd.1234@local.machine.tld>");
    h->appendIdentifier("78910@example.net");
    QCOMPARE(h->identifiers().count(), 4);

    // assemble the final header
    QCOMPARE(h->as7BitString(), QByteArray("<1234@local.machine.example> <3456@example.net> <abcd.1234@local.machine.tld> <78910@example.net>"));
    delete h;

    // parsing of ident with literal domain
    h = new Headers::Generics::Ident();
    const QByteArray ident = QByteArray("<O55F3Y9E5MmKFwBN@[127.0.0.1]>");
    h->appendIdentifier(ident);
    QEXPECT_FAIL("", "Parsing strips square brackets.", Continue);
    QCOMPARE(h->as7BitString(), QByteArray(ident));
    delete h;
}

void HeaderTest::testAddressListHeader()
{
    // empty header
    auto h = new Headers::Generics::AddressList();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QLatin1StringView("joe@where.test"));
    QCOMPARE(h->displayString(), QLatin1StringView("joe@where.test"));
    QCOMPARE(h->asUnicodeString(), QLatin1StringView("joe@where.test"));
    delete h;

    // parsing and re-assembling a single address with display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("Pete <pete@silly.example>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("pete@silly.example"));
    QCOMPARE(h->displayNames().first(), QLatin1StringView("Pete"));
    QCOMPARE(h->displayString(), QLatin1StringView("Pete"));
    QCOMPARE(h->asUnicodeString(),
             QLatin1StringView("Pete <pete@silly.example>"));
    QCOMPARE(h->as7BitString(), QByteArray("Pete <pete@silly.example>"));
    delete h;

    // parsing a single address with legacy comment style display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("jdoe@machine.example (John Doe)");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("jdoe@machine.example"));
    QCOMPARE(h->displayNames().first(), QLatin1StringView("John Doe"));
    QCOMPARE(h->asUnicodeString(),
             QLatin1StringView("John Doe <jdoe@machine.example>"));
    delete h;

    // parsing and re-assembling list of different addresses
    h = new Headers::Generics::AddressList();
    h->from7BitString("Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>");
    QCOMPARE(h->addresses().count(), 3);
    QStringList names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1StringView("Mary Smith"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("jdoe@example.org"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("Who?"));
    QCOMPARE(h->displayString(),
             QLatin1StringView("Mary Smith, jdoe@example.org, Who?"));
    QCOMPARE(h->as7BitString(), QByteArray("Mary Smith <mary@x.test>, jdoe@example.org, Who? <one@y.test>"));
    delete h;

    // same again with some interesting quoting
    h = new Headers::Generics::AddressList();
    h->from7BitString(R"("Joe Q. Public" <john.q.public@example.com>, <boss@nil.test>, "Giant; \"Big\" Box" <sysservices@example.net>)");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1StringView("Joe Q. Public"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("boss@nil.test"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("Giant; \"Big\" Box"));
    QCOMPARE(h->as7BitString(), QByteArray("\"Joe Q. Public\" <john.q.public@example.com>, boss@nil.test, \"Giant; \\\"Big\\\" Box\" <sysservices@example.net>"));
    delete h;

    // a display name with non-latin1 content
    h = new Headers::Generics::AddressList();
    h->from7BitString("Ingo =?iso-8859-15?q?Kl=F6cker?= <kloecker@kde.org>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    QCOMPARE(h->as7BitString(), "Ingo =?UTF-8?B?S2zDtmNrZXI=?= <kloecker@kde.org>");
    h->setRFC2047Charset("iso-8859-1");
    QCOMPARE(h->as7BitString(), QByteArray("Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>"));
    delete h;


    // a display name with non-latin1 content in both name components
    h = new Headers::Generics::AddressList();
    const QString testAddress = QString::fromUtf8("Ingö Klöcker <kloecker@kde.org>");
    h->fromUnicodeString(testAddress);
    QCOMPARE(h->asUnicodeString(), testAddress);
    delete h;

    {
        // a display name with non-latin1 content in both name components
        h = new Headers::Generics::AddressList();
        const QString testAddress = QString::fromUtf8("\"Rüedi-Huser, Thomas\" <test@test.org>");
        h->fromUnicodeString(testAddress);
        QEXPECT_FAIL("", "AddressList::prettyAddresses() does not quote the mailbox correctly", Continue);
        QCOMPARE(h->asUnicodeString(), testAddress);
        delete h;
    }

    // again, this time legacy style
    h = new Headers::Generics::AddressList();
    h->from7BitString("kloecker@kde.org (Ingo =?iso-8859-15?q?Kl=F6cker?=)");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    delete h;

    // parsing an empty group
    h = new Headers::Generics::AddressList();
    h->from7BitString("Undisclosed recipients:;");
    QCOMPARE(h->addresses().count(), 0);
    delete h;

    // parsing and re-assembling an address list with a group
    h = new Headers::Generics::AddressList();
    h->from7BitString("A Group:Chris Jones <c@a.test>,joe@where.test,John <jdoe@one.test>;");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1StringView("Chris Jones"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("joe@where.test"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("John"));
    QCOMPARE(h->as7BitString(), QByteArray("Chris Jones <c@a.test>, joe@where.test, John <jdoe@one.test>"));
    delete h;

    // modifying a header
    h = new Headers::Generics::AddressList();
    h->from7BitString("John <jdoe@one.test>");
    h->addAddress("<kloecker@kde.org>", QString::fromUtf8("Ingo Klöcker"));
    h->addAddress("c@a.test");
    QCOMPARE(h->addresses().count(), 3);
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("John <jdoe@one.test>, Ingo Klöcker <kloecker@kde.org>, c@a.test"));
    QCOMPARE(h->as7BitString(), QByteArray("John <jdoe@one.test>, Ingo =?UTF-8?B?S2zDtmNrZXI=?= <kloecker@kde.org>, c@a.test"));
    h->setRFC2047Charset("ISO-8859-1");
    QCOMPARE(h->as7BitString(), QByteArray("John <jdoe@one.test>, Ingo =?ISO-8859-1?Q?Kl=F6cker?= <kloecker@kde.org>, c@a.test"));
    delete h;

    // parsing from utf-8
    h = new Headers::Generics::AddressList();
    h->fromUnicodeString(QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("kloecker@kde.org"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("Ingo Klöcker"));
    delete h;

    // based on bug #137033, a header broken in various ways: ';' as list separator,
    // unquoted '.' in display name
    h = new Headers::Generics::AddressList();
    h->from7BitString("Vice@censored.serverkompetenz.net,\n    President@mail2.censored.net;\"Int\\\\\\\\\\\\\\\\\\\\'l\" Lotto Commission. <censored@yahoo.fr>");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(),
             QLatin1StringView("Vice@censored.serverkompetenz.net"));
    QCOMPARE(names.takeFirst(),
             QLatin1StringView("President@mail2.censored.net"));
    // there is a wrong ' ' after the name, but since the header is completely
    // broken, we can be happy it parses at all...
    QCOMPARE(names.takeFirst(),
             QLatin1StringView("Int\\\\\\\\\\'l Lotto Commission. "));
    auto addrs = h->addresses();
    QCOMPARE(addrs.takeFirst(), QByteArray("Vice@censored.serverkompetenz.net"));
    QCOMPARE(addrs.takeFirst(), QByteArray("President@mail2.censored.net"));
    QCOMPARE(addrs.takeFirst(), QByteArray("censored@yahoo.fr"));
    delete h;

    // based on bug #102010, a display name containing '<'
    h = new Headers::Generics::AddressList();
    h->from7BitString("\"|<onrad\" <censored@censored.dy>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("censored@censored.dy"));
    QCOMPARE(h->displayNames().first(), QLatin1StringView("|<onrad"));
    QCOMPARE(h->as7BitString(), QByteArray("\"|<onrad\" <censored@censored.dy>"));
    delete h;

    // based on bug #93790 (legacy display name with nested comments)
    h = new Headers::Generics::AddressList();
    h->from7BitString("first.name@domain.tld (first name (nickname))");
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(),
             QLatin1StringView("first name (nickname)"));
    QCOMPARE(h->as7BitString(), QByteArray("\"first name (nickname)\" <first.name@domain.tld>"));
    delete h;

    // rfc 2047 encoding in quoted name (it is not allowed there as per the RFC, but it happens)
    // some software == current KMail (v1.12.90) ...
    h = new Headers::Generics::AddressList();
    h->from7BitString(QByteArray("\"Ingo =?iso-8859-15?q?Kl=F6cker?=\" <kloecker@kde.org>"));
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("Ingo Klöcker <kloecker@kde.org>"));
    delete h;

    // corner case of almost-rfc2047 encoded string in quoted string but not
    h = new Headers::Generics::AddressList();
    h->from7BitString("\"Some =Use ?r\" <user@example.com>");
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->as7BitString(), QByteArray("\"Some =Use ?r\" <user@example.com>"));
    delete h;

    // corner case of almost-rfc2047 encoded string in quoted string but not
    h = new Headers::Generics::AddressList();
    h->from7BitString("\"Some ?=U=?se =?r\" <user@example.com>");
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->as7BitString(), QByteArray("\"Some ?=U=?se =?r\" <user@example.com>"));
    delete h;

    // based on bug #139477, trailing '.' in domain name (RFC 3696, section 2 - https://tools.ietf.org/html/rfc3696#page-4)
    h = new Headers::Generics::AddressList();
    h->from7BitString("joe@where.test.");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe@where.test."));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(), QLatin1StringView("joe@where.test."));
    QCOMPARE(h->asUnicodeString(), QLatin1StringView("joe@where.test."));
    delete h;

    h = new Headers::Generics::AddressList();
    h->from7BitString("Mary Smith <mary@x.test>, jdoe@example.org., Who? <one@y.test>");
    QCOMPARE(h->addresses().count(), 3);
    names = h->displayNames();
    QCOMPARE(names.takeFirst(), QLatin1StringView("Mary Smith"));
    QCOMPARE(names.takeFirst(), QLatin1StringView("jdoe@example.org."));
    QCOMPARE(names.takeFirst(), QLatin1StringView("Who?"));
    QCOMPARE(h->as7BitString(), QByteArray("Mary Smith <mary@x.test>, jdoe@example.org., Who? <one@y.test>"));
    delete h;

    //Bug 421251
    // a display name with non-latin1 content
    h = new Headers::Generics::AddressList();
    h->from7BitString("=?iso-8859-1?Q?=22I=F1igo_Salvador_Azurmendi=22?= <xalba@clientes.euskaltel.es>");
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("xalba@clientes.euskaltel.es"));
    QCOMPARE(h->displayNames().first(), QString::fromUtf8("I\u00F1igo Salvador Azurmendi"));
    QCOMPARE(h->asUnicodeString(), QString::fromUtf8("I\u00F1igo Salvador Azurmendi <xalba@clientes.euskaltel.es>"));
    QCOMPARE(h->as7BitString(), QByteArray("=?UTF-8?B?ScOxaWdv?= Salvador Azurmendi <xalba@clientes.euskaltel.es>"));
    h->setRFC2047Charset("ISO-8859-1");
    QCOMPARE(h->as7BitString(), QByteArray("=?ISO-8859-1?Q?I=F1igo?= Salvador Azurmendi <xalba@clientes.euskaltel.es>"));
    delete h;
}

void HeaderTest::testMailboxListHeader()
{
    // empty header
    auto h = new Headers::Generics::MailboxList();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe_smith@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->addresses().count(), 1);
    QCOMPARE(h->addresses().first(), QByteArray("joe_smith@where.test"));
    QCOMPARE(h->displayNames().count(), 1);
    QCOMPARE(h->displayNames().first(),
             QLatin1StringView("joe_smith@where.test"));
    QCOMPARE(h->displayString(), QLatin1StringView("joe_smith@where.test"));
    QCOMPARE(h->asUnicodeString(), QLatin1StringView("joe_smith@where.test"));

    // https://bugzilla.novell.com/show_bug.cgi?id=421057 (but apparently this was not the cause of the bug)
    h->from7BitString("fr...@ce.sco (Francesco)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailboxes().count(), 1);
    QCOMPARE(h->displayString(), QLatin1StringView("Francesco"));
    QCOMPARE(h->asUnicodeString(),
             QLatin1StringView("Francesco <fr...@ce.sco>"));

    delete h;
}

void HeaderTest::testSingleMailboxHeader()
{
    // empty header
    auto h = new Headers::Generics::SingleMailbox();
    QVERIFY(h->isEmpty());

    // parse single simple address
    h->from7BitString("joe_smith@where.test");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().prettyAddress(), "joe_smith@where.test"_L1);
    QCOMPARE(h->asUnicodeString(), QLatin1StringView("joe_smith@where.test"));

    // parse single simple address with display name
    h->from7BitString("John Smith <joe_smith@where.test>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().name(), "John Smith"_L1);
    QCOMPARE(h->asUnicodeString(),
             QLatin1StringView("John Smith <joe_smith@where.test>"));
    QCOMPARE(h->mailbox().prettyAddress(Types::Mailbox::QuoteAlways), "\"John Smith\" <joe_smith@where.test>"_L1);

    // parse quoted display name with \ in it
    h->from7BitString(R"("Lastname\, Firstname" <firstname.lastname@example.com>)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->asUnicodeString().toLatin1().data(),
             "Lastname, Firstname <firstname.lastname@example.com>");
    QCOMPARE(h->mailbox().prettyAddress(), "Lastname, Firstname <firstname.lastname@example.com>"_L1);
    QCOMPARE(h->mailbox().prettyAddress(Types::Mailbox::QuoteWhenNecessary), "\"Lastname, Firstname\" <firstname.lastname@example.com>"_L1);

    // parse quoted display name with " in it
    h->from7BitString(R"("John \"the guru\" Smith" <john.smith@mail.domain>)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().prettyAddress(Types::Mailbox::QuoteWhenNecessary), "\"John \\\"the guru\\\" Smith\" <john.smith@mail.domain>"_L1);
    QCOMPARE(h->as7BitString().data(),
             "\"John \\\"the guru\\\" Smith\" <john.smith@mail.domain>");

    // The following tests are for broken clients that by accident add quotes inside of encoded words that enclose the
    // display name. We strip away those quotes, which is not strictly correct, but much nicer.
    h->from7BitString("=?iso-8859-1?Q?=22Andre_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().name(), "Andre Woebbeking"_L1);
    h->from7BitString("=?iso-8859-1?Q?=22Andre_=22Mr._Tall=22_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().name(), "Andre \"Mr. Tall\" Woebbeking"_L1);
    h->from7BitString("=?iso-8859-1?Q?=22Andre_=22?= =?iso-8859-1?Q?Mr._Tall?= =?iso-8859-1?Q?=22_Woebbeking=22?= <woebbeking@example.com>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mailbox().name(), "Andre \"Mr. Tall\" Woebbeking"_L1);

    delete h;
}

void HeaderTest::testMailCopiesToHeader()
{
    Headers::MailCopiesTo *h;

    // empty header
    h = new Headers::MailCopiesTo();
    QVERIFY(h->isEmpty());
    QVERIFY(!h->alwaysCopy());
    QVERIFY(!h->neverCopy());

    // set to always copy to poster
    h->setAlwaysCopy();
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    QVERIFY(!h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("poster"));

    // set to never copy
    h->setNeverCopy();
    QVERIFY(!h->isEmpty());
    QVERIFY(!h->alwaysCopy());
    QVERIFY(h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("nobody"));
    delete h;

    // parse copy to poster
    h = new MailCopiesTo;
    h->from7BitString("always");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    delete h;

    h = new MailCopiesTo;
    h->from7BitString("poster");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->alwaysCopy());
    delete h;

    // parse never copy
    h = new MailCopiesTo;
    h->from7BitString("never");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->neverCopy());
    delete h;

    h = new MailCopiesTo;
    h->from7BitString("nobody");
    QVERIFY(h->addresses().isEmpty());
    QVERIFY(!h->isEmpty());
    QVERIFY(h->neverCopy());
    delete h;

    // parsing is case-insensitive
    h = new MailCopiesTo;
    h->from7BitString("AlWays");
    QVERIFY(h->alwaysCopy());
    delete h;

    // parse address
    h = new MailCopiesTo;
    h->from7BitString("vkrause@kde.org");
    QVERIFY(!h->addresses().isEmpty());
    QVERIFY(h->alwaysCopy());
    QVERIFY(!h->neverCopy());
    QCOMPARE(h->as7BitString(), QByteArray("vkrause@kde.org"));
    delete h;
}

void HeaderTest::testParametrizedHeader()
{
    Parametrized *h;

    // empty header
    h = new Parametrized();
    QVERIFY(h->isEmpty());
    QVERIFY(!h->hasParameter("foo"));

    // add a parameter
    h->setParameter(QByteArrayLiteral("filename"), QStringLiteral("bla.jpg"));
    QVERIFY(!h->isEmpty());
    QVERIFY(h->hasParameter("filename"));
    QVERIFY(h->hasParameter("FiLeNaMe"));
    QVERIFY(!h->hasParameter("bla.jpg"));
    QCOMPARE(h->parameter("filename"), QLatin1StringView("bla.jpg"));
    QCOMPARE(h->parameter("FILENAME"), QLatin1StringView("bla.jpg"));
    QCOMPARE(h->as7BitString(), QByteArray("filename=\"bla.jpg\""));
    delete h;

    // parse a parameter list
    h = new Parametrized;
    h->from7BitString("filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\"");
    QCOMPARE(h->parameter("filename"), QLatin1StringView("genome.jpeg"));
    QCOMPARE(h->parameter("modification-date"), QLatin1StringView("Wed, 12 Feb 1997 16:29:51 -0500"));
    QCOMPARE(h->as7BitString(), QByteArray("filename=\"genome.jpeg\"; modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\""));
    delete h;

    // quoting of whitespaces in parameter value
    h = new Parametrized();
    h->setParameter(QByteArrayLiteral("boundary"), QLatin1StringView("simple boundary"));
    QCOMPARE(h->as7BitString(), QByteArray("boundary=\"simple boundary\""));
    delete h;

    // TODO: test RFC 2047 encoded values
    // TODO: test case-insensitive key-names
}

void HeaderTest::testContentDispositionHeader()
{
    ContentDisposition *h;

    // empty header
    h = new ContentDisposition();
    QVERIFY(h->isEmpty());

    // set some values
    h->setFilename(QLatin1StringView("test.jpg"));
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());
    h->setDisposition(CDattachment);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("attachment; filename=\"test.jpg\""));
    delete h;

    // parse parameter-less header
    h = new ContentDisposition;
    h->from7BitString("inline");
    QCOMPARE(h->disposition(), CDinline);
    QVERIFY(h->filename().isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("inline"));
    delete h;

    // parse header with parameter
    h = new ContentDisposition;
    h->from7BitString("attachment; filename=genome.jpeg;\n modification-date=\"Wed, 12 Feb 1997 16:29:51 -0500\";");
    QCOMPARE(h->disposition(), CDattachment);
    QCOMPARE(h->filename(), QLatin1StringView("genome.jpeg"));
    delete h;

    // TODO: test for case-insensitive disposition value

    // Bug 362650
    h = new ContentDisposition;
    h->from7BitString("attachment;\n"
     "filename*0*=UTF-8''%D0%AD%D1%82%D0%BE%D0%92%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD;"
     "filename*1*=%D0%B8%D0%B5%D0%A1%D0%94%D0%BB%D0%B8%D0%BD%D0%BD%D1%8B%D0%BC;"
     "filename*2*=%D0%98%D0%BC%D0%B5%D0%BC%D0%A4%D0%B0%D0%B9%D0%BB%D0%B0%D0%A1;"
     "filename*3*=%D0%BE%D0%B2%D1%81%D0%B5%D0%BC%D0%91%D0%B5%D0%B7%D0%9F%D1%80;"
     "filename*4*=%D0%BE%D0%B1%D0%B5%D0%BB%D0%BE%D0%B2%D0%98%D0%95%D1%89%D1%91;"
     "filename*5*=%D0%A0%D0%B0%D0%B7%D0%AD%D1%82%D0%BE%D0%92%D0%BB%D0%BE%D0%B6;"
     "filename*6*=%D0%B5%D0%BD%D0%B8%D0%B5%D0%A1%D0%94%D0%BB%D0%B8%D0%BD%D0%BD;"
     "filename*7*=%D1%8B%D0%BC%D0%98%D0%BC%D0%B5%D0%BC%D0%A4%D0%B0%D0%B9%D0%BB;"
     "filename*8*=%D0%B0%D0%A1%D0%BE%D0%B2%D1%81%D0%B5%D0%BC%D0%91%D0%B5%D0%B7;"
     "filename*9*=%D0%9F%D1%80%D0%BE%D0%B1%D0%B5%D0%BB%D0%BE%D0%B2%2E%74%78%74");
    QCOMPARE(h->disposition(), CDattachment);
    QCOMPARE(h->filename(), QString::fromUtf8("ЭтоВложениеСДлиннымИмемФайлаСовсемБезПробеловИЕщёРазЭтоВложениеСДлиннымИмемФайлаСовсемБезПробелов.txt"));
    delete h;

    h = new ContentDisposition;
    h->from7BitString("attachment; filename*=UTF-8''%D0%AD%D1%82%D0%BE%D0%92%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD%D0%B8%D0%B5%D0%A1%D0%94%D0%BB%D0%B8%D0%BD%D0%BD%D1%8B%D0%BC%D0%98%D0%BC%D0%B5%D0%BC%D0%A4%D0%B0%D0%B9%D0%BB%D0%B0%D0%A1%D0%BE%D0%B2%D1%81%D0%B5%D0%BC%D0%91%D0%B5%D0%B7%D0%9F%D1%80%D0%BE%D0%B1%D0%B5%D0%BB%D0%BE%D0%B2%D0%98%D0%95%D1%89%D1%91%D0%A0%D0%B0%D0%B7%D0%AD%D1%82%D0%BE%D0%92%D0%BB%D0%BE%D0%B6%D0%B5%D0%BD%D0%B8%D0%B5%D0%A1%D0%94%D0%BB%D0%B8%D0%BD%D0%BD%D1%8B%D0%BC%D0%98%D0%BC%D0%B5%D0%BC%D0%A4%D0%B0%D0%B9%D0%BB%D0%B0%D0%A1%D0%BE%D0%B2%D1%81%D0%B5%D0%BC%D0%91%D0%B5%D0%B7%D0%9F%D1%80%D0%BE%D0%B1%D0%B5%D0%BB%D0%BE%D0%B2%2Etxt");
    QCOMPARE(h->disposition(), CDattachment);
    QCOMPARE(h->filename(), QString::fromUtf8("ЭтоВложениеСДлиннымИмемФайлаСовсемБезПробеловИЕщёРазЭтоВложениеСДлиннымИмемФайлаСовсемБезПробелов.txt"));
    delete h;
}

void HeaderTest::testContentTypeHeader()
{
    ContentType *h;

    //Bug 362650 (test is without space => ok)
    h = new ContentType;
    h->from7BitString("text/plain;\n name=\"=?UTF-8?B?0K3RgtC+0JLQu9C+0LbQtdC90LjQtdCh0JTQu9C40L3QvdGL0LzQmNC8?="
                      "=?UTF-8?B?0LXQvNCk0LDQudC70LDQodC+0LLRgdC10LzQkdC10LfQn9GA0L7QsdC1?="
                      "=?UTF-8?B?0LvQvtCy0JjQldGJ0ZHQoNCw0LfQrdGC0L7QktC70L7QttC10L3QuNC1?="
                      "=?UTF-8?B?0KHQlNC70LjQvdC90YvQvNCY0LzQtdC80KTQsNC50LvQsNCh0L7QstGB?="
                      "=?UTF-8?B?0LXQvNCR0LXQt9Cf0YDQvtCx0LXQu9C+0LIudHh0?=\"");
    QCOMPARE(h->name(), QString::fromUtf8("ЭтоВложениеСДлиннымИмемФайлаСовсемБезПробеловИЕщёРазЭтоВложениеСДлиннымИмемФайлаСовсемБезПробелов.txt"));
    delete h;
    h = new ContentType;
    h->from7BitString("text/plain;\n name=\"=?UTF-8?B?0K3RgtC+0JLQu9C+0LbQtdC90LjQtdCh0JTQu9C40L3QvdGL0LzQmNC8?="
                      " =?UTF-8?B?0LXQvNCk0LDQudC70LDQodC+0LLRgdC10LzQkdC10LfQn9GA0L7QsdC1?="
                      " =?UTF-8?B?0LvQvtCy0JjQldGJ0ZHQoNCw0LfQrdGC0L7QktC70L7QttC10L3QuNC1?="
                      " =?UTF-8?B?0KHQlNC70LjQvdC90YvQvNCY0LzQtdC80KTQsNC50LvQsNCh0L7QstGB?="
                      " =?UTF-8?B?0LXQvNCR0LXQt9Cf0YDQvtCx0LXQu9C+0LIudHh0?=\"");
    QCOMPARE(h->name(), QString::fromUtf8("ЭтоВложениеСДлиннымИмемФайлаСовсемБезПробеловИЕщёРазЭтоВложениеСДлиннымИмемФайлаСовсемБезПробелов.txt"));
    delete h;

    // empty header
    h = new ContentType();
    QVERIFY(h->isEmpty());

    // Empty content-type means text/plain (RFC 2045 §5.2)
    QVERIFY(h->isPlainText());
    QVERIFY(h->isText());

    // set a mimetype
    h->setMimeType("text/plain");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->mimeType(), QByteArray("text/plain"));
    QCOMPARE(h->mediaType(), QByteArray("text"));
    QCOMPARE(h->subType(), QByteArray("plain"));
    QVERIFY(h->isText());
    QVERIFY(h->isPlainText());
    QVERIFY(!h->isMultipart());
    QVERIFY(!h->isPartial());
    QVERIFY(h->isMediatype("text"));
    QVERIFY(h->isSubtype("plain"));
    QCOMPARE(h->as7BitString(), QByteArray("text/plain"));

    // add some parameters
    h->setId("bla");
    h->setCharset("us-ascii");
    QCOMPARE(h->as7BitString(), QByteArray("text/plain; charset=\"us-ascii\"; id=\"bla\""));
    delete h;

    // parse a complete header
    h = new ContentType;
    h->from7BitString("text/plain; charset=us-ascii (Plain text)");
    QVERIFY(h->isPlainText());
    QCOMPARE(h->charset(), QByteArray("us-ascii"));
    delete h;

    // bug #136631 (name with rfc 2231 style parameter wrapping)
    h = new ContentType;
    h->from7BitString("text/plain;\n name*0=\"PIN_Brief_box1@xx.xxx.censored_Konfigkarte.confi\";\n name*1=\"guration.txt\"");
    QVERIFY(h->isPlainText());
    QCOMPARE(
        h->name(),
        QLatin1StringView(
            "PIN_Brief_box1@xx.xxx.censored_Konfigkarte.configuration.txt"));
    delete h;

    // bug #197958 (name of Content-Type sent by Mozilla Thunderbird are not parsed -- test case generated with v2.0.0.22)
    h = new ContentType;
    h->from7BitString("text/plain;\n name=\"=?ISO-8859-1?Q?lor=E9m_ipsum=2Etxt?=\"");
    QCOMPARE(h->name(), QString::fromUtf8("lorém ipsum.txt"));
    delete h;

    // bug #197958 (name of Content-Type sent by Mozilla Thunderbird are not parsed -- test case generated with v2.0.0.22)
    // But with unquoted string
    QEXPECT_FAIL("", "Unquoted rfc2047 strings are not supported as of now", Continue);
    h = new ContentType;
    h->from7BitString("text/plain;\n name==?ISO-8859-1?Q?lor=E9m_ipsum=2Etxt?=");
    QCOMPARE(h->name(), QString::fromUtf8("lorém ipsum.txt"));
    delete h;

    // make ervin's unit test happy
    h = new ContentType;
    h->setMimeType("MULTIPART/MIXED");
    QVERIFY(h->isMultipart());
    QVERIFY(h->isMediatype("multipart"));
    QVERIFY(h->isMediatype("Multipart"));
    QVERIFY(h->isMediatype("MULTIPART"));
    QVERIFY(h->isSubtype("mixed"));
    QVERIFY(h->isSubtype("Mixed"));
    QVERIFY(h->isSubtype("MIXED"));
    QCOMPARE(h->mimeType(), QByteArray("MULTIPART/MIXED"));
    QCOMPARE(h->mediaType(), QByteArray("MULTIPART"));
    QCOMPARE(h->subType(), QByteArray("MIXED"));
    delete h;

}

void HeaderTest::testTokenHeader()
{
    Token *h;

    // empty header
    h = new Token();
    QVERIFY(h->isEmpty());

    // set a token
    h->setToken("bla");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("bla"));
    delete h;

    // parse a header
    h = new Token;
    h->from7BitString("value (comment)");
    QCOMPARE(h->token(), QByteArray("value"));
    QCOMPARE(h->as7BitString(), QByteArray("value"));
    delete h;
}

void HeaderTest::testContentTransferEncoding()
{
    ContentTransferEncoding *h;

    // set an encoding
    h = new ContentTransferEncoding();
    h->setEncoding(CEbinary);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("binary"));
    delete h;

    // parse a header
    h = new ContentTransferEncoding;
    h->from7BitString("(comment) base64");
    QCOMPARE(h->encoding(), CEbase64);
    QCOMPARE(h->as7BitString(), QByteArray("base64"));
    delete h;
}

void HeaderTest::testPhraseListHeader()
{
    PhraseList *h;

    // empty header
    h = new PhraseList();
    QVERIFY(h->isEmpty());
    delete h;

    // parse a simple phrase list
    h = new PhraseList;
    h->from7BitString("foo,\n bar");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->phrases().count(), 2);
    QStringList phrases = h->phrases();
    QCOMPARE(phrases.takeFirst(), QLatin1StringView("foo"));
    QCOMPARE(phrases.takeFirst(), QLatin1StringView("bar"));
    QCOMPARE(h->as7BitString(), QByteArray("foo, bar"));
    delete h;

    // TODO: encoded/quoted phrases
}

void HeaderTest::testDotAtomHeader()
{
    DotAtom *h;

    // empty header
    h = new DotAtom;
    QVERIFY(h->isEmpty());

    // parse a simple dot atom
    h->from7BitString("1.0 (mime version)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->asUnicodeString(), QLatin1StringView("1.0"));
    delete h;

    // TODO: more complex atoms
}

void HeaderTest::testDateHeader()
{
    Date *h;

    // empty header
    h = new Date();
    QVERIFY(h->isEmpty());

    // parse a simple date
    h->from7BitString("Fri, 21 Nov 1997 09:55:06 -0600");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().offsetFromUtc(), -6 * 3600);
    QCOMPARE(h->as7BitString(), QByteArray("Fri, 21 Nov 1997 09:55:06 -0600"));
    delete h;

    // white spaces and comment (from RFC 2822, Appendix A.5)
    h = new Date;
    h->from7BitString("Thu,\n  13\n    Feb\n  1969\n  23:32\n  -0330 (Newfoundland Time)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1969, 2, 13));
    QCOMPARE(h->dateTime().time(), QTime(23, 32));
    QCOMPARE(h->dateTime().offsetFromUtc(), -12600);
    QCOMPARE(h->as7BitString(), QByteArray("Thu, 13 Feb 1969 23:32:00 -0330"));
    delete h;

    // obsolete date format (from RFC 2822, Appendix A.6.2)
    h = new Date;
    h->from7BitString("21 Nov 97 09:55:06 GMT");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().offsetFromUtc(), 0);
    delete h;

    // obsolete whitespaces and commnets (from RFC 2822, Appendix A.6.3)
    h = new Date;
    h->from7BitString("Fri, 21 Nov 1997 09(comment):   55  :  06 -0600");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1997, 11, 21));
    QCOMPARE(h->dateTime().time(), QTime(9, 55, 6));
    QCOMPARE(h->dateTime().offsetFromUtc(), -6 * 3600);
    delete h;

    // Make sure uppercase OCT is parsed correctly - bug 150620
    h = new Date;
    h->from7BitString("08 OCT 08 16:54:05 +0000");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2008, 10, 8));
    QCOMPARE(h->dateTime().time(), QTime(16, 54, 05));
    QCOMPARE(h->dateTime().offsetFromUtc(), 0);
    delete h;

    // Test for bug 111633, year < 1970
    h = new Date;
    h->from7BitString("Mon, 27 Aug 1956 21:31:46 +0200");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(1956, 8, 27));
    QCOMPARE(h->dateTime().time(), QTime(21, 31, 46));
    QCOMPARE(h->dateTime().offsetFromUtc(), +2 * 3600);
    delete h;

    // Test for bug 207766
    h = new Date;
    h->from7BitString("Fri, 18 Sep 2009 04:44:55 -0400");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2009, 9, 18));
    QCOMPARE(h->dateTime().time(), QTime(4, 44, 55));
    QCOMPARE(h->dateTime().offsetFromUtc(), -4 * 3600);
    delete h;

    // Test for bug 260761
    h = new Date;
    h->from7BitString("Sat, 18 Dec 2010 14:01:21 \"GMT\"");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2010, 12, 18));
    QCOMPARE(h->dateTime().time(), QTime(14, 1, 21));
    QCOMPARE(h->dateTime().offsetFromUtc(), 0);
    delete h;

    // old asctime()-like formatted date; regression to KDE3; see bug 117848
    h = new Date;
    h->from7BitString("Thu Mar 30 18:36:28 CEST 2006");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2006, 3, 30));
    QCOMPARE(h->dateTime().time(), QTime(18, 36, 28));
    QCOMPARE(h->dateTime().offsetFromUtc(), 2 * 3600);
    delete h;

    h = new Date;
    h->from7BitString("Thu Mar 30 18:36:28 2006");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2006, 3, 30));
    QCOMPARE(h->dateTime().time(), QTime(18, 36, 28));
    QCOMPARE(h->dateTime().offsetFromUtc(), 0);
    delete h;

    // regression to KDE3; see bug 54098
    h = new Date;
    h->from7BitString("Tue, Feb 04, 2003 00:01:20 +0000");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2003, 2, 4));
    QCOMPARE(h->dateTime().time(), QTime(0, 1, 20));
    QCOMPARE(h->dateTime().offsetFromUtc(), 0);
    delete h;

    // date in a DST change to the future so in a time that doesn't exist
    // unless you take the timezone into account
    h = new Date;
    h->from7BitString("Sun, 31 Mar 2013 02:29:44 -0500");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2013, 3, 31));
    QCOMPARE(h->dateTime().time(), QTime(2, 29, 44));
    QCOMPARE(h->dateTime().offsetFromUtc(), -18000);
    delete h;

    // Bug Date: 13/10/20 12:51:47
    h = new Date;
    h->from7BitString("13/10/20 12:51:47");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2020, 10, 13));
    QCOMPARE(h->dateTime().time(), QTime(12, 51, 47));
    delete h;

    //28/09/23 16:05:54
    h = new Date;
    h->from7BitString("28/09/23 16:05:54");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2023, 9, 28));
    QCOMPARE(h->dateTime().time(), QTime(16, 05, 54));
    delete h;

    //28/09/23 (invalid input)
    h = new Date;
    h->from7BitString("28/09/23");
    QVERIFY(h->isEmpty());
    delete h;

    // Wed, 12 Apr 2030
    h = new Date;
    h->from7BitString("Wed, 12 Apr 2030");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->dateTime().date(), QDate(2030, 4, 12));
    QCOMPARE(h->dateTime().time(), QTime(0, 0, 0));
    delete h;
}

void HeaderTest::testLinesHeader()
{
    Lines *h;

    // empty header
    h = new Lines();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set some content
    h->setNumberOfLines(5);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("5"));
    delete h;

    // parse header with comment
    h = new Lines;
    h->from7BitString("(this is a comment) 10 (and yet another comment)");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->numberOfLines(), 10);
    delete h;
}

void HeaderTest::testNewsgroupsHeader()
{
    Newsgroups *h;

    // empty header
    h = new Newsgroups();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set newsgroups
    QList<QByteArray> groups;
    groups << "gmane.comp.kde.devel.core" << "gmane.comp.kde.devel.buildsystem";
    h->setGroups(groups);
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem"));
    delete h;

    // parse a header
    h = new Newsgroups;
    h->from7BitString("gmane.comp.kde.devel.core,gmane.comp.kde.devel.buildsystem");
    groups = h->groups();
    QCOMPARE(groups.count(), 2);
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core"));
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem"));
    delete h;

    // same again, this time with whitespaces and comments
    h = new Newsgroups();
    h->from7BitString("(comment) gmane.comp.kde.devel.core (second comment),\n gmane.comp.kde.devel.buildsystem (that all)");
    groups = h->groups();
    QCOMPARE(groups.count(), 2);
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.core"));
    QCOMPARE(groups.takeFirst(), QByteArray("gmane.comp.kde.devel.buildsystem"));
    delete h;
}

void HeaderTest::testControlHeader()
{
    Control *h;

    // empty header
    h = new Control();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    // set some content
    h->setCancel("<foo@bar>");
    QVERIFY(!h->isEmpty());
    QVERIFY(h->isCancel());
    QCOMPARE(h->as7BitString(),  QByteArray("cancel <foo@bar>"));
    delete h;

    // parse a control header
    h = new Control;
    h->from7BitString("cancel <foo@bar>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->parameter(), QByteArray("<foo@bar>"));
    QVERIFY(h->isCancel());
    QCOMPARE(h->controlType(), QByteArray("cancel"));
    delete h;
}

void HeaderTest::testReturnPath()
{
    ReturnPath *h;

    h = new ReturnPath();
    QVERIFY(h->isEmpty());
    QVERIFY(h->as7BitString().isEmpty());

    h->from7BitString("<foo@bar>");
    QVERIFY(!h->isEmpty());
    QCOMPARE(h->as7BitString(), QByteArray("<foo@bar>"));

    delete h;
}

void HeaderTest::noAbstractHeaders()
{
    From *h2 = new From(); delete h2;
    auto h3 = new Sender(); delete h3;
    To *h4 = new To(); delete h4;
    Cc *h5 = new Cc(); delete h5;
    Bcc *h6 = new Bcc(); delete h6;
    auto h7 = new ReplyTo(); delete h7;
    auto h8 = new Keywords(); delete h8;
    auto h9 = new MIMEVersion(); delete h9;
    auto h10 = new MessageID(); delete h10;
    auto h11 = new ContentID(); delete h11;
    auto h12 = new Supersedes(); delete h12;
    auto h13 = new InReplyTo(); delete h13;
    auto h14 = new References(); delete h14;
    auto h15 = new Generic(); delete h15;
    auto h16 = new Subject(); delete h16;
    auto h17 = new Organization(); delete h17;
    auto h18 = new ContentDescription(); delete h18;
    auto h22 = new FollowUpTo(); delete h22;
    auto h24 = new UserAgent(); delete h24;
}

void HeaderTest::testInvalidButOkQEncoding()
{
    // A stray '?' should not confuse the parser
    Subject subject;
    subject.from7BitString("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC?" "?=");
    QCOMPARE(subject.as7BitString(), QByteArray("Why? Why do some clients violate the RFC?"));
}

void HeaderTest::testInvalidQEncoding_data()
{
    QTest::addColumn<QString>("encodedWord");

    // All examples below should not be treated as invalid encoded strings, since the '?=' is missing
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC??");
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC?");
    QTest::newRow("") << QString::fromLatin1("=?us-ascii?q?Why?_Why_do_some_clients_violate_the_RFC");
}

void HeaderTest::testInvalidQEncoding()
{
    QFETCH(QString, encodedWord);

    QByteArray tmp = encodedWord.toLatin1() + " <noreply@kde.org>";
    Headers::From hdr;
    hdr.from7BitString(tmp);
    QCOMPARE(hdr.mailboxes().at(0).address(), "noreply@kde.org");
    QCOMPARE(hdr.mailboxes().at(0).name(), encodedWord); // invalid name is not decoded and preserved as-is
}

void HeaderTest::testMissingQuotes()
{
    QByteArray str = "multipart/signed; boundary=nextPart22807781.u8zn2zYrSU; micalg=pgp-sha1; protocol=application/pgp-signature";

    Headers::ContentType ct;
    ct.from7BitString(str);
    QCOMPARE(ct.mimeType(), QByteArray{ "multipart/signed" });
    QCOMPARE(ct.boundary(), QByteArray{ "nextPart22807781.u8zn2zYrSU" });
    QCOMPARE(ct.parameter("micalg"), QStringLiteral("pgp-sha1"));
    QCOMPARE(ct.parameter("protocol"), QStringLiteral("application/pgp-signature"));

}

void HeaderTest::testBug271192()
{
    QFETCH(QString, displayName);
    QFETCH(bool, quote);

    const QString addrSpec = QLatin1StringView("example@example.com");
    const QString mailbox =
        (quote ? QLatin1StringView("\"") : QString()) + displayName +
        (quote ? QLatin1StringView("\"") : QString()) +
        QLatin1StringView(" <") + addrSpec + QLatin1StringView(">");

    auto h = new Headers::Generics::SingleMailbox();
    h->fromUnicodeString(mailbox);
    QCOMPARE(h->mailbox().name(), displayName.remove(QLatin1StringView("\\")));
    delete h;
    h = nullptr;

    auto h2 = new Headers::Generics::MailboxList();
    h2->fromUnicodeString(mailbox + QLatin1StringView(",") + mailbox);
    QCOMPARE(h2->displayNames().size(), 2);
    QCOMPARE(h2->displayNames()[0].toUtf8(),
             displayName.remove(QLatin1StringView("\\")).toUtf8());
    QCOMPARE(h2->displayNames()[1].toUtf8(),
             displayName.remove(QLatin1StringView("\\")).toUtf8());
    delete h2;
    h2 = nullptr;
}

void HeaderTest::testBug271192_data()
{
    QTest::addColumn<QString>("displayName");
    QTest::addColumn<bool>("quote");

    QTest::newRow("Plain") << QString::fromUtf8("John Doe") << false;
    QTest::newRow("Firstname_1") << QString::fromUtf8("Marc-André Lastname") << false;
    QTest::newRow("Firstname_2") << QString::fromUtf8("Интернет-компания Lastname") << false;
    QTest::newRow("Lastname") << QString::fromUtf8("Tobias König") << false;
    QTest::newRow("Firstname_Lastname") << QString::fromUtf8("Интернет-компания König") << false;
    QTest::newRow("Quotemarks") << QString::fromUtf8(R"(John \"Rocky\" Doe)") << true;
    QTest::newRow("Quotemarks_nonascii") << QString::fromUtf8("Jöhn \\\"Röcky\\\" Döe") << true;

    QTest::newRow("quote_Plain") << QString::fromUtf8("John Doe") << true;
    QTest::newRow("quote_Firstname_1") << QString::fromUtf8("Marc-André Lastname") << true;
    QTest::newRow("quote_Firstname_2") << QString::fromUtf8("Интернет-компания Lastname") << true;
    QTest::newRow("quote_Lastname") << QString::fromUtf8("Tobias König") << true;
    QTest::newRow("quote_Firstname_Lastname") << QString::fromUtf8("Интернет-компания König") << true;
    QTest::newRow("quote_LastName_comma_Firstname") << QString::fromUtf8("König, Интернет-компания") << true;
}

void HeaderTest::testParseNextHeader()
{
    QByteArray data("From: konqi@kde.org\nTo: katie@kde.org\n\n");
    QByteArrayView dataView(data);

    auto header = KMime::HeaderParsing::parseNextHeader(dataView);
    QVERIFY(header);
    QCOMPARE(header->type(), "From");
    QVERIFY(dataView.startsWith("To:"));

    header = KMime::HeaderParsing::parseNextHeader(dataView);
    QVERIFY(header);
    QCOMPARE(header->type(), "To");

    QVERIFY(!KMime::HeaderParsing::parseNextHeader(dataView));
    QVERIFY(dataView.isEmpty());
}

#include "moc_headertest.cpp"
