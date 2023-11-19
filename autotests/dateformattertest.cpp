/*
  SPDX-FileCopyrightText: 2015 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_dateformatter.h"

#include <QTest>
#include <QObject>
#include <QDateTime>
#include <QTimeZone>

using namespace KMime;

class DateFormatterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testFancyFormat()
    {
        DateFormatter f(DateFormatter::Fancy);

        auto dt = QDateTime::currentDateTime();
        dt.setTime(QTime(12, 34, 56));
        QCOMPARE(f.dateString(dt), QString::fromLatin1("Today 12:34:56"));

        dt.setDate(dt.date().addDays(-1));
        QCOMPARE(f.dateString(dt), QString::fromLatin1("Yesterday 12:34:56"));

        dt.setDate(dt.date().addDays(-1));
        QVERIFY(f.dateString(dt).startsWith(QLocale::c().toString(dt, QLatin1String("dddd"))));
    }

    void testLocalizedFormat()
    {
        DateFormatter f(DateFormatter::Localized);

        auto dt = QDateTime(QDate(2015, 5, 26), QTime(12, 34, 56));
        QCOMPARE(f.dateString(dt, QLatin1String("de")), QString::fromLatin1("26.05.15 12:34"));
    }

    void testFormat_data()
    {
        QTest::addColumn<KMime::DateFormatter::FormatType>("format");
        QTest::addColumn<QDateTime>("dt");
        QTest::addColumn<QString>("output");

        QTest::newRow("ctime") << DateFormatter::CTime << QDateTime(QDate(2023, 11, 18), QTime(17, 34, 56)) << QStringLiteral("Sat Nov 18 17:34:56 2023");
    }

    void testFormat()
    {
        QFETCH(KMime::DateFormatter::FormatType, format);
        QFETCH(QDateTime, dt);
        QFETCH(QString, output);

        KMime::DateFormatter formatter(format);
        QCOMPARE(formatter.dateString(dt), output);
    }

    void testCustomFormat()
    {
        DateFormatter f(DateFormatter::Custom);
        f.setCustomFormat(QStringLiteral("hh:mm Z"));
        auto dt = QDateTime(QDate(2023, 11, 18), QTime(17, 34, 56), QTimeZone("Europe/Brussels"));
        QCOMPARE(f.dateString(dt), QLatin1String("17:34 +0100"));
    }
};

QTEST_MAIN(DateFormatterTest)

#include "dateformattertest.moc"
