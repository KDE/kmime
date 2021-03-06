#include "kmime_dateformatter.h"
#include "kmime_header_parsing.h"
#include <QDebug>
using namespace KMime;

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
    setenv("TZ", "UTC", 1);
}

Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

int
main()
{
    DateFormatter t;

    time_t ntime = time(nullptr);
    qDebug() << "Time now:";
    qDebug() << "tFancy : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Localized);
    qDebug() << "tLocalized : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::CTime);
    qDebug() << "tCTime : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Iso);
    qDebug() << "tIso   : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Rfc);
    qDebug() << "trfc2822 : \t" << t.dateString(ntime);
    QString rfcd = t.formatDate(DateFormatter::Rfc, ntime);
    QDateTime dt;
    QDateTime qdt;
    QByteArray ba = rfcd.toLatin1();
    const char *str = ba.constData();
    if (HeaderParsing::parseDateTime(str, str + rfcd.length(), dt)) {
        qDebug() << " ntime =" << (ntime) << " dt =" << (dt.toSecsSinceEpoch());
        qdt.setSecsSinceEpoch(dt.toSecsSinceEpoch());
        qDebug() << " qq =" << qdt.toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        qDebug() << " rfc2822 :" << t.formatDate(DateFormatter::Rfc, dt.toSecsSinceEpoch());
    }
    QString ddd = QStringLiteral("Mon, 05 Aug 2002 01:57:51 -0700");
    ba = ddd.toLatin1();
    str = ba.constData();
    if (HeaderParsing::parseDateTime(str, str + ddd.length(), dt)) {
        qDebug() << "dt =" << (dt.toSecsSinceEpoch());
        qDebug() << " rfc2822 :" << t.formatDate(DateFormatter::Rfc, dt.toSecsSinceEpoch());
    }

    t.setCustomFormat(QStringLiteral("MMMM dddd yyyy Z"));
    qDebug() << "tCustom : \t" << t.dateString(ntime);

    ntime -= (24 * 3600 + 1);
    qDebug() << "Time 24 hours and 1 second ago:";
    t.setFormat(DateFormatter::Fancy);
    qDebug() << "tFancy : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Localized);
    qDebug() << "tLocalized : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::CTime);
    qDebug() << "tCTime : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Iso);
    qDebug() << "tIso   : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Rfc);
    qDebug() << "trfc2822 : \t" << t.dateString(ntime);
    t.setCustomFormat(QStringLiteral("MMMM dddd Z yyyy"));
    qDebug() << "tCustom : \t" << t.dateString(ntime);

    t.setFormat(DateFormatter::Fancy);
    ntime -= (24 * 3600 * 30 + 59);
    qDebug() << "Time 31 days and 1 minute ago:";
    qDebug() << "tFancy : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Localized);
    qDebug() << "tLocalized : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::CTime);
    qDebug() << "tCTime : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Iso);
    qDebug() << "tIso   : \t" << t.dateString(ntime);
    t.setFormat(DateFormatter::Rfc);
    qDebug() << "trfc2822 : \t" << t.dateString(ntime);
    t.setCustomFormat(QStringLiteral("MMMM Z dddd yyyy"));
    qDebug() << "tCustom : \t" << t.dateString(ntime);

    qDebug() << "Static functions (dates like in the last test):";
    qDebug() << "tFancy : \t" << DateFormatter::formatDate(DateFormatter::Fancy, ntime);
    qDebug() << "tLocalized : \t" << DateFormatter::formatDate(DateFormatter::Localized, ntime);
    qDebug() << "tCTime : \t" << DateFormatter::formatDate(DateFormatter::CTime, ntime);
    qDebug() << "tIso   : \t" << DateFormatter::formatDate(DateFormatter::Iso, ntime);
    qDebug() << "trfc2822 : \t" << DateFormatter::formatDate(DateFormatter::Rfc, ntime);
    qDebug() << "tCustom : \t" << DateFormatter::formatDate(DateFormatter::Custom, ntime,
             QStringLiteral("Z MMMM dddd yyyy"));
    t.setFormat(DateFormatter::Fancy);
    qDebug() << "QDateTime taking: (dates as in first test)";
    qDebug() << "tFancy : \t" << t.dateString((QDateTime::currentDateTime()));
    t.setFormat(DateFormatter::Localized);
    qDebug() << "tLocalized : \t" << t.dateString(QDateTime::currentDateTime());
    t.setFormat(DateFormatter::CTime);
    qDebug() << "tCTime : \t" << t.dateString(QDateTime::currentDateTime());
    t.setFormat(DateFormatter::Iso);
    qDebug() << "tIso   : \t" << t.dateString(QDateTime::currentDateTime());
    t.setFormat(DateFormatter::Rfc);
    qDebug() << "tIso   : \t" << t.dateString(QDateTime::currentDateTime());
    t.setCustomFormat(QStringLiteral("MMMM d dddd yyyy Z"));
    qDebug() << "tCustom : \t" << t.dateString(QDateTime::currentDateTime());
}
