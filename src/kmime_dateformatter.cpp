/*
  kmime_dateformatter.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the DateFormatter class.

  @brief
  Defines the DateFormatter class.

  @authors the KMime authors (see AUTHORS file)
*/

#include "kmime_dateformatter.h"

#include <config-kmime.h>

#include <QTextStream>

#include <KLocalizedString>

using namespace KMime;

namespace KMime {

class DateFormatterPrivate {
public:
    DateFormatterPrivate()
    {}

    /**
      Returns a QString containing the specified time_t @p t formatted
      using the #Fancy #FormatType.

      @param t is the time_t to use for formatting.
    */
    QString fancy(time_t t);

    /**
      Returns a QString containing the specified time_t @p t formatted
      using the #Localized #FormatType.

      @param t is the time_t to use for formatting.
      @param shortFormat if true, create the short version of the date string.
      @param lang is a QString containing the language to use.
    */
    static QString localized(time_t t, bool shortFormat = true, const QString &lang = QString());

    /**
      Returns a QString containing the specified time_t @p t formatted
      with the ctime() function.

      @param t is the time_t to use for formatting.
    */
    static QString cTime(time_t t);

    /**
      Returns a QString containing the specified time_t @p t in the
      "%Y-%m-%d %H:%M:%S" #Iso #FormatType.

      @param t is the time_t to use for formatting.
    */
    static QString isoDate(time_t t);

    /**
      Returns a QString containing the specified time_t @p t in the
      #Rfc #FormatType.

      @param t is the time_t to use for formatting.
    */
    static QString rfc2822(time_t t);

    /**
      Returns a QString containing the specified time_t @p t formatted
      with a previously specified custom format.

      @param t time used for formatting
    */
    QString custom(time_t t) const;

    /**
      Returns a QString that identifies the timezone (eg."-0500")
      of the specified time_t @p t.

      @param t time to compute timezone from.
    */
    static QByteArray zone(time_t t);

    DateFormatter::FormatType mFormat;
    time_t mTodayOneSecondBeforeMidnight = 0;
    QString mCustomFormat;
};

}

DateFormatter::DateFormatter(FormatType ftype) :
    d(new DateFormatterPrivate)
{
    d->mFormat = ftype;
}

DateFormatter::~DateFormatter() = default;

DateFormatter::FormatType DateFormatter::format() const
{
    return d->mFormat;
}

void DateFormatter::setFormat(FormatType ftype)
{
    d->mFormat = ftype;
}

QString DateFormatter::dateString(time_t t, const QString &lang, bool shortFormat) const
{
    switch (d->mFormat) {
    case Fancy:
        return d->fancy(t);
    case Localized:
        return d->localized(t, shortFormat, lang);
    case CTime:
        return d->cTime(t);
    case Iso:
        return d->isoDate(t);
    case Rfc:
        return d->rfc2822(t);
    case Custom:
        return d->custom(t);
    }
    return QString();
}

QString DateFormatter::dateString(const QDateTime &dt, const QString &lang, bool shortFormat) const
{
    return dateString(dt.toLocalTime().toSecsSinceEpoch(), lang, shortFormat);
}

QString DateFormatterPrivate::rfc2822(time_t t)
{
    QDateTime tmp;
    QString ret;

    tmp.setSecsSinceEpoch(t);

    ret = tmp.toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss "));
    ret += QLatin1String(zone(t));

    return ret;
}

QString DateFormatterPrivate::custom(time_t t) const
{
    if (mCustomFormat.isEmpty()) {
        return QString();
    }

    int z = mCustomFormat.indexOf(QLatin1Char('Z'));
    QDateTime dt;
    QString ret = mCustomFormat;

    dt.setSecsSinceEpoch(t);
    if (z != -1) {
        ret.replace(z, 1, QLatin1String(zone(t)));
    }

    ret = dt.toString(ret);

    return ret;
}

void DateFormatter::setCustomFormat(const QString &format)
{
    d->mCustomFormat = format;
    d->mFormat = Custom;
}

QString DateFormatter::customFormat() const
{
    return d->mCustomFormat;
}

QByteArray DateFormatterPrivate::zone(time_t t)
{
#if defined(HAVE_TIMEZONE) || defined(HAVE_TM_GMTOFF)
    struct tm *local = localtime(&t);
#endif

#if defined(HAVE_TIMEZONE)

    //hmm, could make hours & mins static
    int secs = qAbs(timezone);
    int neg  = (timezone > 0) ? 1 : 0;
    int hours = secs / 3600;
    int mins  = (secs - hours * 3600) / 60;

    // adjust to daylight
    if (local->tm_isdst > 0) {
        if (neg) {
            --hours;
        } else {
            ++hours;
        }
    }

#elif defined(HAVE_TM_GMTOFF)

    int secs = qAbs(local->tm_gmtoff);
    int neg  = (local->tm_gmtoff < 0) ? 1 : 0;
    int hours = secs / 3600;
    int mins  = (secs - hours * 3600) / 60;

#else

    QDateTime d1 = QDateTime::fromString(QString::fromLatin1(asctime(gmtime(&t))));
    QDateTime d2 = QDateTime::fromString(QString::fromLatin1(asctime(localtime(&t))));
    int secs = d1.secsTo(d2);
    int neg = (secs < 0) ? 1 : 0;
    secs = qAbs(secs);
    int hours = secs / 3600;
    int mins  = (secs - hours * 3600) / 60;

#endif /* HAVE_TIMEZONE */

    QByteArray ret;
    QTextStream s(&ret, QIODevice::WriteOnly);
    s << (neg ? '-' : '+')         
      << qSetFieldWidth(2) << qSetPadChar(QLatin1Char('0'))
      << Qt::right
      << hours << mins;
    //old code: ret.sprintf( "%c%.2d%.2d", (neg) ? '-' : '+', hours, mins );

    return ret;
}

QString DateFormatterPrivate::fancy(time_t t)
{
    auto locale = QLocale::system();

    if (t <= 0) {
        return i18nc("invalid time specified", "unknown");
    }

    if (mTodayOneSecondBeforeMidnight < time(nullptr)) {
        // determine time_t value of today 23:59:59
        const QDateTime today(QDate::currentDate(), QTime(23, 59, 59));
        mTodayOneSecondBeforeMidnight = today.toSecsSinceEpoch();
    }

    QDateTime old;
    old.setSecsSinceEpoch(t);

    if (mTodayOneSecondBeforeMidnight >= t) {
        const time_t diff = mTodayOneSecondBeforeMidnight - t;
        if (diff < 7 * 24 * 60 * 60) {
            if (diff < 24 * 60 * 60) {
                return i18n("Today %1",
                            locale.toString(old.time(), QLocale::ShortFormat));
            }
            if (diff < 2 * 24 * 60 * 60) {
                return i18n("Yesterday %1",
                            locale.toString(old.time(), QLocale::ShortFormat));
            }
            for (int i = 3; i < 8; i++) {
                if (diff < i * 24 * 60 * 60) {
                    return i18nc("1. weekday, 2. time", "%1 %2" ,
                                 locale.dayName(old.date().dayOfWeek(), QLocale::LongFormat),
                                 locale.toString(old.time(), QLocale::ShortFormat));
                }
            }
        }
    }

    return locale.toString(old, QLocale::ShortFormat);
}

QString DateFormatterPrivate::localized(time_t t, bool shortFormat, const QString &lang)
{
    QDateTime tmp;
    QString ret;
    auto locale = QLocale::system();

    tmp.setSecsSinceEpoch(t);

    if (!lang.isEmpty()) {
        locale = QLocale(lang);
        ret = locale.toString(tmp, (shortFormat ? QLocale::ShortFormat : QLocale::LongFormat));
    } else {
        ret = locale.toString(tmp, (shortFormat ? QLocale::ShortFormat : QLocale::LongFormat));
    }

    return ret;
}

QString DateFormatterPrivate::cTime(time_t t)
{
    return QString::fromLatin1(ctime(&t)).trimmed();
}

QString DateFormatterPrivate::isoDate(time_t t)
{
    char cstr[64];
    strftime(cstr, 63, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return QLatin1String(cstr);
}

QString DateFormatter::formatDate(FormatType ftype, time_t t,
                                  const QString &data, bool shortFormat)
{
    DateFormatter f(ftype);
    if (ftype == Custom) {
        f.setCustomFormat(data);
    }
    return f.dateString(t, data, shortFormat);
}

QString DateFormatter::formatCurrentDate(FormatType ftype, const QString &data, bool shortFormat)
{
    DateFormatter f(ftype);
    if (ftype == Custom) {
        f.setCustomFormat(data);
    }
    return f.dateString(time(nullptr), data, shortFormat);
}
