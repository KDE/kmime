/*
    Copyright (c) 2006-2015 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS

#include <qtest.h>

#include "attachmenttest.h"

#include <kmime_util.h>
#include <kmime_message.h>

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
    auto c = new KMime::Content;
    root->addContent(c);
    if (!mimeType.isEmpty())
        c->contentType()->setMimeType(mimeType);
    if (!name.isEmpty())
        c->contentType()->setName(name, "utf-8");
    if (!fileName.isEmpty())
        c->contentDisposition()->setFilename(fileName);
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
    root->addContent(c1);

    auto c2 = new KMime::Content;
    c2->contentType()->setMimeType("image/jpeg");
    c2->contentDisposition()->setFilename(QStringLiteral("image.jpg"));
    root->addContent(c2);

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
    root->addContent(c1);

    c2 = new KMime::Content;
    c2->contentType()->setMimeType("text/html");
    root->addContent(c2);

    QCOMPARE(KMime::hasAttachment(root), false);
    QCOMPARE(root->attachments().size(), 0);
    delete root;

    // the main part of multipart/mixed is not an attachment, even if it looks like one
    root = new KMime::Message;
    c1 = new KMime::Content;
    c1->contentType()->setMimeType("text/plain");
    c1->contentType()->setName(QStringLiteral("file.txt"), "utf-8");
    root->addContent(c1, true);
    QCOMPARE(KMime::isAttachment(c1), false);
    QCOMPARE(KMime::hasAttachment(c1), false);
    QCOMPARE(KMime::hasAttachment(root), false);
    QCOMPARE(root->attachments().size(), 0);
    delete root;
}

void AttachmentTest::testNestedMultipart()
{
    auto root = new KMime::Message;
    auto sig = new KMime::Content;
    sig->contentType()->setMimeType("application/pgp-signature");
    sig->contentType()->setName(QStringLiteral("signature.asc"), "utf-8");
    root->addContent(sig);
    root->contentType()->setMimeType("multipart/signed");

    auto mixed = root->contents().at(0);

    auto att = new KMime::Content;
    att->contentType()->setMimeType("image/jpeg");
    att->contentType()->setName(QStringLiteral("attachment.jpg"), "utf-8");
    mixed->addContent(att);

    mixed->contentType("multipart/mixed");

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
