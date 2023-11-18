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

      @param t is the time to use for formatting.
    */
    QString fancy(const QDateTime &t);

    /**
      Returns a QString containing the specified time_t @p t formatted
      using the #Localized #FormatType.

      @param t is the time to use for formatting.
      @param shortFormat if true, create the short version of the date string.
      @param lang is a QString containing the language to use.
    */
    static QString localized(const QDateTime &t, bool shortFormat = true, const QString &lang = QString());

    /**
      Returns a QString containing the specified time_t @p t formatted
      with the ctime() function.

      @param t is the time to use for formatting.
    */
    static QString cTime(const QDateTime &t);

    /**
      Returns a QString containing the specified time_t @p t in the
      "%Y-%m-%d %H:%M:%S" #Iso #FormatType.

      @param t is the time to use for formatting.
    */
    static QString isoDate(const QDateTime &t);

    /**
      Returns a QString containing the specified time @p t in the
      #Rfc #FormatType.

      @param t is the time to use for formatting.
    */
    static QString rfc2822(const QDateTime &t);

    /**
      Returns a QString containing the specified time @p t formatted
      with a previously specified custom format.

      @param t time used for formatting
    */
    QString custom(const QDateTime &t) const;

    /**
      Returns a QString that identifies the timezone (eg."-0500")
      of the specified time_t @p t.

      @param t time to compute timezone from.
    */
    static QByteArray zone(const QDateTime &t);

    DateFormatter::FormatType mFormat;
    QDateTime mTodayOneSecondBeforeMidnight;
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
    return dateString(QDateTime::fromSecsSinceEpoch(t), lang, shortFormat);
}

QString DateFormatter::dateString(const QDateTime &dt, const QString &lang, bool shortFormat) const
{
    switch (d->mFormat) {
    case Fancy:
        return d->fancy(dt);
    case Localized:
        return d->localized(dt, shortFormat, lang);
    case CTime:
        return d->cTime(dt);
    case Iso:
        return d->isoDate(dt);
    case Rfc:
        return d->rfc2822(dt);
    case Custom:
        return d->custom(dt);
    }
    return {};
}

QString DateFormatterPrivate::rfc2822(const QDateTime &t)
{
    QString ret;

    ret = t.toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss "));
    ret += QLatin1String(zone(t));

    return ret;
}

QString DateFormatterPrivate::custom(const QDateTime &t) const
{
    if (mCustomFormat.isEmpty()) {
      return {};
    }

    int z = mCustomFormat.indexOf(QLatin1Char('Z'));
    QDateTime dt;
    QString ret = mCustomFormat;

    if (z != -1) {
        ret.replace(z, 1, QLatin1String(zone(t)));
    }

    ret = t.toString(ret);

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

QByteArray DateFormatterPrivate::zone(const QDateTime &t)
{
    const auto secs = t.offsetFromUtc();
    const auto hours = std::abs(secs / 3600);
    const auto mins  = std::abs((secs - hours * 3600) / 60);

    QByteArray ret(6, 0);
    qsnprintf(ret.data(), ret.size(), "%c%.2d%.2d", (secs < 0) ? '-' : '+', hours, mins);
    ret.chop(1);
    return ret;
}

QString DateFormatterPrivate::fancy(const QDateTime &t)
{
    if (!t.isValid()) {
        return i18nc("invalid time specified", "unknown");
    }

    if (!mTodayOneSecondBeforeMidnight.isValid()) {
        // determine time of today 23:59:59
        mTodayOneSecondBeforeMidnight = QDateTime(QDate::currentDate(), QTime(23, 59, 59));
    }

    QDateTime old(t);

    if (mTodayOneSecondBeforeMidnight >= t) {
        const auto diff = t.secsTo(mTodayOneSecondBeforeMidnight);
        if (diff < 7 * 24 * 60 * 60) {
            if (diff < 24 * 60 * 60) {
                return i18n("Today %1",
                            QLocale().toString(old.time(), QLocale::ShortFormat));
            }
            if (diff < 2 * 24 * 60 * 60) {
                return i18n("Yesterday %1",
                            QLocale().toString(old.time(), QLocale::ShortFormat));
            }
            for (int i = 3; i < 8; i++) {
                if (diff < i * 24 * 60 * 60) {
                    return i18nc("1. weekday, 2. time", "%1 %2" ,
                                 QLocale().dayName(old.date().dayOfWeek(), QLocale::LongFormat),
                                 QLocale().toString(old.time(), QLocale::ShortFormat));
                }
            }
        }
    }

    return QLocale().toString(old, QLocale::ShortFormat);
}

QString DateFormatterPrivate::localized(const QDateTime &t, bool shortFormat, const QString &lang)
{
    QString ret;

    if (!lang.isEmpty()) {
        ret = QLocale(lang).toString(t, (shortFormat ? QLocale::ShortFormat : QLocale::LongFormat));
    } else {
        ret = QLocale().toString(t, (shortFormat ? QLocale::ShortFormat : QLocale::LongFormat));
    }

    return ret;
}

QString DateFormatterPrivate::cTime(const QDateTime &t)
{
    return t.toString(QStringLiteral("ddd MMM dd hh:mm:ss yyyy"));
}

QString DateFormatterPrivate::isoDate(const QDateTime &t)
{
    return t.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
}

QString DateFormatter::formatDate(FormatType ftype, const QDateTime &t,
                                  const QString &data, bool shortFormat)
{
    DateFormatter f(ftype);
    if (ftype == Custom) {
        f.setCustomFormat(data);
    }
    return f.dateString(t, data, shortFormat);
}

QString DateFormatter::formatDate(FormatType ftype, time_t t,
                                  const QString &data, bool shortFormat)
{
    return formatDate(ftype, QDateTime::fromSecsSinceEpoch(t), data, shortFormat);
}

QString DateFormatter::formatCurrentDate(FormatType ftype, const QString &data, bool shortFormat)
{
    DateFormatter f(ftype);
    if (ftype == Custom) {
        f.setCustomFormat(data);
    }
    return f.dateString(QDateTime::currentDateTime(), data, shortFormat);
}
