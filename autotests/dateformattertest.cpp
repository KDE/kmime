/*
  Copyright (c) 2015 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "kmime_dateformatter.h"

#include <qtest.h>
#include <QObject>
#include <QDateTime>
#include <QDebug>

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
        QCOMPARE(f.dateString(dt), QString::fromLatin1("Today 12:34 PM"));

        dt.setDate(dt.date().addDays(-1));
        QCOMPARE(f.dateString(dt), QString::fromLatin1("Yesterday 12:34 PM"));

        dt.setDate(dt.date().addDays(-1));
        QVERIFY(f.dateString(dt).startsWith(QLocale::c().toString(dt, QLatin1String("dddd"))));
    }

};

QTEST_MAIN(DateFormatterTest)

#include "dateformattertest.moc"
