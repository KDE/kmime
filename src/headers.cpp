/*  -*- c++ -*-
    kmime_headers.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 the KMime authors.
    See file AUTHORS for details
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the various header classes:
   - header's base class defining the common interface
   - generic base classes for different types of fields
   - incompatible, Structured-based field classes
   - compatible, Unstructured-based field classes

  @brief
  Defines the various headers classes.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>
*/

#include "headers.h"
#include "headers_p.h"
#include "headerparsing_p.h"

#include "util.h"
#include "util_p.h"
#include "codecs_p.h"
#include "headerfactory_p.h"
#include "kmime_debug.h"
#include "warning_p.h"

#include <KCodecs>

#include <cassert>
#include <cctype>

// macro to generate a default constructor implementation
#define kmime_mk_trivial_ctor( subclass, baseclass )                  \
    subclass::subclass() = default;          \
    subclass::~subclass() = default;

// end kmime_mk_trivial_ctor

#define kmime_mk_trivial_ctor_with_dptr( subclass, baseclass ) \
    subclass::subclass() : baseclass( new subclass##Private ) \
    {                                                                     \
    }                                                                     \
    \
	subclass::~subclass() { \
		Q_D(subclass); \
		delete d;  /* see comment above the BasePrivate class */ \
		d_ptr = nullptr; \
	}

// end kmime_mk_trivial_ctor_with_dptr

#define kmime_mk_trivial_ctor_with_name( subclass, baseclass, name )  \
    kmime_mk_trivial_ctor( subclass, baseclass )                          \
    \
    const char *subclass::type() const                                    \
    {                                                                     \
        return staticType();                                                \
    }                                                                     \
    const char *subclass::staticType() { return #name; }

#define kmime_mk_trivial_ctor_with_name_and_dptr( subclass, baseclass, name ) \
    kmime_mk_trivial_ctor_with_dptr( subclass, baseclass ) \
    const char *subclass::type() const { return staticType(); } \
    const char *subclass::staticType() { return #name; }

#define kmime_mk_dptr_ctor( subclass, baseclass ) \
    subclass::subclass( subclass##Private *d ) : baseclass( d ) {}

using namespace KMime;
using namespace KMime::Headers;
using namespace KMime::Types;
using namespace KMime::HeaderParsing;

namespace KMime
{
namespace Headers
{
//-----<Base>----------------------------------
Base::Base() : d_ptr(new BasePrivate)
{
}

Base::Base(BasePrivate *dd) :
    d_ptr(dd)
{
}

Base::~Base()
{
    delete d_ptr;
    d_ptr = nullptr;
}

QByteArray Base::rfc2047Charset() const
{
    if (d_ptr->encCS.isEmpty()) {
        return QByteArrayLiteral("UTF-8");
    } else {
        return d_ptr->encCS;
    }
}

void Base::setRFC2047Charset(const QByteArray &cs)
{
    d_ptr->encCS = cachedCharset(cs);
}

const char *Base::type() const
{
    return "";
}

bool Base::is(QByteArrayView t) const
{
    return t.compare(type(), Qt::CaseInsensitive) == 0;
}

//-----</Base>---------------------------------

namespace Generics
{

//-----<Unstructured>-------------------------

//@cond PRIVATE
kmime_mk_dptr_ctor(Unstructured, Base)
//@endcond

Unstructured::Unstructured() : Base(new UnstructuredPrivate)
{
}

Unstructured::~Unstructured()
{
    Q_D(Unstructured);
    delete d;
    d_ptr = nullptr;
}

void Unstructured::from7BitString(QByteArrayView s)
{
    Q_D(Unstructured);
    d->decoded = KCodecs::decodeRFC2047String(s, &d->encCS, QByteArrayLiteral("UTF-8"));
}

QByteArray Unstructured::as7BitString() const
{
    const Q_D(Unstructured);
    return encodeRFC2047String(d->decoded, rfc2047Charset()) ;
}

void Unstructured::fromUnicodeString(const QString &s)
{
    Q_D(Unstructured);
    d->decoded = s;
}

QString Unstructured::asUnicodeString() const
{
    return d_func()->decoded;
}

bool Unstructured::isEmpty() const
{
    return d_func()->decoded.isEmpty();
}

//-----</Unstructured>-------------------------

//-----<Structured>-------------------------

Structured::Structured() : Base(new StructuredPrivate)
{
}

kmime_mk_dptr_ctor(Structured, Base)

Structured::~Structured()
{
    Q_D(Structured);
    delete d;
    d_ptr = nullptr;
}


void Structured::from7BitString(QByteArrayView s)
{
    Q_D(Structured);
    if (d->encCS.isEmpty()) {
        d->encCS = QByteArrayLiteral("UTF-8");
    }
    auto p = s.data();
    parse(p, p + s.size());
}

QString Structured::asUnicodeString() const
{
    return QString::fromLatin1(as7BitString());
}

void Structured::fromUnicodeString(const QString &s)
{
    Q_D(Structured);
    from7BitString(s.toLatin1());
}

//-----</Structured>-------------------------

// helper method used in AddressList and MailboxList
static bool stringToMailbox(const QByteArray &address,
                            const QString &displayName, Types::Mailbox &mbox)
{
    Types::AddrSpec addrSpec;
    mbox.setName(displayName);
    const char *cursor = address.constData();
    if (!parseAngleAddr(cursor, cursor + address.length(), addrSpec)) {
        if (!parseAddrSpec(cursor, cursor + address.length(), addrSpec)) {
            qCWarning(KMIME_LOG) << "stringToMailbox: Invalid address";
            return false;
        }
    }
    mbox.setAddress(addrSpec);
    return true;
}

//-----<MailboxList>-------------------------

kmime_mk_trivial_ctor_with_dptr(MailboxList, Structured)
kmime_mk_dptr_ctor(MailboxList, Structured)

QByteArray MailboxList::as7BitString() const
{
    const Q_D(MailboxList);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    for (const Types::Mailbox &mbox : std::as_const(d->mailboxList)) {
        rv += mbox.as7BitString(rfc2047Charset());
        rv += ", ";
    }
    rv.resize(rv.length() - 2);
    return rv;
}

void MailboxList::fromUnicodeString(const QString &s)
{
    Q_D(MailboxList);
    from7BitString(encodeRFC2047Sentence(s, rfc2047Charset()));
}

QString MailboxList::asUnicodeString() const
{
    Q_D(const MailboxList);
    return Mailbox::listToUnicodeString(d->mailboxList);
}

bool MailboxList::isEmpty() const
{
    return d_func()->mailboxList.isEmpty();
}

void MailboxList::addAddress(const Types::Mailbox &mbox)
{
    Q_D(MailboxList);
    d->mailboxList.append(mbox);
}

void MailboxList::addAddress(const QByteArray &address,
                             const QString &displayName)
{
    Q_D(MailboxList);
    Types::Mailbox mbox;
    if (stringToMailbox(address, displayName, mbox)) {
        d->mailboxList.append(mbox);
    }
}

QList<QByteArray> MailboxList::addresses() const {
    QList<QByteArray> rv;
    rv.reserve(d_func()->mailboxList.count());
    const auto mailboxList = d_func()->mailboxList;
    for (const Types::Mailbox &mbox : mailboxList) {
        rv.append(mbox.address());
    }
    return rv;
}

QStringList MailboxList::displayNames() const
{
    Q_D(const MailboxList);
    QStringList rv;
    rv.reserve(d->mailboxList.count());
    for (const Types::Mailbox &mbox : std::as_const(d->mailboxList)) {
        if (mbox.hasName()) {
            rv.append(mbox.name());
        } else {
            rv.append(QString::fromLatin1(mbox.address()));
        }
    }
    return rv;
}

QString MailboxList::displayString() const
{
    Q_D(const MailboxList);
    if (d->mailboxList.size() == 1) { // fast-path to avoid temporary QStringList in the common case of just one From address
        const auto& mbox = d->mailboxList.at(0);
        if (mbox.hasName()) {
            return mbox.name();
        } else {
            return QString::fromLatin1(mbox.address());
        }
    }
    return displayNames().join(QLatin1StringView(", "));
}

QList<Types::Mailbox> MailboxList::mailboxes() const
{
    return d_func()->mailboxList;
}

void MailboxList::setMailboxes(const QList<Types::Mailbox> &mailboxes)
{
    Q_D(MailboxList);
    d->mailboxList = mailboxes;
}

bool MailboxList::parse(const char *&scursor, const char *const send,
                        NewlineType newline)
{
    Q_D(MailboxList);
    // examples:
    // from := "From:" mailbox-list CRLF
    // sender := "Sender:" mailbox CRLF

    // parse an address-list:
    QList<Types::Address> maybeAddressList;
    if (!parseAddressList(scursor, send, maybeAddressList, newline)) {
        return false;
    }

    d->mailboxList.clear();
    d->mailboxList.reserve(maybeAddressList.count());

    // extract the mailboxes and complain if there are groups:
    for (const auto &it : std::as_const(maybeAddressList)) {
        if (!(it).displayName().isEmpty()) {
            KMIME_WARN << "mailbox groups in header disallowing them! Name:" << (it).displayName();
        }
        d->mailboxList += (it).mailboxList;
    }
    return true;
}

//-----</MailboxList>-------------------------

//-----<SingleMailbox>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(SingleMailbox, Structured)
//@endcond

QByteArray SingleMailbox::as7BitString() const
{
    const Q_D(SingleMailbox);
    if (isEmpty()) {
        return {};
    }

    return d->mailbox.as7BitString(rfc2047Charset());
}

void SingleMailbox::fromUnicodeString(const QString &s)
{
    Q_D(SingleMailbox);
    from7BitString(encodeRFC2047Sentence(s, rfc2047Charset()));
}

QString SingleMailbox::asUnicodeString() const
{
    Q_D(const SingleMailbox);
    return d->mailbox.prettyAddress();
}

bool SingleMailbox::isEmpty() const
{
    Q_D(const SingleMailbox);
    return !d->mailbox.hasAddress() && !d->mailbox.hasName();
}

Types::Mailbox SingleMailbox::mailbox() const
{
    Q_D(const SingleMailbox);
    return d->mailbox;
}

void SingleMailbox::setMailbox(const Types::Mailbox &mailbox)
{
    Q_D(SingleMailbox);
    d->mailbox = mailbox;
}

bool SingleMailbox::parse(const char *&scursor, const char *const send, NewlineType newline)
{
    Q_D(SingleMailbox);
    Types::Mailbox maybeMailbox;
    if (!parseMailbox(scursor, send, maybeMailbox)) {
        return false;
    }
    eatCFWS(scursor, send, newline);
    if (scursor != send) {
        return false;
    }
    d->mailbox = maybeMailbox;
    return true;
}

//-----</SingleMailbox>-------------------------

//-----<AddressList>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(AddressList, Structured)
kmime_mk_dptr_ctor(AddressList, Structured)
//@endcond

QByteArray AddressList::as7BitString() const
{
    const Q_D(AddressList);
    if (d->addressList.isEmpty()) {
      return {};
    }

    QByteArray rv;
    for (const Types::Address &addr : std::as_const(d->addressList)) {
        const auto mailBoxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailBoxList) {
            rv += mbox.as7BitString(rfc2047Charset());
            rv += ", ";
        }
    }
    rv.resize(rv.length() - 2);
    return rv;
}

void AddressList::fromUnicodeString(const QString &s)
{
    Q_D(AddressList);
    from7BitString(encodeRFC2047Sentence(s, rfc2047Charset()));
}

QString AddressList::asUnicodeString() const
{
    Q_D(const AddressList);
    QStringList rv;
    for (const Types::Address &addr : std::as_const(d->addressList)) {
        rv.reserve(rv.size() + addr.mailboxList.size());
        const auto mailboxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailboxList) {
            rv.append(mbox.prettyAddress());
        }
    }
    return rv.join(QLatin1StringView(", "));
}

bool AddressList::isEmpty() const
{
    return d_func()->addressList.isEmpty();
}

void AddressList::addAddress(const Types::Mailbox &mbox)
{
    Q_D(AddressList);
    Types::Address addr;
    addr.mailboxList.append(mbox);
    d->addressList.append(addr);
}

void AddressList::addAddress(const QByteArray &address,
                             const QString &displayName)
{
    Q_D(AddressList);
    Types::Address addr;
    Types::Mailbox mbox;
    if (stringToMailbox(address, displayName, mbox)) {
        addr.mailboxList.append(mbox);
        d->addressList.append(addr);
    }
}

QList<QByteArray> AddressList::addresses() const {
    QList<QByteArray> rv;
    const auto addressList = d_func()->addressList;
    for (const Types::Address &addr : addressList) {
        const auto mailboxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailboxList) {
            rv.append(mbox.address());
        }
    }
    return rv;
}

QStringList AddressList::displayNames() const
{
    Q_D(const AddressList);
    QStringList rv;
    for (const Types::Address &addr : std::as_const(d->addressList)) {
        const auto mailboxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailboxList) {
            if (mbox.hasName()) {
                rv.append(mbox.name());
            } else {
                rv.append(QString::fromLatin1(mbox.address()));
            }
        }
    }
    return rv;
}

QString AddressList::displayString() const
{
    // optimize for single entry and avoid creation of the QStringList in that case?
    return displayNames().join(QLatin1StringView(", "));
}

QList<Types::Mailbox> AddressList::mailboxes() const
{
    QList<Types::Mailbox> rv;
    const auto addressList = d_func()->addressList;
    for (const Types::Address &addr : addressList) {
        const auto mailboxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailboxList) {
            rv.append(mbox);
        }
    }
    return rv;
}

void AddressList::setAddressList(const QList<Types::Address> &addresses)
{
    Q_D(AddressList);
    d->addressList = addresses;
}

bool AddressList::parse(const char *&scursor, const char *const send,
                        NewlineType newline)
{
    Q_D(AddressList);
    QList<Types::Address> maybeAddressList;
    if (!parseAddressList(scursor, send, maybeAddressList, newline)) {
        return false;
    }

    d->addressList = maybeAddressList;
    return true;
}

//-----</AddressList>-------------------------

//-----<Token>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(Token, Structured)
kmime_mk_dptr_ctor(Token, Structured)
//@endcond

QByteArray Token::as7BitString() const
{
    if (isEmpty()) {
      return {};
    }
    return d_func()->token;
}

bool Token::isEmpty() const
{
    return d_func()->token.isEmpty();
}

QByteArray Token::token() const
{
    return d_func()->token;
}

void Token::setToken(const QByteArray &t)
{
    Q_D(Token);
    d->token = t;
}

bool Token::parse(const char *&scursor, const char *const send, NewlineType newline)
{
    Q_D(Token);
    d->token.clear();
    eatCFWS(scursor, send, newline);
    // must not be empty:
    if (scursor == send) {
        return false;
    }

    QByteArrayView maybeToken;
    if (!parseToken(scursor, send, maybeToken, ParseTokenNoFlag)) {
        return false;
    }
    d->token = maybeToken.toByteArray();

    // complain if trailing garbage is found:
    eatCFWS(scursor, send, newline);
    if (scursor != send) {
        KMIME_WARN << "trailing garbage after token in header allowing "
                   "only a single token!"
                   << Qt::endl;
    }
    return true;
}

//-----</Token>-------------------------

//-----<PhraseList>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(PhraseList, Structured)
//@endcond

QByteArray PhraseList::as7BitString() const
{
    const Q_D(PhraseList);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    for (int i = 0; i < d->phraseList.count(); ++i) {
        // FIXME: only encode when needed, quote when needed, etc.
        rv += encodeRFC2047String(d->phraseList[i], rfc2047Charset(), false);
        if (i != d->phraseList.count() - 1) {
            rv += ", ";
        }
    }

    return rv;
}

QString PhraseList::asUnicodeString() const
{
  return d_func()->phraseList.join(QLatin1StringView(", "));
}

bool PhraseList::isEmpty() const
{
    return d_func()->phraseList.isEmpty();
}

QStringList PhraseList::phrases() const
{
    return d_func()->phraseList;
}

bool PhraseList::parse(const char *&scursor, const char *const send,
                       NewlineType newline)
{
    Q_D(PhraseList);
    d->phraseList.clear();

    while (scursor != send) {
        eatCFWS(scursor, send, newline);
        // empty entry ending the list: OK.
        if (scursor == send) {
            return true;
        }
        // empty entry: ignore.
        if (*scursor == ',') {
            scursor++;
            continue;
        }

        QString maybePhrase;
        if (!parsePhrase(scursor, send, maybePhrase, newline)) {
            return false;
        }
        d->phraseList.append(maybePhrase);

        eatCFWS(scursor, send, newline);
        // non-empty entry ending the list: OK.
        if (scursor == send) {
            return true;
        }
        // comma separating the phrases: eat.
        if (*scursor == ',') {
            scursor++;
        }
    }
    return true;
}

//-----</PhraseList>-------------------------

//-----<DotAtom>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(DotAtom, Structured)
//@endcond

QByteArray DotAtom::as7BitString() const
{
    if (isEmpty()) {
      return {};
    }
    return d_func()->dotAtom;
}

QString DotAtom::asUnicodeString() const
{
    return QString::fromLatin1(d_func()->dotAtom);
}

bool DotAtom::isEmpty() const
{
    return d_func()->dotAtom.isEmpty();
}

bool DotAtom::parse(const char *&scursor, const char *const send,
                    NewlineType newline)
{
    Q_D(DotAtom);
    QByteArrayView maybeDotAtom;
    if (!parseDotAtom(scursor, send, maybeDotAtom, newline)) {
        return false;
    }

    d->dotAtom = maybeDotAtom.toByteArray();

    eatCFWS(scursor, send, newline);
    if (scursor != send) {
        KMIME_WARN << "trailing garbage after dot-atom in header allowing "
                   "only a single dot-atom!"
                   << Qt::endl;
    }
    return true;
}

//-----</DotAtom>-------------------------

//-----<Parametrized>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(Parametrized, Structured)
kmime_mk_dptr_ctor(Parametrized, Structured)
//@endcond

QByteArray Parametrized::as7BitString() const
{
    const Q_D(Parametrized);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    bool first = true;
    for (const auto &it : d->parameterHash) {
        if (!first) {
            rv += "; ";
        } else {
            first = false;
        }
        if (isUsAscii(it.second)) {
            rv += it.first + '=';
            QByteArray tmp = it.second.toLatin1();
            addQuotes(tmp, true);   // force quoting, e.g. for whitespaces in parameter value
            rv += tmp;
        } else {
            rv += it.first + "*=";
            rv += encodeRFC2231String(it.second, rfc2047Charset());
        }
    }

    return rv;
}

QString Parametrized::parameter(QByteArrayView key) const
{
    Q_D(const Parametrized);
    const auto it = d->parameterHash.find(key);
    return it != d->parameterHash.end() ? (*it).second : QString();
}

bool Parametrized::hasParameter(QByteArrayView key) const
{
    return d_func()->parameterHash.contains(key);
}

void Parametrized::setParameter(const QByteArray &key, const QString &value)
{
    Q_D(Parametrized);
    d->parameterHash[key] = value;
}

bool Parametrized::isEmpty() const
{
    return d_func()->parameterHash.empty();
}

bool Parametrized::parse(const char  *&scursor, const char *const send,
                         NewlineType newline)
{
    Q_D(Parametrized);
    d->parameterHash.clear();
    QByteArray charset;
    if (!parseParameterListWithCharset(scursor, send, d->parameterHash, charset, newline)) {
        return false;
    }
    d->encCS = charset;
    return true;
}

//-----</Parametrized>-------------------------

//-----<Ident>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(Ident, Structured)
kmime_mk_dptr_ctor(Ident, Structured)
//@endcond

QByteArray Ident::as7BitString() const
{
    const Q_D(Ident);
    if (d->msgIdList.isEmpty()) {
      return {};
    }

    QByteArray rv;
    for (const Types::AddrSpec &addr : std::as_const(d->msgIdList)) {
        if (!addr.isEmpty()) {
            const QString asString = addr.asString();
            rv += '<';
            if (!asString.isEmpty()) {
                rv += asString.toLatin1(); // FIXME: change parsing to use QByteArrays
            }
            rv += "> ";
        }
    }
    if (!rv.isEmpty()) {
        rv.resize(rv.length() - 1);
    }
    return rv;
}

bool Ident::isEmpty() const
{
    return d_func()->msgIdList.isEmpty();
}

bool Ident::parse(const char *&scursor, const char *const send, NewlineType newline)
{
    Q_D(Ident);
    // msg-id   := "<" id-left "@" id-right ">"
    // id-left  := dot-atom-text / no-fold-quote / local-part
    // id-right := dot-atom-text / no-fold-literal / domain
    //
    // equivalent to:
    // msg-id   := angle-addr

    d->msgIdList.clear();

    while (scursor != send) {
        eatCFWS(scursor, send, newline);
        // empty entry ending the list: OK.
        if (scursor == send) {
            return true;
        }
        // empty entry: ignore.
        if (*scursor == ',') {
            scursor++;
            continue;
        }

        AddrSpec maybeMsgId;
        if (!parseAngleAddr(scursor, send, maybeMsgId, newline)) {
            return false;
        }
        d->msgIdList.append(maybeMsgId);

        eatCFWS(scursor, send, newline);
        // header end ending the list: OK.
        if (scursor == send) {
            return true;
        }
        // regular item separator: eat it.
        if (*scursor == ',') {
            scursor++;
        }
    }
    return true;
}

QList<QByteArray> Ident::identifiers() const {
    QList<QByteArray> rv;
    const auto msgIdList = d_func()->msgIdList;
    for (const Types::AddrSpec &addr : msgIdList) {
        if (!addr.isEmpty()) {
            const QString asString = addr.asString();
            if (!asString.isEmpty()) {
                rv.append(asString.toLatin1());   // FIXME: change parsing to use QByteArrays
            }
        }
    }
    return rv;
}

void Ident::fromIdent(const Ident &ident)
{
    d_func()->encCS = ident.d_func()->encCS;
    d_func()->msgIdList = ident.d_func()->msgIdList;
}

void Ident::appendIdentifier(const QByteArray &id)
{
    Q_D(Ident);
    QByteArray tmp = id;
    if (!tmp.startsWith('<')) {
        tmp.prepend('<');
    }
    if (!tmp.endsWith('>')) {
        tmp.append('>');
    }
    AddrSpec msgId;
    const char *cursor = tmp.constData();
    if (parseAngleAddr(cursor, cursor + tmp.length(), msgId)) {
        d->msgIdList.append(msgId);
    } else {
        qCWarning(KMIME_LOG) << "Unable to parse address spec!";
    }
}

//-----</Ident>-------------------------

//-----<SingleIdent>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(SingleIdent, Structured)
kmime_mk_dptr_ctor(SingleIdent, Structured)
//@endcond

QByteArray SingleIdent::as7BitString() const
{
    Q_D(const SingleIdent);
    if (d->msgId.isEmpty()) {
      return {};
    }

    QByteArray rv;
    const QString asString = d->msgId.asString();
    rv += '<';
    if (!asString.isEmpty()) {
        rv += asString.toLatin1(); // FIXME: change parsing to use QByteArrays
    }
    rv += '>';
    return rv;
}

bool SingleIdent::isEmpty() const
{
    Q_D(const SingleIdent);
    return d->msgId.isEmpty();
}

QByteArray SingleIdent::identifier() const
{
    Q_D(const SingleIdent);
    if (d->msgId.isEmpty()) {
        return {};
    }
    if (d->cachedIdentifier.isEmpty()) {
        const QString asString = d->msgId.asString();
        if (!asString.isEmpty()) {
            d->cachedIdentifier = asString.toLatin1();// FIXME: change parsing to use QByteArrays
        }
    }

    return d->cachedIdentifier;
}

void SingleIdent::setIdentifier(const QByteArray &id)
{
    Q_D(SingleIdent);
    d->msgId = {};
    d->cachedIdentifier.clear();

    QByteArray tmp = id;
    if (!tmp.startsWith('<')) {
        tmp.prepend('<');
    }
    if (!tmp.endsWith('>')) {
        tmp.append('>');
    }
    AddrSpec msgId;
    const char *cursor = tmp.constData();
    if (parseAngleAddr(cursor, cursor + tmp.length(), msgId)) {
        d->msgId = msgId;
    } else {
        qCWarning(KMIME_LOG) << "Unable to parse address spec!";
    }
}

bool SingleIdent::parse(const char *&scursor, const char *const send, NewlineType newline)
{
    Q_D(SingleIdent);

    d->msgId = {};
    d->cachedIdentifier.clear();

    AddrSpec maybeMsgId;
    if (!parseAngleAddr(scursor, send, maybeMsgId, newline)) {
        return false;
    }
    eatCFWS(scursor, send, newline);
    // header end ending the list: OK.
    if (scursor == send) {
        d->msgId = maybeMsgId;
        return true;
    }

    return false;
}

//-----</SingleIdent>-------------------------

} // namespace Generics

//-----<ReturnPath>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ReturnPath, Generics::Structured, Return-Path)
//@endcond

QByteArray ReturnPath::as7BitString() const
{
    if (isEmpty()) {
      return {};
    }

    return '<' + d_func()->mailbox.as7BitString(rfc2047Charset()) + '>';
}

bool ReturnPath::isEmpty() const
{
    const Q_D(ReturnPath);
    return !d->mailbox.hasAddress() && !d->mailbox.hasName();
}

bool ReturnPath::parse(const char *&scursor, const char *const send,
                       NewlineType newline)
{
    Q_D(ReturnPath);
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return false;
    }

    const char *oldscursor = scursor;

    Mailbox maybeMailbox;
    if (!parseMailbox(scursor, send, maybeMailbox, newline)) {
        // mailbox parsing failed, but check for empty brackets:
        scursor = oldscursor;
        if (*scursor != '<') {
            return false;
        }
        scursor++;
        eatCFWS(scursor, send, newline);
        if (scursor == send || *scursor != '>') {
            return false;
        }
        scursor++;

        // prepare a Null mailbox:
        AddrSpec emptyAddrSpec;
        maybeMailbox.setName(QString());
        maybeMailbox.setAddress(emptyAddrSpec);
    } else {
        // check that there was no display-name:
        if (maybeMailbox.hasName()) {
            KMIME_WARN << "display-name \"" << maybeMailbox.name()
                       << "\" in Return-Path!" << Qt::endl;
        }
    }
    d->mailbox = maybeMailbox;

    // see if that was all:
    eatCFWS(scursor, send, newline);
    // and warn if it wasn't:
    if (scursor != send) {
        KMIME_WARN << "trailing garbage after angle-addr in Return-Path!"
                   << Qt::endl;
    }
    return true;
}

//-----</ReturnPath>-------------------------

//-----<Generic>-------------------------------

// NOTE: Do *not* register Generic with HeaderFactory, since its type() is changeable.

Generic::Generic(const char *type, qsizetype len) : Generics::Unstructured(new GenericPrivate)
{
    Q_D(Generic);
    if (type) {
        const auto l = (len < 0 ? strlen(type) : len) + 1;
        d->type = new char[l];
        qstrncpy(d->type, type, l);
    }
}

Generic::~Generic()
{
    Q_D(Generic);
    delete d;
    d_ptr = nullptr;
}

bool Generic::isEmpty() const
{
    return d_func()->type == nullptr || Unstructured::isEmpty();
}

const char *Generic::type() const
{
    return d_func()->type;
}

//-----<Generic>-------------------------------

//-----<MessageID>-----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name(MessageID, Generics::SingleIdent, Message-ID)
//@endcond

void MessageID::generate(const QByteArray &fqdn)
{
    QByteArray suffix = fqdn;
    if (suffix.isEmpty()) {
        qCWarning(KMIME_LOG) << "Unable to generate a Message-ID, falling back to 'localhost.localdomain'.";
        suffix = "local.domain";
    }

    setIdentifier('<' + uniqueString() + '@' + suffix + '>');
}

//-----</MessageID>----------------------------

//-----<Control>-------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Control, Generics::Structured, Control)
//@endcond

QByteArray Control::as7BitString() const
{
    const Q_D(Control);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    rv += d->name;
    if (!d->parameter.isEmpty()) {
        rv += ' ' + d->parameter;
    }
    return rv;
}

bool Control::isEmpty() const
{
    return d_func()->name.isEmpty();
}

QByteArray Control::controlType() const
{
    return d_func()->name;
}

QByteArray Control::parameter() const
{
    return d_func()->parameter;
}

bool Control::isCancel() const
{
    return d_func()->name.toLower() == "cancel";
}

void Control::setCancel(const QByteArray &msgid)
{
    Q_D(Control);
    d->name = "cancel";
    d->parameter = msgid;
}

bool Control::parse(const char *&scursor, const char *const send, NewlineType newline)
{
    Q_D(Control);
    d->name.clear();
    d->parameter.clear();
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return false;
    }
    const char *start = scursor;
    while (scursor != send && !isspace(*scursor)) {
        ++scursor;
    }
    d->name = QByteArray(start, scursor - start);
    eatCFWS(scursor, send, newline);
    d->parameter = QByteArray(scursor, send - scursor);
    return true;
}

//-----</Control>------------------------------

//-----<MailCopiesTo>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(MailCopiesTo,
        Generics::AddressList, Mail-Copies-To)
//@endcond

QByteArray MailCopiesTo::as7BitString() const
{
    QByteArray rv;
    if (!AddressList::isEmpty()) {
        return AddressList::as7BitString();
    }
    if (d_func()->alwaysCopy) {
        return "poster";
    }
    if (d_func()->neverCopy) {
        return "nobody";
    }
    return {};
}

QString MailCopiesTo::asUnicodeString() const
{
    if (!AddressList::isEmpty()) {
        return AddressList::asUnicodeString();
    }
    if (d_func()->alwaysCopy) {
        return QStringLiteral("poster");
    }
    if (d_func()->neverCopy) {
        return QStringLiteral("nobody");
    }
    return {};
}

bool MailCopiesTo::isEmpty() const
{
    return AddressList::isEmpty() && !(d_func()->alwaysCopy || d_func()->neverCopy);
}

bool MailCopiesTo::alwaysCopy() const
{
    return !AddressList::isEmpty() || d_func()->alwaysCopy;
}

void MailCopiesTo::setAlwaysCopy()
{
    Q_D(MailCopiesTo);
    d->addressList.clear();
    d->neverCopy = false;
    d->alwaysCopy = true;
}

bool MailCopiesTo::neverCopy() const
{
    return d_func()->neverCopy;
}

void MailCopiesTo::setNeverCopy()
{
    Q_D(MailCopiesTo);
    d->addressList.clear();
    d->alwaysCopy = false;
    d->neverCopy = true;
}

bool MailCopiesTo::parse(const char  *&scursor, const char *const send,
                         NewlineType newline)
{
    Q_D(MailCopiesTo);
    d->addressList.clear();
    d->alwaysCopy = false;
    d->neverCopy = false;
    if (send - scursor == 5) {
        if (qstrnicmp("never", scursor, 5) == 0) {
            d->neverCopy = true;
            return true;
        }
    }
    if (send - scursor == 6) {
        if (qstrnicmp("always", scursor, 6) == 0 || qstrnicmp("poster", scursor, 6) == 0) {
            d->alwaysCopy = true;
            return true;
        }
        if (qstrnicmp("nobody", scursor, 6) == 0) {
            d->neverCopy = true;
            return true;
        }
    }
    return AddressList::parse(scursor, send, newline);
}

//-----</MailCopiesTo>-------------------------

//-----<Date>----------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Date, Generics::Structured, Date)
//@endcond

QByteArray Date::as7BitString() const
{
    if (isEmpty()) {
      return {};
    }

    //QT5 fix port to QDateTime Qt::RFC2822Date is not enough we need to fix it. We need to use QLocale("C") + add "ddd, ";
    //rv += d_func()->dateTime.toString(  Qt::RFC2822Date ).toLatin1();
    return QLocale::c().toString(d_func()->dateTime, QStringLiteral("ddd, ")).toLatin1()
         + d_func()->dateTime.toString(Qt::RFC2822Date).toLatin1();
}

bool Date::isEmpty() const {
    return d_func()->dateTime.isNull() || !d_func()->dateTime.isValid();
}

QDateTime Date::dateTime() const {
    return d_func()->dateTime;
}

void Date::setDateTime(const QDateTime & dt) {
    Q_D(Date);
    d->dateTime = dt;
}

bool Date::parse(const char *&scursor, const char *const send, NewlineType newline) {
    Q_D(Date);
    const char *start = scursor;
    bool result = parseDateTime(scursor, send, d->dateTime, newline);
    if (!result) {
        result = parseQDateTime(start, send, d->dateTime, newline);
    }
    return result;
}

//-----</Date>---------------------------------

//-----<Newsgroups>----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Newsgroups, Generics::Structured, Newsgroups)
kmime_mk_trivial_ctor_with_name(FollowUpTo, Newsgroups, Followup-To)
//@endcond

QByteArray Newsgroups::as7BitString() const {
    const Q_D(Newsgroups);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    for (int i = 0; i < d->groups.count(); ++i) {
        rv += d->groups[ i ];
        if (i != d->groups.count() - 1) {
            rv += ',';
        }
    }
    return rv;
}

void Newsgroups::fromUnicodeString(const QString & s) {
    Q_D(Newsgroups);
    from7BitString(s.toUtf8());
    d->encCS = cachedCharset("UTF-8");
}

QString Newsgroups::asUnicodeString() const
{
    return QString::fromUtf8(as7BitString());
}

bool Newsgroups::isEmpty() const {
    return d_func()->groups.isEmpty();
}

QList<QByteArray> Newsgroups::groups() const { return d_func()->groups; }

void Newsgroups::setGroups(const QList<QByteArray> &groups) {
    Q_D(Newsgroups);
    d->groups = groups;
}

bool Newsgroups::isCrossposted() const {
    return d_func()->groups.count() >= 2;
}

bool Newsgroups::parse(const char *&scursor, const char *const send, NewlineType newline) {
    Q_D(Newsgroups);
    d->groups.clear();
    HeaderParsing::ParserState state;
    while (true) {
        eatCFWS(scursor, send, newline, state);
        if (scursor != send && *scursor == ',') {
            ++scursor;
            eatCFWS(scursor, send, newline, state);
        }
        if (scursor == send) {
            return true;
        }
        const char *start = scursor;
        while (scursor != send && !isspace(*scursor) && *scursor != ',') {
            ++scursor;
        }
        if (start != scursor) {
            QByteArray group(start, scursor - start);
            d->groups.append(group);
        }
    }
    return true;
}

//-----</Newsgroups>---------------------------

//-----<Lines>---------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Lines, Generics::Structured, Lines)
//@endcond

QByteArray Lines::as7BitString() const
{
    if (isEmpty()) {
        return {};
    }
    return QByteArray::number(d_func()->lines);
}

QString Lines::asUnicodeString() const {
    if (isEmpty()) {
      return {};
    }
    return QString::number(d_func()->lines);
}

bool Lines::isEmpty() const {
    return d_func()->lines == -1;
}

int Lines::numberOfLines() const {
    return d_func()->lines;
}

void Lines::setNumberOfLines(int lines) {
    Q_D(Lines);
    d->lines = lines;
}

bool Lines::parse(const char *&scursor, const char *const send, NewlineType newline) {
    Q_D(Lines);
    eatCFWS(scursor, send, newline);
    if (parseDigits(scursor, send, d->lines)  == 0) {
        d->lines = -1;
        return false;
    }
    return true;
}

//-----</Lines>--------------------------------

//-----<Content-Type>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ContentType, Generics::Parametrized,
            Content-Type)
//@endcond

bool ContentType::isEmpty() const {
    return d_func()->mimeType.isEmpty();
}

QByteArray ContentType::as7BitString() const {
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    rv += mimeType();
    if (!Parametrized::isEmpty()) {
        rv += "; " + Parametrized::as7BitString();
    }

    return rv;
}

QByteArray ContentType::mimeType() const {
    Q_D(const ContentType);
    return d->mimeType;
}

QByteArray ContentType::mediaType() const {
    Q_D(const ContentType);
    const auto pos = d->mimeType.indexOf('/');
    if (pos < 0) {
        return d->mimeType;
    } else {
        return d->mimeType.left(pos);
    }
}

QByteArray ContentType::subType() const {
    Q_D(const ContentType);
    const auto pos = d->mimeType.indexOf('/');
    if (pos < 0) {
      return {};
    } else {
        return d->mimeType.mid(pos + 1);
    }
}

void ContentType::setMimeType(const QByteArray & mimeType) {
    Q_D(ContentType);
    d->mimeType = mimeType;
}

bool ContentType::isMediatype(const char *mediatype) const {
    Q_D(const ContentType);
    const auto len = (qsizetype)strlen(mediatype);
    return qstrnicmp(d->mimeType.constData(), mediatype, len) == 0 &&
            (d->mimeType.at(len) == '/' || d->mimeType.size() == len);
}

bool ContentType::isSubtype(const char *subtype) const {
    Q_D(const ContentType);
    const auto pos = d->mimeType.indexOf('/');
    if (pos < 0) {
        return false;
    }
    const auto len = (qsizetype)strlen(subtype);
    return qstrnicmp(d->mimeType.constData() + pos + 1, subtype, len) == 0 &&
            d->mimeType.size() == pos + len + 1;
}

bool ContentType::isMimeType(const char* mimeType) const
{
    Q_D(const ContentType);
    return qstricmp(d->mimeType.constData(), mimeType) == 0;
}

bool ContentType::isText() const {
    return (isMediatype("text") || isEmpty());
}

bool ContentType::isPlainText() const {
    return (qstricmp(d_func()->mimeType.constData(), "text/plain") == 0 || isEmpty());
}

bool ContentType::isHTMLText() const {
    return qstricmp(d_func()->mimeType.constData(), "text/html") == 0;
}

bool ContentType::isImage() const {
    return isMediatype("image");
}

bool ContentType::isMultipart() const {
    return isMediatype("multipart");
}

bool ContentType::isPartial() const {
    return qstricmp(d_func()->mimeType.constData(), "message/partial") == 0;
}

QByteArray ContentType::charset() const {
    QByteArray ret = parameter("charset").toLatin1();
    if (ret.isEmpty()) {
        //return the default-charset if necessary
        ret = QByteArrayLiteral("UTF-8");
    }
    return ret;
}

void ContentType::setCharset(const QByteArray & s) {
    setParameter(QByteArrayLiteral("charset"), QString::fromLatin1(s));
}

QByteArray ContentType::boundary() const {
    return parameter("boundary").toLatin1();
}

void ContentType::setBoundary(const QByteArray & s) {
    setParameter(QByteArrayLiteral("boundary"), QString::fromLatin1(s));
}

QString ContentType::name() const {
    return parameter("name");
}

void ContentType::setName(const QString & s) {
    Q_D(ContentType);
    setParameter(QByteArrayLiteral("name"), s);
}

QByteArray ContentType::id() const {
    return parameter("id").toLatin1();
}

void ContentType::setId(const QByteArray & s) {
    setParameter(QByteArrayLiteral("id"), QString::fromLatin1(s));
}

int ContentType::partialNumber() const {
    QByteArray p = parameter("number").toLatin1();
    if (!p.isEmpty()) {
        return p.toInt();
    } else {
        return -1;
    }
}

void ContentType::setPartialNumber(int number)
{
    setParameter(QByteArrayLiteral("number"), QString::number(number));
}

int ContentType::partialCount() const {
    QByteArray p = parameter("total").toLatin1();
    if (!p.isEmpty()) {
        return p.toInt();
    } else {
        return -1;
    }
}

void ContentType::setPartialCount(int total)
{
    setParameter(QByteArrayLiteral("total"), QString::number(total));
}

bool ContentType::parse(const char *&scursor, const char *const send,
                        NewlineType newline) {
    Q_D(ContentType);
    // content-type: type "/" subtype *(";" parameter)
    d->mimeType.clear();
    d->parameterHash.clear();
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return false; // empty header
    }

    // type
    QByteArrayView maybeMimeType;
    if (!parseToken(scursor, send, maybeMimeType, ParseTokenNoFlag)) {
        return false;
    }
    // subtype
    eatCFWS(scursor, send, newline);
    if (scursor == send || *scursor != '/') {
        return false;
    }
    scursor++;
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return false;
    }
    QByteArrayView maybeSubType;
    if (!parseToken(scursor, send, maybeSubType, ParseTokenNoFlag)) {
        return false;
    }

    d->mimeType.reserve(maybeMimeType.size() + maybeSubType.size() + 1);
    d->mimeType.append(maybeMimeType);
    d->mimeType.append('/');
    d->mimeType.append(maybeSubType);
    d->mimeType = std::move(d->mimeType).toLower();

    // parameter list
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return true; // no parameters
    }
    if (*scursor != ';') {
        return false;
    }
    scursor++;

    if (!Parametrized::parse(scursor, send, newline)) {
        return false;
    }

    return true;
}

//-----</Content-Type>-------------------------

//-----<ContentID>----------------------

kmime_mk_trivial_ctor_with_name_and_dptr(ContentID, SingleIdent, Content-ID)
kmime_mk_dptr_ctor(ContentID, SingleIdent)

bool ContentID::parse(const char *&scursor, const char *const send, NewlineType newline) {
    Q_D(ContentID);
    // Content-id := "<" contentid ">"
    // contentid := now whitespaces

    const char *origscursor = scursor;
    if (!SingleIdent::parse(scursor, send, newline)) {
        scursor = origscursor;
        d->msgId = {};
        d->cachedIdentifier.clear();

        while (scursor != send) {
            eatCFWS(scursor, send, newline);
            // empty entry ending the list: OK.
            if (scursor == send) {
                return true;
            }
            // empty entry: ignore.
            if (*scursor == ',') {
                scursor++;
                continue;
            }

            AddrSpec maybeContentId;
            // Almost parseAngleAddr
            if (scursor == send || *scursor != '<') {
                return false;
            }
            scursor++; // eat '<'

            eatCFWS(scursor, send, newline);
            if (scursor == send) {
                return false;
            }

            // Save chars until '>''
            QByteArrayView result;
            if (!parseDotAtom(scursor, send, result, NewlineType::LF)) {
                return false;
            }

            eatCFWS(scursor, send, newline);
            if (scursor == send || *scursor != '>') {
                return false;
            }
            scursor++;
            // /Almost parseAngleAddr

            maybeContentId.localPart = QString::fromLatin1(result); // FIXME: just use QByteArray instead of AddrSpec in msgIdList?
            d->msgId = maybeContentId;

            eatCFWS(scursor, send, newline);
            // header end ending the list: OK.
            if (scursor == send) {
                return true;
            }
            // regular item separator: eat it.
            if (*scursor == ',') {
                scursor++;
            }
        }
        return true;
    } else {
        return true;
    }
}

//-----</ContentID>----------------------

//-----<ContentTransferEncoding>----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ContentTransferEncoding,
        Generics::Token, Content-Transfer-Encoding)
//@endcond

struct {
    const char *s;
    contentEncoding e;
} constexpr inline const encTable[] = {
    { "7Bit", CE7Bit },
    { "8Bit", CE8Bit },
    { "quoted-printable", CEquPr },
    { "base64", CEbase64 },
    { "x-uuencode", CEuuenc },
    { "binary", CEbinary },
};

bool ContentTransferEncoding::isEmpty() const
{
    return false;
}

QByteArray ContentTransferEncoding::as7BitString() const
{
    Q_D(const ContentTransferEncoding);

    if (d->token.isEmpty()) {
        for (const auto &enc : encTable) {
            if (d->cte == enc.e) {
                return QByteArray(enc.s);
            }
        }
    }

    return d->token;
}

contentEncoding ContentTransferEncoding::encoding() const
{
    return d_func()->cte;
}

void ContentTransferEncoding::setEncoding(contentEncoding e)
{
    Q_D(ContentTransferEncoding);
    d->cte = e;
    d->token.clear();
}

bool ContentTransferEncoding::parse(const char  *&scursor, const char *const send, NewlineType newline)
{
    Q_D(ContentTransferEncoding);
    setEncoding(CE7Bit);

    eatCFWS(scursor, send, newline);
    // must not be empty:
    if (scursor == send) {
        return false;
    }

    QByteArrayView token;
    if (!parseToken(scursor, send, token, ParseTokenNoFlag)) {
        return false;
    }

    for (const auto &enc : encTable) {
        if (token.compare(enc.s, Qt::CaseInsensitive) == 0) {
            d->cte = enc.e;
            return true;
        }
    }

    d->token = token.toByteArray();
    return true;
}

//-----</ContentTransferEncoding>---------------------------

//-----<ContentDisposition>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ContentDisposition,
        Generics::Parametrized, Content-Disposition)
//@endcond

QByteArray ContentDisposition::as7BitString() const {
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (d_func()->disposition == CDattachment) {
        rv += "attachment";
    } else if (d_func()->disposition == CDinline) {
        rv += "inline";
    } else {
      return {};
    }

    if (!Parametrized::isEmpty()) {
        rv += "; " + Parametrized::as7BitString();
    }

    return rv;
}

bool ContentDisposition::isEmpty() const {
    return d_func()->disposition == CDInvalid;
}

contentDisposition ContentDisposition::disposition() const {
    return d_func()->disposition;
}

void ContentDisposition::setDisposition(contentDisposition disp) {
    Q_D(ContentDisposition);
    d->disposition = disp;
}

QString KMime::Headers::ContentDisposition::filename() const {
    return parameter("filename");
}

void ContentDisposition::setFilename(const QString & filename) {
    setParameter(QByteArrayLiteral("filename"), filename);
}

bool ContentDisposition::parse(const char  *&scursor, const char *const send,
                                NewlineType newline) {
    Q_D(ContentDisposition);
    d->parameterHash.clear();
    d->disposition = CDInvalid;

    // token
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return false;
    }

    QByteArrayView token;
    if (!parseToken(scursor, send, token, ParseTokenNoFlag)) {
        return false;
    }

    if (token.compare("inline", Qt::CaseInsensitive) == 0) {
        d->disposition = CDinline;
    } else if (token.compare("attachment", Qt::CaseInsensitive) == 0) {
        d->disposition = CDattachment;
    } else {
        return false;
    }

    // parameter list
    eatCFWS(scursor, send, newline);
    if (scursor == send) {
        return true; // no parameters
    }

    if (*scursor != ';') {
        return false;
    }
    scursor++;

    return Parametrized::parse(scursor, send, newline);
}

//-----</ContentDisposition>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name(Subject, Generics::Unstructured, Subject)
//@endcond

std::unique_ptr<Base> createHeader(QByteArrayView type)
{
    auto h = HeaderFactory::createHeader(type);
    return h ? std::move(h) : std::make_unique<Headers::Generic>(type.constData(), type.size());
}

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name(ContentDescription,
                                Generics::Unstructured, Content-Description)
kmime_mk_trivial_ctor_with_name(ContentLocation,
                                Generics::Unstructured, Content-Location)
kmime_mk_trivial_ctor_with_name(From, Generics::MailboxList, From)
kmime_mk_trivial_ctor_with_name(Sender, Generics::SingleMailbox, Sender)
kmime_mk_trivial_ctor_with_name(To, Generics::AddressList, To)
kmime_mk_trivial_ctor_with_name(Cc, Generics::AddressList, Cc)
kmime_mk_trivial_ctor_with_name(Bcc, Generics::AddressList, Bcc)
kmime_mk_trivial_ctor_with_name(ReplyTo, Generics::AddressList, Reply-To)
kmime_mk_trivial_ctor_with_name(Keywords, Generics::PhraseList, Keywords)
kmime_mk_trivial_ctor_with_name(MIMEVersion, Generics::DotAtom, MIME-Version)
kmime_mk_trivial_ctor_with_name(Supersedes, Generics::SingleIdent, Supersedes)
kmime_mk_trivial_ctor_with_name(InReplyTo, Generics::Ident, In-Reply-To)
kmime_mk_trivial_ctor_with_name(References, Generics::Ident, References)
kmime_mk_trivial_ctor_with_name(Organization, Generics::Unstructured, Organization)
kmime_mk_trivial_ctor_with_name(UserAgent, Generics::Unstructured, User-Agent)
//@endcond

} // namespace Headers

} // namespace KMime
