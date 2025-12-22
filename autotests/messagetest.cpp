/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "messagetest.h"
#include <QTest>
#include <QDebug>
#include <QFile>
#include <codecs.cpp>

using namespace Qt::Literals;
using namespace KMime;

QTEST_MAIN(MessageTest)

void MessageTest::testMainBodyPart()
{
    auto msg = std::make_unique<Message>();
    auto msg2 = new Message();
    auto textPtr = std::make_unique<Content>();
    textPtr->contentType()->setMimeType("text/plain");
    const auto text = textPtr.get();
    auto htmlPtr = std::make_unique<Content>();
    htmlPtr->contentType()->setMimeType("text/html");
    const auto html = htmlPtr.get();

    // empty message
    QCOMPARE(msg->mainBodyPart(), msg.get());
    QCOMPARE(msg->mainBodyPart("text/plain"), (Content *)nullptr);

    // non-multipart
    msg->contentType()->setMimeType("text/html");

    QCOMPARE(msg->mainBodyPart(), msg.get());
    QCOMPARE(msg->mainBodyPart("text/plain"), (Content *)nullptr);
    QCOMPARE(msg->mainBodyPart("text/html"), msg.get());

    // multipart/mixed
    msg2->contentType()->setMimeType("multipart/mixed");
    msg2->appendContent(std::move(textPtr));
    msg2->appendContent(std::move(htmlPtr));

    QCOMPARE(msg2->mainBodyPart(), text);
    QCOMPARE(msg2->mainBodyPart("text/plain"), text);
    QCOMPARE(msg2->mainBodyPart("text/html"), (Content *)nullptr);

    textPtr = msg2->takeContent(text);
    htmlPtr = msg2->takeContent(html);

    // multipart/alternative
    msg->contentType()->setMimeType("multipart/alternative");
    msg->appendContent(std::move(htmlPtr));
    msg->appendContent(std::move(textPtr));

    QCOMPARE(msg->mainBodyPart(), html);
    QCOMPARE(msg->mainBodyPart("text/plain"), text);
    QCOMPARE(msg->mainBodyPart("text/html"), html);

    // multipart/alternative inside multipart/mixed
    auto msg3 = new Message();
    msg3->contentType()->setMimeType("multipart/mixed");
    msg3->appendContent(std::move(msg));
    auto attach = new Content();
    attach->contentType()->setMimeType("text/plain");

    QCOMPARE(msg3->mainBodyPart(), html);
    QCOMPARE(msg3->mainBodyPart("text/plain"), text);
    QCOMPARE(msg3->mainBodyPart("text/html"), html);

    delete msg2;
    delete msg3;
    delete attach;
}

void MessageTest::testBrunosMultiAssembleBug()
{
    QByteArray data =
        "From: Sender <sender@test.org>\n"
        "Subject: Sample message\n"
        "To: Receiver <receiver@test.org>\n"
        "Date: Sat, 04 Aug 2007 12:44:00 +0200\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain\n"
        "X-Foo: bla\n"
        "X-Bla: foo\n"
        "\n"
        "body";

    auto msg = std::make_unique<Message>();
    msg->setContent(data);
    msg->parse();
    msg->assemble();
    QCOMPARE(msg->encodedContent(), data);

    msg->inReplyTo();
    msg->assemble();
    QCOMPARE(msg->encodedContent(), data);

    const auto clone = msg->clone();
    QCOMPARE(clone->encodedContent(), data);
}

void MessageTest::testWillsAndTillsCrash()
{
    QByteArray deadlyMail = "From: censored@yahoogroups.com\n"
                            "To: censored@yahoogroups.com\n"
                            "Sender: censored@yahoogroups.com\n"
                            "MIME-Version: 1.0\n"
                            "Date: 29 Jan 2006 23:58:21 -0000\n"
                            "Subject: [censored] Birthday Reminder\n"
                            "Reply-To: censored@yahoogroups.com\n"
                            "Content-Type: multipart/alternative;\n boundary=\"YCalReminder=cNM4SNTGA4Cg1MVLaPpqNF1138579098\"\n"
                            "X-Length: 9594\n"
                            "X-UID: 6161\n"
                            "Status: RO\n"
                            "X-Status: OC\n"
                            "X-KMail-EncryptionState:\n"
                            "X-KMail-SignatureState:\n"
                            "X-KMail-MDN-Sent:\n\n";

    auto msg = new KMime::Message;
    msg->setContent(deadlyMail);
    msg->parse();
    QVERIFY(!msg->date()->isEmpty());
    QCOMPARE(msg->subject()->as7BitString(), QByteArray("[censored] Birthday Reminder"));
    QCOMPARE(msg->from()->mailboxes().count(), 1);
    QCOMPARE(msg->sender()->mailbox().address(), "censored@yahoogroups.com");
    QCOMPARE(msg->replyTo()->mailboxes().count(), 1);
    QCOMPARE(msg->to()->mailboxes().count(), 1);
    QCOMPARE(msg->cc()->mailboxes().count(), 0);
    QCOMPARE(msg->bcc()->mailboxes().count(), 0);
    QCOMPARE(msg->inReplyTo()->identifiers().count(), 0);
    QVERIFY(msg->messageID()->identifier().isEmpty());
    delete msg;
}

void MessageTest::testDavidsParseCrash()
{
    auto mail = readAndParseMail(QStringLiteral("dfaure-crash.mbox"));
    QCOMPARE(mail->to()->asUnicodeString().toLatin1(), "frank@domain.com");
}

void MessageTest::testHeaderFieldWithoutSpace()
{
    // Headers without a space, like the CC header here, are allowed according to
    // the examples in RFC2822, Appendix A5
    const QString mail = QStringLiteral("From:\n"
                                 "To: heinz@test.de\n"
                                 "Cc:moritz@test.de\n"
                                 "Subject: Test\n"
                                 "X-Mailer:");
    KMime::Message msg;
    msg.setContent(mail.toLatin1());
    msg.parse();

    QCOMPARE(msg.to()->asUnicodeString(), QLatin1StringView("heinz@test.de"));
    QCOMPARE(msg.from()->asUnicodeString(), QString());
    QCOMPARE(msg.cc()->asUnicodeString(), QLatin1StringView("moritz@test.de"));
    QCOMPARE(msg.subject()->asUnicodeString(), QLatin1StringView("Test"));
    QVERIFY(msg.hasHeader("X-Mailer"));
    QVERIFY(msg.headerByType("X-Mailer")->asUnicodeString().isEmpty());
}

void MessageTest::testWronglyFoldedHeaders()
{
    // The first subject line here doesn't contain anything. This is invalid,
    // however there are some mailers out there that produce those messages.
    const QString mail = QStringLiteral("Subject:\n"
                                 " Hello\n"
                                 " World\n"
                                 "To: \n"
                                 " test@test.de\n\n"
                                 "<Body>");
    KMime::Message msg;
    msg.setContent(mail.toLatin1());
    msg.parse();

    QCOMPARE(msg.subject()->asUnicodeString(),
             QLatin1StringView("Hello World"));
    QCOMPARE(msg.body(), "<Body>");
    QCOMPARE(msg.to()->asUnicodeString(), QLatin1StringView("test@test.de"));
}

void MessageTest::missingHeadersTest()
{
    // Test that the message body is OK even though some headers are missing
    KMime::Message msg;
    const QString body = QStringLiteral("Hi Donald, look at those nice pictures I found!\n");
    const QString content =
        QLatin1StringView("From: georgebush@whitehouse.org\n"
                          "To: donaldrumsfeld@whitehouse.org\n"
                          "Subject: Cute Kittens\n"
                          "\n") +
        body;
    msg.setContent(content.toLatin1());
    msg.parse();
    msg.assemble();

    QCOMPARE(body, QString::fromLatin1(msg.body()));

    // Now create a new message, based on the content of the first one.
    // The body of the new message should still be the same.
    // (there was a bug that caused missing mandatory headers to be
    //  added as an empty newline, which caused parts of the header to
    //  leak into the body)
    KMime::Message msg2;
    msg2.setContent(msg.encodedContent());
    msg2.parse();
    msg2.assemble();

    QCOMPARE(body, QString::fromLatin1(msg2.body()));
}

void MessageTest::testBug219749()
{
    // Test that the message body is OK even though some headers are missing
    KMime::Message msg;
    const QString content = QStringLiteral(
                                "Content-Type: MULTIPART/MIXED;\n"
                                " BOUNDARY=\"0-1804289383-1260384639=:52580\"\n"
                                "\n"
                                "--0-1804289383-1260384639=:52580\n"
                                "Content-Type: TEXT/plain; CHARSET=UTF-8\n"
                                "\n"
                                "--0-1804289383-1260384639=:52580\n"
                                "Content-Type: APPLICATION/octet-stream\n"
                                "Content-Transfer-Encoding: BASE64\n"
                                "Content-ID: <jaselka1.docx4AECA1F9@9230725.3CDBB752>\n"
                                "Content-Disposition: ATTACHMENT; FILENAME=\"jaselka 1.docx\"\n"
                                "\n"
                                "UEsDBBQABgAIAAAAIQDd/JU3ZgEAACAFAAATAAgCW0NvbnRlbnRfVHlwZXNd\n"
                                "SUwAAAAA\n"
                                "\n"
                                "--0-1804289383-1260384639=:52580--\n");

    msg.setContent(content.toLatin1());
    msg.parse();

    QCOMPARE(msg.contents().size(), 2);
    auto attachment = msg.contents()[1];
    QCOMPARE(attachment->contentType()->mediaType(), "application");
    QCOMPARE(attachment->contentType()->subType(), "octet-stream");
    QCOMPARE(attachment->contentID()->identifier(), "jaselka1.docx4AECA1F9@9230725.3CDBB752");
    QCOMPARE(attachment->contentID()->as7BitString(), "<jaselka1.docx4AECA1F9@9230725.3CDBB752>");
    Headers::ContentDisposition *cd = attachment->contentDisposition();
    QVERIFY(cd);
    QCOMPARE(cd->filename(), QLatin1StringView("jaselka 1.docx"));
}

void MessageTest::testBidiSpoofing()
{
    const QString RLO(QChar(0x202E));
    //const QString PDF( QChar( 0x202C ) );

    const QByteArray senderAndRLO =
        encodeRFC2047String(QString(QLatin1StringView("Sender") + RLO +
                                    QLatin1StringView(" <sender@test.org>")),
                            "utf-8");

    // The display name of the "From" has an RLO, make sure the KMime parser balances it
    QByteArray data =
        "From: " + senderAndRLO + "\n"
        "\n"
        "Body";

    KMime::Message msg;
    msg.setContent(data);
    msg.parse();

    // Test adjusted for taking into account that KMIME now removes bidi control chars
    // instead of adding PDF chars, because of broken KHTML.
    //const QString expectedDisplayName = "\"Sender" + RLO + PDF + "\"";
    const QString expectedDisplayName = QStringLiteral("Sender");
    const QString expectedMailbox =
        expectedDisplayName + QLatin1StringView(" <sender@test.org>");
    QCOMPARE(msg.from()->addresses().count(), 1);
    QCOMPARE(msg.from()->asUnicodeString(), expectedMailbox);
    QCOMPARE(msg.from()->displayNames().first(), expectedDisplayName);
    QCOMPARE(msg.from()->mailboxes().first().name(), expectedDisplayName);
    QCOMPARE(msg.from()->mailboxes().first().address(), "sender@test.org");
}

// Test to see if header fields of mails with an UTF-16 body are properly read
// and written.
// See also https://issues.kolab.org/issue3707
void MessageTest::testUtf16()
{
    QByteArray data =
        "From: foo@bar.com\n"
        "Subject: UTF-16 Test\n"
        "MIME-Version: 1.0\n"
        "Content-Type: Text/Plain;\n"
        "  charset=\"utf-16\"\n"
        "Content-Transfer-Encoding: base64\n"
        "\n"
        "//5UAGgAaQBzACAAaQBzACAAVQBUAEYALQAxADYAIABUAGUAeAB0AC4ACgAKAAo";

    KMime::Message msg;
    msg.setContent(data);
    msg.parse();

    QCOMPARE(msg.from()->asUnicodeString(), QLatin1StringView("foo@bar.com"));
    QCOMPARE(msg.subject()->asUnicodeString(),
             QLatin1StringView("UTF-16 Test"));
    QCOMPARE(msg.decodedText(Content::TrimNewlines),
             QLatin1StringView("This is UTF-16 Text."));

    // Add a new To header, for testings
    auto to = std::make_unique<KMime::Headers::To>();
    KMime::Types::Mailbox address;
    address.setAddress("test@test.de");
    address.setName(QStringLiteral("Fränz Töster"));
    to->addAddress(address);
    to->setRFC2047Charset("ISO-8859-1"); // default changed to UTF-8 in KF6, which is fine, but breaks the test
    msg.appendHeader(std::move(to));
    msg.assemble();

    QByteArray newData =
        "From: foo@bar.com\n"
        "Subject: UTF-16 Test\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain; charset=\"utf-16\"\n"
        "Content-Transfer-Encoding: base64\n"
        "To: =?ISO-8859-1?Q?Fr=E4nz_T=F6ster?= <test@test.de>\n"
        "\n"
        "//5UAGgAaQBzACAAaQBzACAAVQBUAEYALQAxADYAIABUAGUAeAB0AC4ACgAKAAo=\n"
        "\n";

    QCOMPARE(msg.encodedContent(), newData);
}

void MessageTest::testDecodedText()
{
    QByteArray data =
        "Subject: Test\n"
        "\n"
        "Testing Whitespace   \n  \n \n\n\n";

    KMime::Message msg;
    msg.setContent(data);
    msg.parse();

    QCOMPARE(msg.decodedText(Content::TrimSpaces),
             QLatin1StringView("Testing Whitespace"));
    QCOMPARE(msg.decodedText(Content::TrimNewlines),
             QLatin1StringView("Testing Whitespace   \n  \n "));

    QByteArray data2 =
        "Subject: Test\n"
        "\n"
        "Testing Whitespace   \n  \n \n\n\n ";

    KMime::Message msg2;
    msg2.setContent(data2);
    msg2.parse();

    QCOMPARE(msg2.decodedText(Content::TrimSpaces),
             QLatin1StringView("Testing Whitespace"));
    QCOMPARE(msg2.decodedText(Content::TrimNewlines),
             QLatin1StringView("Testing Whitespace   \n  \n \n\n\n "));
}

void MessageTest::testInlineImages()
{
    const QByteArray data =
        "From: <kde@kde.org>\n"
        "To: kde@kde.org\n"
        "Subject: Inline Image (unsigned)\n"
        "Date: Wed, 23 Dec 2009 14:00:59 +0100\n"
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/related;\n"
        "  boundary=\"Boundary-02=_LShMLJyjC7zqmVP\"\n"
        "Content-Transfer-Encoding: 7bit\n"
        "\n"
        "\n"
        "--Boundary-02=_LShMLJyjC7zqmVP\n"
        "Content-Type: multipart/alternative;\n"
        "  boundary=\"Boundary-01=_LShMLzAUPqE38S8\"\n"
        "Content-Transfer-Encoding: 7bit\n"
        "Content-Disposition: inline\n"
        "\n"
        "--Boundary-01=_LShMLzAUPqE38S8\n"
        "Content-Type: text/plain;\n"
        "  charset=\"us-ascii\"\n"
        "Content-Transfer-Encoding: 7bit\n"
        "\n"
        "First line\n"
        "\n"
        "\n"
        "Image above\n"
        "\n"
        "Last line\n"
        "\n"
        "--Boundary-01=_LShMLzAUPqE38S8\n"
        "Content-Type: text/html;\n"
        "  charset=\"us-ascii\"\n"
        "Content-Transfer-Encoding: 7bit\n"
        "\n"
        "Line 1\n"
        "--Boundary-01=_LShMLzAUPqE38S8--\n"
        "\n"
        "--Boundary-02=_LShMLJyjC7zqmVP\n"
        "Content-Type: image/png;\n"
        "  name=\"inlineimage.png\"\n"
        "Content-Transfer-Encoding: base64\n"
        "Content-Id: <740439759>\n"
        "\n"
        "jxrG/ha/VB+rODav6/d5i1US6Za/YEMvtm2SgJC/CXVFiD3UFSH2UFeE2ENdEWIPdUWIPdQVIfZQ\n"
        "V4TYQ10RYg91RYg91BUh9lBXhNhDXRFiD3VFiD3UFSH2UFeE2ENdEWIPdUWIPdQVIfZQV4TYQ10R\n"
        "Yg91RYg91BUh9lBX5E+Tz6Vty1HSx+NR++UuCOqKEHv+Ax0Y5U59+AHBAAAAAElFTkSuQmCC\n"
        "\n"
        "--Boundary-02=_LShMLJyjC7zqmVP--";

    KMime::Message msg;
    msg.setContent(data);
    msg.parse();

    QCOMPARE(msg.contents().size(), 2);
    QCOMPARE(msg.contents()[0]->contentType()->isMultipart(), true);
    QCOMPARE(msg.contents()[0]->contentType()->subType(), "alternative");

    QCOMPARE(msg.contents()[1]->contentType()->isImage(), true);
    QCOMPARE(msg.contents()[1]->contentType()->name(),
             QLatin1StringView("inlineimage.png"));
    QCOMPARE(msg.contents()[1]->contentID()->identifier(), "740439759");
    QCOMPARE(msg.contents()[1]->contentID()->as7BitString(), "<740439759>");

    const auto clone = msg.clone();
    QCOMPARE(msg.contents().size(), 2);
    QCOMPARE(msg.encodedContent(), clone->encodedContent());
}

void MessageTest::testIssue3908()
{
    auto msg = readAndParseMail(QStringLiteral("issue3908.mbox"));
    QCOMPARE(msg->contents().size(), 2);
    auto attachment = msg->contents()[1];
    QVERIFY(attachment);
    QVERIFY(attachment->contentDescription());
    QCOMPARE(attachment->contentDescription()->asUnicodeString(), QString::fromUtf8(
                 "Kontact oder auch KDE-PIM ist der Groupware-Client aus der KDE Software Compilation 4.Eine der Besonderheiten von Kontact "
                 "gegenüber anderen Groupware-Clients ist, dass die Teil-Programme auch weiterhin unabhängig von Kontact gestartet werden "
                 "können. So spielt es zum Beispiel keine Rolle für das Arbeiten mit KMail, ob es mal allein oder mal im Rahmen von Kontact "
                 "gestartet wird: Die Mails und die persönlichen Einstellungen bleiben stets erhalten.Auch sieht Kontact eine modulare "
                 "Anbindung der Programme vor, wodurch sich auch in Zukunft weitere Module entwickeln und anfügen lassen, ohne Kontact "
                 "dafür zu ändern. Dies bietet die Möglichkeit, auch privat entwickelte Module einzubinden und so die Groupware grundlegend "
                 "eigenen Bedürfnissen anzupassen."));
}

void MessageTest::testIssue3914()
{
    // This loads a mail which has a content-disposition of which the filename parameter is empty.
    // Check that the parser doesn't choke on this.
    auto msg = readAndParseMail(QStringLiteral("broken-content-disposition.mbox"));

    QCOMPARE(msg->subject()->as7BitString(), "Fwd: test broken mail");
    QCOMPARE(msg->contents().size(), 2);
    auto attachedMail =  msg->contents()[1];
    QCOMPARE(attachedMail->contentType()->mimeType(), "message/rfc822");
    QVERIFY(attachedMail->contentDisposition());
    QVERIFY(attachedMail->contentDisposition()->hasParameter("filename"));
    QVERIFY(attachedMail->contentDisposition()->parameter("filename").isEmpty());
}

void MessageTest::testBug223509()
{
    auto msg = readAndParseMail(QStringLiteral("encoding-crash.mbox"));

    QCOMPARE(msg->subject()->as7BitString(), "Blub");
    QCOMPARE(msg->contents().size(), 0);
    QCOMPARE(msg->contentTransferEncoding()->encoding(), KMime::Headers::CEbinary);
    QCOMPARE(msg->decodedText().toLatin1(), "Bla Bla Bla");
    QCOMPARE(msg->encodedBody(), "Bla Bla Bla\n");

    // encodedContent() was crashing in this bug because of an invalid assert
    QVERIFY(!msg->encodedContent().isEmpty());

    // Make sure that the encodedContent() is sane, by parsing it again.
    KMime::Message msg2;
    msg2.setContent(msg->encodedContent());
    msg2.parse();
    QCOMPARE(msg2.subject()->as7BitString(), "Blub");
    QCOMPARE(msg2.contents().size(), 0);
    QCOMPARE(msg2.contentTransferEncoding()->encoding(), KMime::Headers::CEbinary);

    QCOMPARE(msg2.decodedText().toLatin1(), "Bla Bla Bla");
    QCOMPARE(msg2.decodedText(Content::TrimSpaces).toLatin1(),
             "Bla Bla Bla");
}

void MessageTest::testEncapsulatedMessages()
{
    //
    // First, test some basic properties to check that the parsing was correct
    //
    auto msg = readAndParseMailMut(QStringLiteral("simple-encapsulated.mbox"));
    QCOMPARE(msg->contentType()->mimeType(), "multipart/mixed");
    QCOMPARE(msg->contents().size(), 2);
    QVERIFY(msg->isTopLevel());

    auto const textContent = msg->contents()[0];
    QCOMPARE(textContent->contentType()->mimeType(), "text/plain");
    QVERIFY(textContent->contents().isEmpty());
    QVERIFY(!textContent->bodyIsMessage());
    QVERIFY(!textContent->bodyAsMessage());
    QVERIFY(!textContent->isTopLevel());
    QCOMPARE(
        textContent->decodedText(Content::TrimSpaces),
        QLatin1StringView(
            "Hi Hans!\nLook at this interesting mail I forwarded to you!"));
    QCOMPARE(textContent->index().toString().toLatin1(), "1");

    auto messageContent = msg->contents()[1];
    QCOMPARE(messageContent->contentType()->mimeType(), "message/rfc822");
    QVERIFY(messageContent->body().isEmpty());
    QCOMPARE(messageContent->contents().count(), 1);
    QVERIFY(messageContent->bodyIsMessage());
    QVERIFY(messageContent->bodyAsMessage());
    QVERIFY(!messageContent->isTopLevel());
    QCOMPARE(messageContent->index().toString().toLatin1(), "2");

    std::shared_ptr<KMime::Message> encapsulated = messageContent->bodyAsMessage();
    QCOMPARE(encapsulated->contents().size(), 0);
    QCOMPARE(encapsulated->contentType()->mimeType(), "text/plain");
    QVERIFY(!encapsulated->bodyIsMessage());
    QVERIFY(!encapsulated->bodyAsMessage());
    QCOMPARE(encapsulated->subject()->as7BitString(), "Foo");
    QCOMPARE(encapsulated->decodedText(Content::NoTrim),
             QLatin1StringView("This is the encapsulated message body."));
    QCOMPARE(encapsulated.get(), messageContent->bodyAsMessage().get());
    QCOMPARE(encapsulated.get(), messageContent->contents().constFirst());
    QCOMPARE(encapsulated->parent(), messageContent);
    QVERIFY(!encapsulated->isTopLevel());
    QCOMPARE(encapsulated->topLevel(), msg.get());
    QCOMPARE(encapsulated->index().toString().toLatin1(), "2.1");

    // Now test some misc functions
    QCOMPARE(msg->storageSize(), msg->head().size() + textContent->storageSize() +
             messageContent->storageSize());
    QCOMPARE(messageContent->storageSize(), messageContent->head().size() +
             encapsulated->storageSize());

    // Now change some properties on the encapsulated message
    encapsulated->subject()->fromUnicodeString(QStringLiteral("New subject"));
    encapsulated->fromUnicodeString(QStringLiteral("New body string."));

    // Since we didn't assemble the encapsulated message yet, it should still have the old headers
    QVERIFY(encapsulated->encodedContent().contains("Foo"));
    QVERIFY(!encapsulated->encodedContent().contains("New subject"));

    // Now assemble the container message
    msg->assemble();

    // Assembling the container message should have assembled the encapsulated message as well.
    QVERIFY(!encapsulated->encodedContent().contains("Foo"));
    QVERIFY(encapsulated->encodedContent().contains("New subject"));
    QCOMPARE(encapsulated->body(), "New body string.");
    QVERIFY(msg->encodedContent().contains(encapsulated->body()));
    QCOMPARE(msg->contentType()->mimeType(), "multipart/mixed");
    QCOMPARE(msg->contents().size(), 2);
    messageContent = msg->contents()[1];
    QCOMPARE(messageContent->contentType()->mimeType(), "message/rfc822");
    QVERIFY(encapsulated == messageContent->bodyAsMessage());

    // Setting a new body and then parsing it should discard the encapsulated message
    messageContent->contentType()->setMimeType("text/plain");
    messageContent->assemble();
    messageContent->setBody("Some new body");
    messageContent->parse();
    QVERIFY(!messageContent->bodyIsMessage());
    QVERIFY(!messageContent->bodyAsMessage());
    QCOMPARE(messageContent->contents().size(), 0);
}

void MessageTest::testOutlookAttachmentNaming()
{
    // Try and decode
    auto msg = readAndParseMailMut(QStringLiteral("outlook-attachment.mbox"));
    QVERIFY(msg->attachments().size() == 1);

    auto attachment = msg->contents()[1];
    QCOMPARE(attachment->contentType(DontCreate)->mediaType(), "text");
    QCOMPARE(attachment->contentType(DontCreate)->subType(), "x-patch");

    Headers::ContentDisposition *cd = attachment->contentDisposition(DontCreate);
    QVERIFY(cd);
    QCOMPARE(cd->filename(), QString::fromUtf8("å.diff"));

    // Try and encode
    attachment->clear();// = new Content();
    attachment->contentDisposition()->setDisposition(Headers::CDattachment);
    attachment->contentDisposition()->setFilename(QStringLiteral("å.diff"));
    attachment->contentDisposition()->setRFC2047Charset("ISO-8859-1"); // default changed to UTF-8 in KF6, which is fine, but breaks the test
    attachment->assemble();
    qDebug() << "got:" << attachment->contentDisposition()->as7BitString();
    QCOMPARE(attachment->contentDisposition()->as7BitString(), QByteArray("attachment; filename*=ISO-8859-1''%E5%2Ediff"));
}

void MessageTest::testEncryptedMails()
{
    auto msg = readAndParseMail(QStringLiteral("x-pkcs7.mbox"));
    QVERIFY(msg->contents().size() == 0);
    QVERIFY(msg->attachments().size() == 0);
    QVERIFY(KMime::isEncrypted(msg.get()) == true);
    QVERIFY(KMime::isInvitation(msg.get()) == false);
    QVERIFY(KMime::isSigned(msg.get()) == false);
}

void MessageTest::testReturnSameMail()
{
    auto msg = readAndParseMail(QStringLiteral("dontchangemail.mbox"));
    QFile file(QLatin1StringView(TEST_DATA_DIR) + QLatin1StringView("/dontchangemail.mbox"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray fileContent = file.readAll();
    QCOMPARE(msg->encodedContent(), fileContent);
    QCOMPARE(msg->decodedText(), QLatin1StringView(""));
    KMime::Message msg2;
    msg2.setContent(msg->encodedContent());
    msg2.parse();
    QCOMPARE(msg2.encodedContent(), fileContent);
}

void MessageTest::testEmptySubject()
{
    auto msg = readAndParseMail(QStringLiteral("empty-subject.mbox"));
    QVERIFY(msg); // was crashing for Andre
    QVERIFY(msg->hasHeader("Subject"));
    QVERIFY(msg->subject()->asUnicodeString().isEmpty());
}

void MessageTest::testReplyHeader()
{
    auto msg = readAndParseMail(QStringLiteral("reply-header.mbox"));
    QVERIFY(msg);
    QVERIFY(!msg->replyTo());
    QCOMPARE(msg->hasHeader("Reply-To"), false);
    QCOMPARE(msg->hasHeader("Reply"), true);
    QVERIFY(msg->headerByType("Reply"));
}

std::unique_ptr<KMime::Message> MessageTest::readAndParseMailMut(const QString &mailFile) const
{
    QFile file(QLatin1StringView(TEST_DATA_DIR) + QLatin1StringView("/") + mailFile);
    const bool ok = file.open(QIODevice::ReadOnly);
    if (!ok) {
        qWarning() << file.fileName() << "not found";
    }
    Q_ASSERT(ok);
    const QByteArray data = KMime::CRLFtoLF(file.readAll());
    Q_ASSERT(!data.isEmpty());
    auto msg = std::make_unique<KMime::Message>();
    msg->setContent(data);
    msg->parse();
    return msg;
}

std::unique_ptr<const KMime::Message> MessageTest::readAndParseMail(const QString &mailFile) const
{
    return readAndParseMailMut(mailFile);
}

void MessageTest::testBug392239()
{
    auto msg = readAndParseMail(QStringLiteral("bug392239.mbox"));

    QCOMPARE(msg->subject()->as7BitString(), QByteArray());
    QCOMPARE(msg->contents().size(), 0);
    QCOMPARE(msg->contentTransferEncoding()->encoding(), KMime::Headers::CEbase64);
    QCOMPARE(msg->decodedText().toUtf8(), "Following this paragraph there is a double line break which should result in vertical spacing.\r\rPreceding this paragraph there is a double line break which should result in vertical spacing.\r");
}

void MessageTest::testBugAttachment387423()
{
    auto msg = readAndParseMail(QStringLiteral("kmail-attachmentstatus.mbox"));

    QCOMPARE(msg->subject()->as7BitString(), "XXXXXXXXXXXXXXXXXXXXX");
    QEXPECT_FAIL("", "Problem with searching attachment", Continue);
    QCOMPARE(msg->attachments().size(), 1);
    QCOMPARE(msg->contents().size(), 2);

    auto attachment = msg->contents()[1];
    QCOMPARE(attachment->contentType()->mediaType(), "image");
    QCOMPARE(attachment->contentType()->subType(), "gif");
    QCOMPARE(attachment->contentType()->subType(), "gif");
    QCOMPARE(attachment->contentDisposition()->filename(), QStringLiteral("new.gif"));
    QCOMPARE(attachment->contentDisposition()->disposition(), Headers::CDattachment);
}

void MessageTest::testCrashReplyInvalidEmail()
{
    auto msg = readAndParseMail(QStringLiteral("crash-invalid-email-reply.mbox"));
    QCOMPARE(msg->subject()->as7BitString(), "Re: Authorization required to post to gmane.network.wireguard (b96565298414a43aabcf9fbedf5e7e27)");
    QCOMPARE(msg->contentType()->mimeType(), "text/plain");
    QCOMPARE(msg->contentType()->charset(), "us-ascii");
    QVERIFY(msg->isTopLevel());
}

void MessageTest::testHeadersWithNullBytes()
{
    auto msg = readAndParseMail(QStringLiteral("headers-with-nullbytes.mbox"));
    QCOMPARE(msg->subject()->as7BitString(), "This header type has a trailing null byte");
    QCOMPARE(msg->headerByType("SubjectInvalid")->as7BitString(), "This header type contains a null byte");
}

void MessageTest::testBigAllocation()
{
    auto msg = readAndParseMail(QStringLiteral("big-allocation.mbox"));
    QCOMPARE(msg->contents().size(), 20);
    for (const auto &part : msg->contents()) {
        QVERIFY(part->contents().empty());
    }
}

void MessageTest::testGarbage_data()
{
    QTest::addColumn<QString>("filename");
    QTest::newRow("multipart-parse-abort-1") << u"multipart-parse-abort.mbox"_s;
    QTest::newRow("multipart-parse-abort-2") << u"multipart-parse-abort-2.mbox"_s;
    QTest::newRow("digits-overflow") << u"read-digits-overflow.mbox"_s;
    QTest::newRow("uninitialized-memory") << u"uninitialized-memory-use.mbox"_s;
    QTest::newRow("infinite-memory") << u"clusterfuzz-testcase-minimized-kmime_fuzzer-5255984894509056"_s;
    QTest::newRow("assert") << u"clusterfuzz-testcase-minimized-kmime_fuzzer-5617955779182592"_s;
    QTest::newRow("yenc-large-alloc") << u"clusterfuzz-testcase-minimized-kmime_fuzzer-4804196479336448"_s;
    QTest::newRow("yenc-corrupt-size") << u"yenc-single-part.yenc"_s;
    QTest::newRow("uuencode-no-filename") << u"clusterfuzz-testcase-minimized-kmime_fuzzer-6349101081100288"_s;
}

void MessageTest::testGarbage()
{
    // all this does is to ensure parsing the input file doesn't crash, trigger ASAN or infinitely loop
    QFETCH(QString, filename);
    auto msg = readAndParseMail(filename);
    QVERIFY(msg);
}

void MessageTest::testYenc()
{
    auto msg = readAndParseMail(u"yenc-single-part.yenc"_s);
    QVERIFY(msg);

    QFile refFile(QLatin1StringView(TEST_DATA_DIR) + "/yenc-single-part.txt"_L1);
    QVERIFY(refFile.open(QFile::ReadOnly));
    QCOMPARE(msg->subject()->asUnicodeString(), "yEnc-Prefix: \"testfile.txt\" 584 yEnc bytes - yEnc test (1)"_L1);
    QCOMPARE(msg->contents().size(), 2);
    QCOMPARE(msg->contents()[1]->decodedBody(), refFile.readAll());
}

#include "moc_messagetest.cpp"
