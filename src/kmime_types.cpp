/*  -*- c++ -*-
    kmime_header_types.cpp

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kmime_types.h"
#include "kmime_codecs.h"
#include "kmime_header_parsing.h"
#include "kmime_header_parsing_p.h"
#include "kmime_util.h"
#include "kmime_util_p.h"
#include "kmime_debug.h"

#include <KCodecs>

#include <QStringList>
#include <QUrl>

namespace KMime
{

namespace Types
{

// QUrl::fromAce is extremely expensive, so only use it when necessary.
// Fortunately, the presence of IDNA is readily detected with a substring match...
static inline QString QUrl_fromAce_wrapper(const QString &domain)
{
    if (domain.contains(QLatin1String("xn--"))) {
        return QUrl::fromAce(domain.toLatin1());
    } else {
        return domain;
    }
}

static QString addr_spec_as_string(const AddrSpec &as, bool pretty)
{
    if (as.isEmpty()) {
        return QString();
    }

    static const QChar dotChar = QLatin1Char('.');
    static const QChar backslashChar = QLatin1Char('\\');
    static const QChar quoteChar = QLatin1Char('"');

    bool needsQuotes = false;
    QString result;
    result.reserve(as.localPart.length() + as.domain.length() + 1);
    for (int i = 0 ; i < as.localPart.length() ; ++i) {
        const QChar ch = as.localPart.at(i);
        if (ch == dotChar || isAText(ch.toLatin1())) {
            result += ch;
        } else {
            needsQuotes = true;
            if (ch == backslashChar || ch == quoteChar) {
                result += backslashChar;
            }
            result += ch;
        }
    }
    const QString dom = pretty ? QUrl_fromAce_wrapper(as.domain) : as.domain ;
    if (needsQuotes) {
        result = quoteChar + result + quoteChar;
    }
    if (dom.isEmpty()) {
        return result;
    } else {
        result += QLatin1Char('@');
        result += dom;
        return result;
    }
}

QString AddrSpec::asString() const
{
    return addr_spec_as_string(*this, false);
}

QString AddrSpec::asPrettyString() const
{
    return addr_spec_as_string(*this, true);
}

bool AddrSpec::isEmpty() const
{
    return localPart.isEmpty() && domain.isEmpty();
}

QByteArray Mailbox::address() const
{
    QByteArray result;
    const QString asString = addr_spec_as_string(mAddrSpec, false);
    if (!asString.isEmpty()) {
        result = asString.toLatin1();
    }
    return result;
    //return mAddrSpec.asString().toLatin1();
}

AddrSpec Mailbox::addrSpec() const
{
    return mAddrSpec;
}

QString Mailbox::name() const
{
    return mDisplayName;
}

void Mailbox::setAddress(const AddrSpec &addr)
{
    mAddrSpec = addr;
}

void Mailbox::setAddress(const QByteArray &addr)
{
    const char *cursor = addr.constData();
    if (!HeaderParsing::parseAngleAddr(cursor,
                                       cursor + addr.length(), mAddrSpec)) {
        if (!HeaderParsing::parseAddrSpec(cursor, cursor + addr.length(),
                                          mAddrSpec)) {
            qCWarning(KMIME_LOG) << "Mailbox: Invalid address";
            return;
        }
    }
}

void Mailbox::setName(const QString &name)
{
    mDisplayName = removeBidiControlChars(name);
}

void Mailbox::setNameFrom7Bit(const QByteArray &name,
                              const QByteArray &defaultCharset)
{
    QByteArray cs;
    setName(KCodecs::decodeRFC2047String(name, &cs, defaultCharset));
}

bool Mailbox::hasAddress() const
{
    return !mAddrSpec.isEmpty();
}

bool Mailbox::hasName() const
{
    return !mDisplayName.isEmpty();
}

QString Mailbox::prettyAddress(Quoting quoting) const
{
    if (!hasName()) {
        return QLatin1String(address());
    }
    QString s = name();
    if (quoting != QuoteNever) {
        addQuotes(s, quoting == QuoteAlways /*bool force*/);
    }

    if (hasAddress()) {
        s += QLatin1String(" <") + QLatin1String(address()) + QLatin1Char('>');
    }
    return s;
}

void Mailbox::fromUnicodeString(const QString &s)
{
    from7BitString(encodeRFC2047Sentence(s, "utf-8"));
}

void Mailbox::from7BitString(const QByteArray &s)
{
    const char *cursor = s.constData();
    HeaderParsing::parseMailbox(cursor, cursor + s.length(), *this);
}

QByteArray Mailbox::as7BitString(const QByteArray &encCharset) const
{
    if (!hasName()) {
        return address();
    }
    QByteArray rv;
    if (isUsAscii(name())) {
        QByteArray tmp = name().toLatin1();
        addQuotes(tmp, false);
        rv += tmp;
    } else {
        rv += encodeRFC2047String(name(), encCharset, true);
    }
    if (hasAddress()) {
        rv += " <" + address() + '>';
    }
    return rv;
}

QVector<KMime::Types::Mailbox> Mailbox::listFromUnicodeString(const QString &s)
{
    return listFrom7BitString(encodeRFC2047Sentence(s, "utf-8"));
}

QVector<KMime::Types::Mailbox> Mailbox::listFrom7BitString(const QByteArray& s)
{
    QVector<KMime::Types::Mailbox> res;
    QVector<KMime::Types::Address> maybeAddressList;
    const char *scursor = s.constData();
    if (!HeaderParsing::parseAddressList(scursor, scursor + s.size(), maybeAddressList)) {
        return res;
    }

    res.reserve(maybeAddressList.size());
    for (const auto &it : qAsConst(maybeAddressList)) {
        res += (it).mailboxList;
    }
    return res;
}

QString Mailbox::listToUnicodeString(const QVector<Mailbox>& mailboxes)
{
    if (mailboxes.size() == 1) // QStringList free fast-path for the common case
        return mailboxes.at(0).prettyAddress();

    QStringList rv;
    rv.reserve(mailboxes.count());
    for (const Types::Mailbox &mbox : mailboxes) {
        rv.append(mbox.prettyAddress());
    }
    return rv.join(QLatin1String(", "));
}

} // namespace Types

} // namespace KMime
