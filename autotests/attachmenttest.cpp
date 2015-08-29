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
        QCOMPARE(KMime::attachments(root).size(), 1);
        QCOMPARE(KMime::attachments(root).at(0), c);
    } else {
        QVERIFY(KMime::attachments(root).isEmpty());
    }
    QVERIFY(KMime::attachments(c).isEmpty());

    delete root;
}

// stuff not covered above
void AttachmentTest::testIsAttachmentSpecial()
{
    // don't crash on invalid input
    QCOMPARE(KMime::isAttachment(Q_NULLPTR), false);
    QCOMPARE(KMime::hasAttachment(Q_NULLPTR), false);

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
    QCOMPARE(KMime::attachments(root).size(), 0);

    // just to make sure this actually works for non multipart/related
    QCOMPARE(KMime::isAttachment(c2), true);
    root->contentType()->setMimeType("multipart/mixed");
    QCOMPARE(KMime::hasAttachment(root), true);
    QCOMPARE(KMime::attachments(root).size(), 1);
    QCOMPARE(KMime::attachments(root).at(0), c2);
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
    QCOMPARE(KMime::attachments(root).size(), 0);
    delete root;

    // the main part of multipart/mixed is not an attachment, even if it looks like one
    root = new KMime::Message;
    c1 = new KMime::Content;
    c1->contentType()->setMimeType("text/plain");
    c1->contentType()->setName(QStringLiteral("file.txt"), "utf-8");
    root->addContent(c1);
    QCOMPARE(KMime::isAttachment(c1), true);
    QEXPECT_FAIL("", "not implemented yet", Continue);
    QCOMPARE(KMime::hasAttachment(root), false);
    QEXPECT_FAIL("", "not implemented yet", Continue);
    QCOMPARE(KMime::attachments(root).size(), 0);
    delete root;
}
