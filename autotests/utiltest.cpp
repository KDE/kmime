/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#undef QT_USE_FAST_CONCATENATION
#undef QT_USE_FAST_OPERATOR_PLUS

#include <QTest>

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

void UtilTest::testLFCRLF_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("expected");
    QTest::addColumn<QByteArray>("convertedBack");

    const QByteArray noNewline("no newline character");
    QTest::newRow("none") << noNewline << noNewline << noNewline;
    QTest::newRow("alone") << QByteArray("\n") << QByteArray("\r\n") << QByteArray("\n");
    QTest::newRow("CRLF") << QByteArray("\r\n") << QByteArray("\r\n") << QByteArray("\n");
    QTest::newRow("single_first") << QByteArray("\nfoo") << QByteArray("\r\nfoo") << QByteArray("\nfoo");
    QTest::newRow("single_last") << QByteArray("foo\n") << QByteArray("foo\r\n") << QByteArray("foo\n");
    QTest::newRow("single_two_lines") << QByteArray("foo\nbar") << QByteArray("foo\r\nbar") << QByteArray("foo\nbar");
    QTest::newRow("two_lines") << QByteArray("foo\nbar\n") << QByteArray("foo\r\nbar\r\n") << QByteArray("foo\nbar\n");
    QTest::newRow("already_CRLF") << QByteArray("foo\r\nbar\r\n") << QByteArray("foo\r\nbar\r\n") << QByteArray("foo\nbar\n");
    QTest::newRow("mixed_CRLF_LF_unchanged") << QByteArray("foo\r\nbar\n") << QByteArray("foo\r\nbar\n") << QByteArray("foo\nbar\n");
    // out of scope QTest::newRow("mixed_LF_CRLF_unchanged") << QByteArray("foo\nbar\r\n") << QByteArray("foo\nbar\r\n") << QByteArray("foo\nbar\n");
}

void UtilTest::testLFCRLF()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, expected);
    QFETCH(QByteArray, convertedBack);

    const QByteArray output = KMime::LFtoCRLF(input);
    QCOMPARE(output, expected);
    const QByteArray output2 = KMime::LFtoCRLF(input.constData()); // test the const char* overload
    QCOMPARE(output2, expected);

    const QByteArray back = KMime::CRLFtoLF(output);
    QCOMPARE(back, convertedBack);
}

void UtilTest::testLFCRLF_performance()
{
    const QByteArray line = "This is one line\n";
    const int count = 1000;
    QByteArray input;
    input.reserve(line.size() * count);
    for (int i = 0 ; i < count; ++i) {
        input += line;
    }

    QByteArray output;
    QBENCHMARK {
        output = KMime::LFtoCRLF(input);
    }
    QByteArray expected = input;
    expected.replace('\n', "\r\n");
    QCOMPARE(output, expected);
}
