/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "utiltest.h"

#include <kmime_util.h>
#include <kmime_message.h>

using namespace KMime;

QTEST_MAIN(UtilTest)

void UtilTest::testUnfoldHeader()
{
    // empty header
    QCOMPARE(KMime::unfoldHeader(""), QByteArray());
    // identity
    QCOMPARE(KMime::unfoldHeader("bla"), QByteArray("bla"));
    // single folding
    QCOMPARE(KMime::unfoldHeader("bla\nblub"), QByteArray("bla blub"));
    QCOMPARE(KMime::unfoldHeader("bla\n \t blub"), QByteArray("bla blub"));
    QCOMPARE(KMime::unfoldHeader("bla   \r\nblub"), QByteArray("bla blub"));
    // multiple folding
    QCOMPARE(KMime::unfoldHeader("bla\nbla\nblub"), QByteArray("bla bla blub"));
    QCOMPARE(KMime::unfoldHeader("bla  \r\n   bla  \r\n  blub"), QByteArray("bla bla blub"));
    QCOMPARE(KMime::unfoldHeader("bla\n"), QByteArray("bla"));
    // bug #86302 - malformed header continuation
    QCOMPARE(KMime::unfoldHeader("bla\n=20bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla\n=09bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla\r\n=20bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla\r\n=09bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla \n=20 bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla \n=09 bla"), QByteArray("bla bla"));
    QCOMPARE(KMime::unfoldHeader("bla \n =20 bla"), QByteArray("bla =20 bla"));
    QCOMPARE(KMime::unfoldHeader("bla \n =09 bla"), QByteArray("bla =09 bla"));
}

void UtilTest::testExtractHeader()
{
    QByteArray header("To: <foo@bla.org>\n"
                      "Subject: =?UTF-8?Q?_Notification_for_appointment:?=\n"
                      " =?UTF-8?Q?_Test?=\n"
                      "Continuation: =?UTF-8?Q?_TEST\n"
                      "=20CONT1?= =?UTF-8?Q?_TEST\n"
                      "=09CONT2?=\n"
                      "MIME-Version: 1.0");

    // basic tests
    QVERIFY(extractHeader(header, "Foo").isEmpty());
    QCOMPARE(extractHeader(header, "To"), QByteArray("<foo@bla.org>"));

    // case insensitive matching
    QCOMPARE(extractHeader(header, "mime-version"), QByteArray("1.0"));

    // extraction of multi-line headers
    QCOMPARE(extractHeader(header, "Subject"),
             QByteArray("=?UTF-8?Q?_Notification_for_appointment:?= =?UTF-8?Q?_Test?="));

    // bug #86302 - malformed header continuation
    QCOMPARE(extractHeader(header, "Continuation"),
             QByteArray("=?UTF-8?Q?_TEST CONT1?= =?UTF-8?Q?_TEST CONT2?="));

    // missing space after ':'
    QCOMPARE(extractHeader("From:<toma@kovoks.nl>", "From"), QByteArray("<toma@kovoks.nl>"));
}

void UtilTest::testBalanceBidiState()
{
    QFETCH(QString, input);
    QFETCH(QString, expResult);

    QCOMPARE(balanceBidiState(input), expResult);
}

void UtilTest::testBalanceBidiState_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expResult");

    const QString LRO(QChar(0x202D));
    const QString RLO(QChar(0x202E));
    const QString LRE(QChar(0x202A));
    const QString RLE(QChar(0x202B));
    const QString PDF(QChar(0x202C));

    QTest::newRow("") << QString::fromLatin1("Normal") << QString::fromLatin1("Normal");
    QTest::newRow("") << RLO + QString::fromLatin1("Balanced") + PDF << RLO + QString::fromLatin1("Balanced") + PDF;
    QTest::newRow("") << RLO + QString::fromLatin1("MissingPDF1") << RLO + QString::fromLatin1("MissingPDF1") + PDF;
    QTest::newRow("") << QString::fromLatin1("\"") + RLO + QString::fromLatin1("Quote\"") << QString::fromLatin1("\"") + RLO + QString::fromLatin1("Quote") + PDF + QString::fromLatin1("\"");
    QTest::newRow("") << QString::fromLatin1("MissingPDF2") + RLO << QString::fromLatin1("MissingPDF2") + RLO + PDF;
    QTest::newRow("") << RLO + QString::fromLatin1("MultipleRLO") + RLO << RLO + QString::fromLatin1("MultipleRLO") + RLO + PDF + PDF;
    QTest::newRow("") << LRO + QString::fromLatin1("Mixed") + LRE + RLE + RLO + QString::fromLatin1("Bla")
                      << LRO + QString::fromLatin1("Mixed") + LRE + RLE + RLO + QString::fromLatin1("Bla") + PDF.repeated(4);
    QTest::newRow("") << RLO + QString::fromLatin1("TooManyPDF") + PDF + RLO + PDF + PDF
                      << RLO + QString::fromLatin1("TooManyPDF") + PDF + RLO + PDF;
    QTest::newRow("") << PDF + QString::fromLatin1("WrongOrder") + RLO
                      << QString::fromLatin1("WrongOrder") + RLO + PDF;
    QTest::newRow("") << QString::fromLatin1("ComplexOrder") + RLO + PDF + PDF + RLO
                      << QString::fromLatin1("ComplexOrder") + RLO + PDF + RLO + PDF;
    QTest::newRow("") << QString::fromLatin1("ComplexOrder2") + RLO + PDF + PDF + PDF + RLO + PDF + PDF + PDF
                      << QString::fromLatin1("ComplexOrder2") + RLO + PDF + RLO + PDF;
    QTest::newRow("") << PDF + PDF + PDF + QString::fromLatin1("ComplexOrder3") + PDF + PDF + RLO + PDF + PDF + PDF
                      << QString::fromLatin1("ComplexOrder3") + RLO + PDF;
}

void UtilTest::testAddQuotes()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, expResult);
    QFETCH(bool, forceQuotes);

    addQuotes(input, forceQuotes);
    QCOMPARE(input.data(), expResult.data());
}

void UtilTest::testAddQuotes_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("expResult");
    QTest::addColumn<bool>("forceQuotes");

    QTest::newRow("") << QByteArray("Test") << QByteArray("Test") << false;
    QTest::newRow("") << QByteArray("Test") << QByteArray("\"Test\"") << true;
    QTest::newRow("") << QByteArray("Lastname, Firstname")
                      << QByteArray("\"Lastname, Firstname\"") << false;
    QTest::newRow("") << QByteArray("John \"the hacker\" Smith")
                      << QByteArray("\"John \\\"the hacker\\\" Smith\"") << false;

    // Test the whole thing on strings as well, for one example
    QString string(QLatin1String("John \"the hacker\" Smith"));
    addQuotes(string, false);
    QCOMPARE(string, QString::fromLatin1("\"John \\\"the hacker\\\" Smith\""));
}

void UtilTest::testIsSigned_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<bool>("hasSignature");

    QTest::newRow("pgp") << QByteArray(
                             "From: xxx xxx <xxx@xxx.xxx>\n"
                             "To: xxx xxx <xxx@xxx.xxx>\n"
                             "Subject: Re: xxx\n"
                             "Date: Mon, 13 Dec 2010 12:22:03 +0100\n"
                             "MIME-Version: 1.0\n"
                             "Content-Type: multipart/signed;\n"
                             "  boundary=\"nextPart1571960.gHxU0aGA9V\";\n"
                             "  protocol=\"application/pgp-signature\";\n"
                             "  micalg=pgp-sha1\n"
                             "Content-Transfer-Encoding: 7bit\n\n"
                             "--nextPart1571960.gHxU0aGA9V\n"
                             "Content-Type: text/plain;\n"
                             "  charset=\"iso-8859-15\"\n"
                             "Content-Transfer-Encoding: quoted-printable\n"
                             "Content-Disposition: inline\n\n"
                             "Hi there...\n\n"
                             "--nextPart1571960.gHxU0aGA9V\n"
                             "Content-Type: application/pgp-signature; name=signature.asc\n"
                             "Content-Description: This is a digitally signed message part.\n\n"
                             "-----BEGIN PGP SIGNATURE-----\n"
                             "Version: GnuPG v2.0.15 (GNU/Linux)\n"
                             "...\n"
                             "-----END PGP SIGNATURE-----\n\n"
                             "--nextPart1571960.gHxU0aGA9V--\n"
                         ) << true;
}

void UtilTest::testIsSigned()
{
    QFETCH(QByteArray, input);
    QFETCH(bool, hasSignature);

    KMime::Message::Ptr msg(new KMime::Message);
    msg->setContent(input);
    msg->parse();
    QCOMPARE(isSigned(msg.data()), hasSignature);
}

void UtilTest::testIsCryptoPart_data()
{
    QTest::addColumn<QByteArray>("mimeType");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("isCrypto");

    QTest::newRow("pgp-encrypted") << QByteArray("application/pgp-encrypted") << QString() << true;
    QTest::newRow("pgp-encrypted upper case") << QByteArray("APPLICATION/PGP-ENCRYPTED") << QString() << true;
    QTest::newRow("pgp-signature") << QByteArray("application/pgp-signature") << QString() << true;
    QTest::newRow("pkcs7-mime") << QByteArray("application/pkcs7-mime") << QString() << true;
    QTest::newRow("pkcs7-signature") << QByteArray("application/pkcs7-signature") << QString() << true;
    QTest::newRow("x-pkcs7-signature") << QByteArray("application/x-pkcs7-signature") << QString() << true;
    QTest::newRow("msg.asc") << QByteArray("application/octet-stream") << QStringLiteral("msg.asc") << true;
    QTest::newRow("msg.asc upper case") << QByteArray("application/octet-stream") << QStringLiteral("MSG.ASC") << true;
    QTest::newRow("encrypted.asc") << QByteArray("application/octet-stream") << QStringLiteral("encrypted.asc") << true;
    QTest::newRow("octet-stream") << QByteArray("application/octet-stream") << QStringLiteral("bla.foo") << false;
    QTest::newRow("wrong mimetype") << QByteArray("application/foo") << QString() << false;
    QTest::newRow("text") << QByteArray("text/plain") << QString() << false;
    QTest::newRow("encrypted.asc wrong type") << QByteArray("application/foo") << QStringLiteral("encrypted.asc") << false;
    QTest::newRow("msc.asc wrong type") << QByteArray("application/foo") << QStringLiteral("msc.asc") << false;
}

void UtilTest::testIsCryptoPart()
{
    QFETCH(QByteArray, mimeType);
    QFETCH(QString, fileName);
    QFETCH(bool, isCrypto);

    KMime::Content c;
    c.contentType()->setMimeType(mimeType);
    c.contentDisposition()->setFilename(fileName);

    QCOMPARE(KMime::isCryptoPart(&c), isCrypto);
}

void UtilTest::testIsAttachment_data()
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

void UtilTest::testIsAttachment()
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
void UtilTest::testIsAttachmentSpecial()
{
    // don't crash on invalid input
    QCOMPARE(KMime::isAttachment(Q_NULLPTR), false);
    QCOMPARE(KMime::hasAttachment(Q_NULLPTR), false);

    // disposition type "attachment" is a clear indicator...
    KMime::Content c;
    c.contentDisposition()->setDisposition(Headers::CDattachment);
    QCOMPARE(KMime::isAttachment(&c), true);
}

void UtilTest::testHasAttachment()
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

    QEXPECT_FAIL("", "still broken", Continue);
    QCOMPARE(KMime::hasAttachment(root), false);
    QEXPECT_FAIL("", "still broken", Continue);
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
