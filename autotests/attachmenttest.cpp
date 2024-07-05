/*
    SPDX-FileCopyrightText: 2006-2015 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS

#include <QTest>

#include "attachmenttest.h"

#include "util.h"
#include "message.h"

using namespace KMime;

QTEST_MAIN(AttachmentTest)

void AttachmentTest::testIsAttachment_data()
{
    QTest::addColumn<QByteArray>("mimeType");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("isAtt");

    QTest::newRow("empty") << QByteArray() << QString() << QString() << false;
    QTest::newRow("text part") << QByteArray("text/plain") << QString() << QString() << false;
    QTest::newRow("text part w/ CT name") << QByteArray("text/plain") << QStringLiteral("file.txt") << QString() << true;
    QTest::newRow("text part w/ CD name") << QByteArray("text/plain") << QString() << QStringLiteral("file.txt") << true;

    // multipart is never an attachment, even with a CD name
    QTest::newRow("multipart/mixed") << QByteArray("multipart/mixed") << QString() << QStringLiteral("file.txt") << false;
    QTest::newRow("multipart/mixed upper") << QByteArray("MULTIPART/MIXED") << QString() << QStringLiteral("file.txt") << false;

    // emails are always attachments, even without CT/CD names
    QTest::newRow("message/rfc822") <<  QByteArray("message/rfc822") << QString() << QString() << true;

    // crypto nodes, even when looking like attachments, are not attachments
    QTest::newRow("crypto part") << QByteArray("application/octet-stream") << QString() << QStringLiteral("msg.asc") << false;
}

void AttachmentTest::testIsAttachment()
{
    QFETCH(QByteArray, mimeType);
    QFETCH(QString, name);
    QFETCH(QString, fileName);
    QFETCH(bool, isAtt);

    auto root = new KMime::Message;
    root->contentType()->from7BitString("multipart/mixed");
    root->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    auto c0 = new KMime::Content;
    root->appendContent(c0);
    auto c = new KMime::Content;
    root->appendContent(c);
    if (!mimeType.isEmpty()) {
        c->contentType()->setMimeType(mimeType);
    }
    if (!name.isEmpty()) {
        c->contentType()->setName(name);
    }
    if (!fileName.isEmpty()) {
        c->contentDisposition()->setFilename(fileName);
    }
    QCOMPARE(KMime::isAttachment(c), isAtt);
    QCOMPARE(KMime::hasAttachment(root), isAtt);

    if (isAtt) {
        QCOMPARE(root->attachments().size(), 1);
        QCOMPARE(root->attachments().at(0), c);
    } else {
        QVERIFY(root->attachments().isEmpty());
    }
    QVERIFY(c->attachments().isEmpty());

    delete root;
}

// stuff not covered above
void AttachmentTest::testIsAttachmentSpecial()
{
    // don't crash on invalid input
    QCOMPARE(KMime::isAttachment(nullptr), false);
    QCOMPARE(KMime::hasAttachment(nullptr), false);

    // disposition type "attachment" is a clear indicator...
    KMime::Content c;
    c.contentDisposition()->setDisposition(Headers::CDattachment);
    QCOMPARE(KMime::isAttachment(&c), true);
}

void AttachmentTest::testHasAttachment()
{
    // multipart/related is not an attachment
    auto root = new KMime::Message;
    root->contentType()->setMimeType("multipart/related");

    auto c1 = new KMime::Content;
    c1->contentType()->setMimeType("text/plain");
    root->appendContent(c1);

    auto c2 = new KMime::Content;
    c2->contentType()->setMimeType("image/jpeg");
    c2->contentDisposition()->setFilename(QStringLiteral("image.jpg"));
    root->appendContent(c2);

    QCOMPARE(KMime::hasAttachment(root), false);
    QCOMPARE(root->attachments().size(), 0);

    // just to make sure this actually works for non multipart/related
    QCOMPARE(KMime::isAttachment(c2), true);
    root->contentType()->setMimeType("multipart/mixed");
    QCOMPARE(KMime::hasAttachment(root), true);
    QCOMPARE(root->attachments().size(), 1);
    QCOMPARE(root->attachments().at(0), c2);
    delete root;

    // multipart/alternative is also not an attachment
    root = new KMime::Message;
    root->contentType()->setMimeType("multipart/alternative");

    c1 = new KMime::Content;
    c1->contentType()->setMimeType("text/plain");
    root->appendContent(c1);

    c2 = new KMime::Content;
    c2->contentType()->setMimeType("text/html");
    root->appendContent(c2);

    QCOMPARE(KMime::hasAttachment(root), false);
    QCOMPARE(root->attachments().size(), 0);
    delete root;

    // the main part of multipart/mixed is not an attachment, even if it looks like one
    root = new KMime::Message;
    root->contentType()->from7BitString("multipart/mixed");
    root->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    auto c0 = new KMime::Content;
    root->appendContent(c0);

    c1 = new KMime::Content;
    c1->contentType()->setMimeType("text/plain");
    c1->contentType()->setName(QStringLiteral("file.txt"));
    root->prependContent(c1);
    QCOMPARE(KMime::isAttachment(c1), false);
    QCOMPARE(KMime::hasAttachment(c1), false);
    QCOMPARE(KMime::hasAttachment(root), false);
    QCOMPARE(root->attachments().size(), 0);
    delete root;
}

void AttachmentTest::testNestedMultipart()
{
    auto root = new KMime::Message;
    root->contentType()->from7BitString("multipart/mixed");
    root->contentTransferEncoding()->setEncoding(KMime::Headers::CE7Bit);
    auto c0 = new KMime::Content;
    root->appendContent(c0);

    auto sig = new KMime::Content;
    sig->contentType()->setMimeType("application/pgp-signature");
    sig->contentType()->setName(QStringLiteral("signature.asc"));
    root->appendContent(sig);
    root->contentType()->setMimeType("multipart/signed");

    auto mixed = root->contents().at(0);

    auto att = new KMime::Content;
    att->contentType()->setMimeType("image/jpeg");
    att->contentType()->setName(QStringLiteral("attachment.jpg"));
    mixed->appendContent(att);

    mixed->contentType()->setMimeType("multipart/mixed");

    QVERIFY(KMime::hasAttachment(root));
    QVERIFY(KMime::hasAttachment(mixed));
    QCOMPARE(root->attachments().size(), 1);
    QCOMPARE(root->attachments().at(0), att);

    delete root;
}

void AttachmentTest::testEncrypted()
{
    auto root = new KMime::Message;
    root->setContent(
        "From: firstname.lastname@example.com\n"
        "To: test@kolab.org\n"
        "Subject: OpenPGP encrypted with 2 text attachments\n"
        "Date: Sun, 30 Aug 2015 12:05:17 +0200\n"
        "Message-ID: <1505824.VT0nqpAGu0@vkpc5>\n"
        "MIME-Version: 1.0\n"
        "Content-Type: multipart/encrypted; boundary=\"nextPart3335835.KxmPgziKxd\"; protocol=\"application/pgp-encrypted\"\n"
        "\n"
        "--nextPart3335835.KxmPgziKxd\n"
        "Content-Type: application/pgp-encrypted\n"
        "Content-Disposition: attachment\n"
        "Content-Transfer-Encoding: 7Bit\n"
        "\n"
        "Version: 1\n"
        "--nextPart3335835.KxmPgziKxd\n"
        "Content-Type: application/octet-stream\n"
        "Content-Disposition: inline; filename=\"msg.asc\"\n"
        "Content-Transfer-Encoding: 7Bit\n"
        "\n"
        "-----BEGIN PGP MESSAGE-----\n"
        "-----END PGP MESSAGE-----\n"
        "\n"
        "--nextPart3335835.KxmPgziKxd--\n"
    );
    root->parse();
    QCOMPARE(hasAttachment(root), false);
    delete root;
}

void AttachmentTest::testAttachment1()
{
    auto root = new KMime::Message;
    root->setContent("From: Sender <sender@test.org>\n"
                     "To: Receiver <receiver@test.org>\n"
                     "Subject: Test\n"
                     "Date: Thu, 19 Jul 2018 10:30:06 +0200\n"
                     "MIME-Version: 1.0\n"
                     "Content-Type: multipart/mixed; boundary=\"nextPart5103690.GVhdRC0Mqz\"\n"
                     "Content-Transfer-Encoding: 7Bit\n"
                     "\n"
                     "This is a multi-part message in MIME format.\n"
                     "\n"
                     "--nextPart5103690.GVhdRC0Mqz\n"
                     "Content-Transfer-Encoding: 7Bit\n"
                     "Content-Type: text/plain; charset=\"us-ascii\"\n"
                     "\n"
                     "Foo\n"
                     "\n"
                     "--nextPart5103690.GVhdRC0Mqz\n"
                     "Content-Disposition: attachment; filename=\"Screenshot_20180719_102529.png\"\n"
                     "Content-Transfer-Encoding: base64\n"
                     "Content-Type: image/png; name=\"Screenshot_20180719_102529.png\"\n"
                     "\n"
                     "ddd\n"
                     "\n"
                     "--nextPart5103690.GVhdRC0Mqz\n"
                     "Content-Disposition: attachment; filename=\"Screenshot_20180719_102550.png\"\n"
                     "Content-Transfer-Encoding: base64\n"
                     "Content-Type: image/png; name=\"Screenshot_20180719_102550.png\"\n"
                     "\n"
                     "zzzz\n"
                     "\n"
                     "--nextPart5103690.GVhdRC0Mqz--\n");

        root->parse();
        QCOMPARE(hasAttachment(root), true);
        delete root;
}

void AttachmentTest::testAttachment2()
{
    auto root = new KMime::Message;
    root->setContent("From: Sender <sender@test.org>\n"
                     "Content-Type: multipart/alternative; boundary=\"Apple-Mail=_627B41D2-E6ED-4B17-8F96-6CD63EC055AE\"\n"
                     "MIME-Version: 1.0\n"
                     "Subject: Test\n"
                     "Date: Tue, 11 Dec 2018 10:44:41 +0000\n"
                     "To: Receiver <receiver@test.org>\n"
                     "\n"
                     "\n"
                     "\n"
                     "--Apple-Mail=_627B41D2-E6ED-4B17-8F96-6CD63EC055AE\n"
                     "Content-Transfer-Encoding: quoted-printable\n"
                     "Content-Type: text/plain; charset=\"us-ascii\"\n"
                     "\n"
                     "Text blabla\n"
                     "\n"
                     "--Apple-Mail=_627B41D2-E6ED-4B17-8F96-6CD63EC055AE\n"
                     "Content-Type: multipart/mixed; boundary=\"Apple-Mail=_5FDAE280-1EA6-4604-9F81-BBB9B9137CE1\"\n"
                     "\n"
                     "\n"
                     "--Apple-Mail=_5FDAE280-1EA6-4604-9F81-BBB9B9137CE1\n"
                     "Content-Transfer-Encoding: 7bit\n"
                     "Content-Type: text/html; charset=\"us-ascii\"\n"
                     "\n"
                     "<html><head><body>foo</body></html>\n"
                     "--Apple-Mail=_5FDAE280-1EA6-4604-9F81-BBB9B9137CE1\n"
                     "Content-Disposition: inline; filename=\"bla.pdf\"\n"
                     "Content-Type: application/pdf; name=\"bla.pdf\"; x-unix-mode=\"0644\"\n"
                     "Content-Transfer-Encoding: base64\n"
                     "\n"
                     "bla\n"
                     "\n"
                     "--Apple-Mail=_5FDAE280-1EA6-4604-9F81-BBB9B9137CE1\n"
                     "Content-Transfer-Encoding: quoted-printable\n"
                     "Content-Type: text/html; charset=\"us-ascii\"\n"
                     "\n"
                     "<html><body>foo html</body></html>=\n"
                     "\n"
                     "--Apple-Mail=_5FDAE280-1EA6-4604-9F81-BBB9B9137CE1--\n"
                     "\n"
                     "--Apple-Mail=_627B41D2-E6ED-4B17-8F96-6CD63EC055AE--\n");

        root->parse();
        //Fix show has attachment
        QCOMPARE(hasAttachment(root), true);
        delete root;
}

#include "moc_attachmenttest.cpp"
