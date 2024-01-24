/*
  SPDX-FileCopyrightText: 2015 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_dateformatter.h"

#include <QTest>
#include <QObject>
#include <QDateTime>

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
        QCOMPARE(f.dateString(dt), QLatin1String("Today 12:34 PM"));

        dt.setDate(dt.date().addDays(-1));
        QCOMPARE(f.dateString(dt), QLatin1String("Yesterday 12:34 PM"));

        dt.setDate(dt.date().addDays(-1));
        QVERIFY(f.dateString(dt).startsWith(QLocale::c().toString(dt, QLatin1String("dddd"))));
    }

    void testLocalizedFormat()
    {
        DateFormatter f(DateFormatter::Localized);

        auto dt = QDateTime(QDate(2015, 5, 26), QTime(12, 34, 56));
        QCOMPARE(f.dateString(dt, QLatin1String("de")), QString::fromLatin1("26.05.15 12:34"));
    }

};

QTEST_MAIN(DateFormatterTest)

#include "dateformattertest.moc"
