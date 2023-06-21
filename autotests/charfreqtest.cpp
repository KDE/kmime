/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "charfreqtest.h"
#include <QTest>


#include <../src/kmime_charfreq.cpp>
using namespace KMime;

QTEST_MAIN(CharFreqTest)

void CharFreqTest::test8bitData()
{
    {
        // If it has NUL then it's Binary (equivalent to EightBitData in CharFreq).
        QByteArray data("123");
        data += char(0);
        data += "test";
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::Binary);
    }

    {
        // If it has lines longer than 998, it's EightBitData.
        QByteArray data;
        for (int i = 0; i < 999; i++) {
            data += char(169);
        }
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitData);
    }

    {
        // If #CR != #CRLF then it's EightBitData.
        QByteArray data("©line1\r\nline2\r");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitData);
    }

    {
        // If #LF != #CRLF then it's EightBitData.
        QByteArray data("©line1\r\nline2\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitData);
    }

    {
        // If it has a lot of control chars, it's EightBitData.
        QByteArray data("©test\a\a\a\a\a\a\a");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitData);
    }
}

void CharFreqTest::test8bitText()
{
    {
        // If the text only contains newlines and some random accented chars, then it is EightBitText
        QByteArray data("asdfasdfasdfasdfasdfasdfäöü\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitText);
    }

    {
        // If it has no NULs, few CTLs, and only CRLFs, it's EightBitText.
        QByteArray data("©beware the beast but enjoy the feast he offers...\r\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::EightBitText);
    }
}

void CharFreqTest::test7bitData()
{
    {
        // If it has lines longer than 998, it's SevenBitData.
        QByteArray data;
        for (int i = 0; i < 999; i++) {
            data += 'a';
        }
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitData);
    }

    {
        // If #CR != #CRLF then it's SevenBitData.
        QByteArray data("line1\r\nline2\r");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitData);
    }

    {
        // If #LF != #CRLF then it's SevenBitData.
        QByteArray data("line1\r\nline2\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitData);
    }

    {
        // If it has a lot of control chars, it's SevenBitData.
        QByteArray data("test\a\a\a\a\a\a\a");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitData);
    }
}

void CharFreqTest::test7bitText()
{
    {
        // If the text only contains newlines, then it is SevenBitText
        QByteArray data("line1\nline2\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitText);
    }

    {
        // If it has no NULs, few CTLs, and only CRLFs, it's SevenBitText.
        QByteArray data("beware the beast but enjoy the feast he offers...\r\n");
        CharFreq cf(data);
        QCOMPARE(cf.type(), CharFreq::SevenBitText);
    }
}

void CharFreqTest::testTrailingWhitespace()
{
    QByteArray data("test ");
    CharFreq cf(data);
    QVERIFY(cf.hasTrailingWhitespace());
}

void CharFreqTest::testLeadingFrom()
{
    QByteArray data("From here thither");
    CharFreq cf(data);
    QVERIFY(cf.hasLeadingFrom());
}


#include "moc_charfreqtest.cpp"
