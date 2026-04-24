/*
    SPDX-FileCopyrightText: 2026 Arnt Gulbrandsen

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* This file is based on https://github.com/arnt/eai-test-messages,
   just mapped to C++. I really don't think there's enough creativity
   here to claim any rights. The SPDX comment is purely formal.
*/

#include "eaitest.h"

#include <QFile>
#include <QTest>

#include <KMime/Message>

using namespace Qt::Literals::StringLiterals;

QTEST_MAIN(EaiTest)

std::unique_ptr<KMime::Message>
EaiTest::readAndParseMail(const QString &mailFile) const {
  QFile file(QLatin1StringView(TEST_DATA_DIR) + QLatin1StringView("/") +
             mailFile);
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

// The 'addresses' message has Unicode addresses in From and Cc.
// Signed-Off-By looks like an address field but is not a standard header.
void EaiTest::testAddresses() {
  auto msg = readAndParseMail(QStringLiteral("eai-addresses.mbox"));

  QCOMPARE(msg->from()->mailboxes().count(), 1);
  QCOMPARE(msg->from()->mailboxes().first().name(),
           QStringLiteral("Jøran Øygårdvær"));
  QCOMPARE(msg->from()->mailboxes().first().address(),
           QByteArray("jøran@example.com"));

  QCOMPARE(msg->cc()->mailboxes().count(), 1);
  QCOMPARE(msg->cc()->mailboxes().first().name(),
           QStringLiteral("Jøran Øygårdvær"));
  QCOMPARE(msg->cc()->mailboxes().first().address(),
           QByteArray("jøran@example.com"));

  QCOMPARE(msg->to()->mailboxes().count(), 1);
  QCOMPARE(msg->to()->mailboxes().first().address(),
           QByteArray("arnt@example.com"));
}

// The 'from' message has a single Unicode From address.
void EaiTest::testFrom() {
  auto msg = readAndParseMail(QStringLiteral("eai-from.mbox"));

  QCOMPARE(msg->from()->mailboxes().count(), 1);
  QCOMPARE(msg->from()->mailboxes().first().name(),
           QStringLiteral("Jøran Øygårdvær"));
  QCOMPARE(msg->from()->mailboxes().first().address(),
           QByteArray("jøran@example.com"));
}

// The 'mimefield' message has a Content-Disposition with a UTF-8 filename.
void EaiTest::testMimefield() {
  auto msg = readAndParseMail(QStringLiteral("eai-mimefield.mbox"));

  QCOMPARE(msg->contentDisposition()->disposition(),
           KMime::Headers::CDattachment);
  QCOMPARE(msg->contentDisposition()->filename(),
           QStringLiteral("blåbærsyltetøy"));
}

// The 'not-emoji' message has From: xn--ls8ha@outlook.com.
// The localpart starts with xn-- but is NOT a punycode-encoded emoji;
// localparts are never decoded.
void EaiTest::testNotEmoji() {
  auto msg = readAndParseMail(QStringLiteral("eai-not-emoji.mbox"));

  QCOMPARE(msg->from()->mailboxes().count(), 1);
  QCOMPARE(msg->from()->mailboxes().first().address(),
           QByteArray("xn--ls8ha@outlook.com"));
}

// The 'punycode' message has an ASCII localpart with a punycode domain in From,
// and a UTF-8 localpart with a punycode domain in To.
void EaiTest::testPunycode() {
  auto msg = readAndParseMail(QStringLiteral("eai-punycode.mbox"));

  QCOMPARE(msg->from()->mailboxes().count(), 1);
  QCOMPARE(msg->from()->mailboxes().first().address(),
           QByteArray("info@xn--dmi-0na.fo"));

  QCOMPARE(msg->to()->mailboxes().count(), 1);
  QCOMPARE(msg->to()->mailboxes().first().address(),
           QByteArray("dømi@xn--dmi-0na.fo"));
}

// The 'attachment' message is a multipart/mixed with a plain text part
// and a JPEG attachment with a UTF-8 filename.
// A From header with raw UTF-8 bytes in both localpart and domain, as used in
// SMTPUTF8/EAI (RFC 6531/6532). parseDotAtom must accept 8-bit bytes and
// parseDomain must decode them as UTF-8 (not Latin-1).
void EaiTest::testUtf8Domain() {
  auto msg = std::make_unique<KMime::Message>();
  msg->setContent(QByteArray(
      "From: =?utf-8?q?Gr=C3=A5_katt?= <gr\xC3\xA5@gr\xC3\xA5.org>\r\n"
      "To: test@example.com\r\n"
      "Subject: Test\r\n"
      "MIME-Version: 1.0\r\n"
      "Content-Type: text/plain; charset=utf-8\r\n"
      "\r\n"
      "Body\r\n"));
  msg->parse();

  QVERIFY(!msg->from()->mailboxes().isEmpty());
  QCOMPARE(msg->from()->mailboxes().first().name(), u"Grå katt"_s);
  QCOMPARE(msg->from()->mailboxes().first().address(),
           QByteArray("gr\xC3\xA5@gr\xC3\xA5.org"));
}

void EaiTest::testAttachment() {
  auto msg = readAndParseMail(QStringLiteral("eai-attachment.mbox"));

  QCOMPARE(msg->contentType()->mimeType(), QByteArray("multipart/mixed"));
  QCOMPARE(msg->contents().count(), 2);

  KMime::Content *attachment = msg->contents().at(1);
  QCOMPARE(attachment->contentType()->mimeType(), QByteArray("image/png"));
  QCOMPARE(attachment->contentDisposition()->disposition(),
           KMime::Headers::CDattachment);
  QCOMPARE(attachment->contentDisposition()->filename(),
           QStringLiteral("blåbærsyltetøy"));
}
