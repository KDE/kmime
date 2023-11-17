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

#include "kmime_headers.h"
#include "kmime_headers_p.h"

#include "kmime_util.h"
#include "kmime_util_p.h"
#include "kmime_codecs.h"
#include "kmime_content.h"
#include "kmime_headerfactory_p.h"
#include "kmime_debug.h"
#include "kmime_warning.h"

#include <KCodecs>

#include <cassert>
#include <cctype>

// macro to generate a default constructor implementation
#define kmime_mk_trivial_ctor( subclass, baseclass )                  \
    subclass::subclass() : baseclass()           \
    {                                                                     \
    }                                                                     \
    \
    subclass::~subclass() {}

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

void Base::from7BitString(const char *s, size_t len)
{
    from7BitString(QByteArray::fromRawData(s, len));
}

QByteArray Base::rfc2047Charset() const
{
    if (d_ptr->encCS.isEmpty()) {
        return Content::defaultCharset();
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

bool Base::is(const char *t) const
{
    return qstricmp(t, type()) == 0;
}

bool Base::isMimeHeader() const
{
    return qstrnicmp(type(), "Content-", 8) == 0;
}

QByteArray Base::typeIntro() const
{
    return QByteArray(type()) + ": ";
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

void Unstructured::from7BitString(const QByteArray &s)
{
    Q_D(Unstructured);
    d->decoded = KCodecs::decodeRFC2047String(s, &d->encCS, Content::defaultCharset());
}

QByteArray Unstructured::as7BitString(bool withHeaderType) const
{
    const Q_D(Unstructured);
    QByteArray result;
    if (withHeaderType) {
        result = typeIntro();
    }
    result += encodeRFC2047String(d->decoded, d->encCS) ;

    return result;
}

void Unstructured::fromUnicodeString(const QString &s, const QByteArray &b)
{
    Q_D(Unstructured);
    d->decoded = s;
    d->encCS = cachedCharset(b);
}

QString Unstructured::asUnicodeString() const
{
    return d_func()->decoded;
}

void Unstructured::clear()
{
    Q_D(Unstructured);
    d->decoded.truncate(0);
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


void Structured::from7BitString(const char *s, size_t len)
{
    Q_D(Structured);
    if (d->encCS.isEmpty()) {
        d->encCS = Content::defaultCharset();
    }
    parse(s, s + len);
}

void Structured::from7BitString(const QByteArray &s)
{
#if 0
    Q_D(Structured);
    //Bug about mailto with space which are replaced by "_" so it failed to parse
    //=> we reconvert to correct encoding as RFC2047
    const QString str = KCodecs::decodeRFC2047String(s, &d->encCS, Content::defaultCharset());
    const QByteArray ba = KCodecs::encodeRFC2047String(str, d->encCS);
    from7BitString(ba.constData(), ba.length());
#else
    from7BitString(s.constData(), s.length());
#endif
}

QString Structured::asUnicodeString() const
{
    return QString::fromLatin1(as7BitString(false));
}

void Structured::fromUnicodeString(const QString &s, const QByteArray &b)
{
    Q_D(Structured);
    d->encCS = cachedCharset(b);
    from7BitString(s.toLatin1());
}

//-----</Structured>-------------------------

//-----<Address>-------------------------

Address::Address() : Structured(new AddressPrivate)
{
}

kmime_mk_dptr_ctor(Address, Structured)

    Address::~Address() = default;

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

//-----</Address>-------------------------

//-----<MailboxList>-------------------------

kmime_mk_trivial_ctor_with_dptr(MailboxList, Address)
kmime_mk_dptr_ctor(MailboxList, Address)

QByteArray MailboxList::as7BitString(bool withHeaderType) const
{
    const Q_D(MailboxList);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv = typeIntro();
    }
    for (const Types::Mailbox &mbox : std::as_const(d->mailboxList)) {
        rv += mbox.as7BitString(d->encCS);
        rv += ", ";
    }
    rv.resize(rv.length() - 2);
    return rv;
}

void MailboxList::fromUnicodeString(const QString &s, const QByteArray &b)
{
    Q_D(MailboxList);
    d->encCS = cachedCharset(b);
    from7BitString(encodeRFC2047Sentence(s, b));
}

QString MailboxList::asUnicodeString() const
{
    Q_D(const MailboxList);
    return Mailbox::listToUnicodeString(d->mailboxList);
}

void MailboxList::clear()
{
    Q_D(MailboxList);
    d->mailboxList.clear();
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
    return displayNames().join(QLatin1String(", "));
}

Types::Mailbox::List MailboxList::mailboxes() const
{
    return d_func()->mailboxList;
}

bool MailboxList::parse(const char *&scursor, const char *const send,
                        bool isCRLF)
{
    Q_D(MailboxList);
    // examples:
    // from := "From:" mailbox-list CRLF
    // sender := "Sender:" mailbox CRLF

    // parse an address-list:
    QList<Types::Address> maybeAddressList;
    if (!parseAddressList(scursor, send, maybeAddressList, isCRLF)) {
        return false;
    }

    d->mailboxList.clear();
    d->mailboxList.reserve(maybeAddressList.count());

    // extract the mailboxes and complain if there are groups:
    for (const auto &it : std::as_const(maybeAddressList)) {
        if (!(it).displayName.isEmpty()) {
            KMIME_WARN << "mailbox groups in header disallowing them! Name: \""
                       << (it).displayName << "\""
                       << Qt::endl
                          ;
        }
        d->mailboxList += (it).mailboxList;
    }
    return true;
}

//-----</MailboxList>-------------------------

//-----<SingleMailbox>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(SingleMailbox, MailboxList)
//@endcond

bool SingleMailbox::parse(const char *&scursor, const char *const send,
                          bool isCRLF)
{
    Q_D(MailboxList);
    if (!MailboxList::parse(scursor, send, isCRLF)) {
        return false;
    }

    if (d->mailboxList.count() > 1) {
        KMIME_WARN << "multiple mailboxes in header allowing only a single one!"
                   << Qt::endl;
    }
    return true;
}

//-----</SingleMailbox>-------------------------

//-----<AddressList>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(AddressList, Address)
kmime_mk_dptr_ctor(AddressList, Address)
//@endcond

QByteArray AddressList::as7BitString(bool withHeaderType) const
{
    const Q_D(AddressList);
    if (d->addressList.isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv = typeIntro();
    }
    for (const Types::Address &addr : std::as_const(d->addressList)) {
        const auto mailBoxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailBoxList) {
            rv += mbox.as7BitString(d->encCS);
            rv += ", ";
        }
    }
    rv.resize(rv.length() - 2);
    return rv;
}

void AddressList::fromUnicodeString(const QString &s, const QByteArray &b)
{
    Q_D(AddressList);
    d->encCS = cachedCharset(b);
    from7BitString(encodeRFC2047Sentence(s, b));
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
    return rv.join(QLatin1String(", "));
}

void AddressList::clear()
{
    Q_D(AddressList);
    d->addressList.clear();
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
    return displayNames().join(QLatin1String(", "));
}

Types::Mailbox::List AddressList::mailboxes() const
{
    Types::Mailbox::List rv;
    const auto addressList = d_func()->addressList;
    for (const Types::Address &addr : addressList) {
        const auto mailboxList = addr.mailboxList;
        for (const Types::Mailbox &mbox : mailboxList) {
            rv.append(mbox);
        }
    }
    return rv;
}

bool AddressList::parse(const char *&scursor, const char *const send,
                        bool isCRLF)
{
    Q_D(AddressList);
    QList<Types::Address> maybeAddressList;
    if (!parseAddressList(scursor, send, maybeAddressList, isCRLF)) {
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

QByteArray Token::as7BitString(bool withHeaderType) const
{
    if (isEmpty()) {
      return {};
    }
    if (withHeaderType) {
        return typeIntro() + d_func()->token;
    }
    return d_func()->token;
}

void Token::clear()
{
    Q_D(Token);
    d->token.clear();
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

bool Token::parse(const char *&scursor, const char *const send, bool isCRLF)
{
    Q_D(Token);
    clear();
    eatCFWS(scursor, send, isCRLF);
    // must not be empty:
    if (scursor == send) {
        return false;
    }

    QPair<const char *, int> maybeToken;
    if (!parseToken(scursor, send, maybeToken, ParseTokenNoFlag)) {
        return false;
    }
    d->token = QByteArray(maybeToken.first, maybeToken.second);

    // complain if trailing garbage is found:
    eatCFWS(scursor, send, isCRLF);
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

QByteArray PhraseList::as7BitString(bool withHeaderType) const
{
    const Q_D(PhraseList);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv = typeIntro();
    }

    for (int i = 0; i < d->phraseList.count(); ++i) {
        // FIXME: only encode when needed, quote when needed, etc.
        rv += encodeRFC2047String(d->phraseList[i], d->encCS, false, false);
        if (i != d->phraseList.count() - 1) {
            rv += ", ";
        }
    }

    return rv;
}

QString PhraseList::asUnicodeString() const
{
    return d_func()->phraseList.join(QLatin1String(", "));
}

void PhraseList::clear()
{
    Q_D(PhraseList);
    d->phraseList.clear();
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
                       bool isCRLF)
{
    Q_D(PhraseList);
    d->phraseList.clear();

    while (scursor != send) {
        eatCFWS(scursor, send, isCRLF);
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
        if (!parsePhrase(scursor, send, maybePhrase, isCRLF)) {
            return false;
        }
        d->phraseList.append(maybePhrase);

        eatCFWS(scursor, send, isCRLF);
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

QByteArray DotAtom::as7BitString(bool withHeaderType) const
{
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    rv += d_func()->dotAtom;
    return rv;
}

QString DotAtom::asUnicodeString() const
{
    return QString::fromLatin1(d_func()->dotAtom);
}

void DotAtom::clear()
{
    Q_D(DotAtom);
    d->dotAtom.clear();
}

bool DotAtom::isEmpty() const
{
    return d_func()->dotAtom.isEmpty();
}

bool DotAtom::parse(const char *&scursor, const char *const send,
                    bool isCRLF)
{
    Q_D(DotAtom);
    QByteArray maybeDotAtom;
    if (!parseDotAtom(scursor, send, maybeDotAtom, isCRLF)) {
        return false;
    }

    d->dotAtom = maybeDotAtom;

    eatCFWS(scursor, send, isCRLF);
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

QByteArray Parametrized::as7BitString(bool withHeaderType) const
{
    const Q_D(Parametrized);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    bool first = true;
    for (QMap<QString, QString>::ConstIterator it = d->parameterHash.constBegin();
            it != d->parameterHash.constEnd(); ++it) {
        if (!first) {
            rv += "; ";
        } else {
            first = false;
        }
        if (isUsAscii(it.value())) {
            rv += it.key().toLatin1() + '=';
            QByteArray tmp = it.value().toLatin1();
            addQuotes(tmp, true);   // force quoting, eg. for whitespaces in parameter value
            rv += tmp;
        } else {
            rv += it.key().toLatin1() + "*=";
            rv += encodeRFC2231String(it.value(), d->encCS);
        }
    }

    return rv;
}

QString Parametrized::parameter(const QString &key) const
{
    return d_func()->parameterHash.value(key.toLower());
}

bool Parametrized::hasParameter(const QString &key) const
{
    return d_func()->parameterHash.contains(key.toLower());
}

void Parametrized::setParameter(const QString &key, const QString &value)
{
    Q_D(Parametrized);
    d->parameterHash.insert(key.toLower(), value);
}

bool Parametrized::isEmpty() const
{
    return d_func()->parameterHash.isEmpty();
}

void Parametrized::clear()
{
    Q_D(Parametrized);
    d->parameterHash.clear();
}

bool Parametrized::parse(const char  *&scursor, const char *const send,
                         bool isCRLF)
{
    Q_D(Parametrized);
    d->parameterHash.clear();
    QByteArray charset;
    if (!parseParameterListWithCharset(scursor, send, d->parameterHash, charset, isCRLF)) {
        return false;
    }
    d->encCS = charset;
    return true;
}

//-----</Parametrized>-------------------------

//-----<Ident>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_dptr(Ident, Address)
kmime_mk_dptr_ctor(Ident, Address)
//@endcond

QByteArray Ident::as7BitString(bool withHeaderType) const
{
    const Q_D(Ident);
    if (d->msgIdList.isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv = typeIntro();
    }
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

void Ident::clear()
{
    Q_D(Ident);
    d->msgIdList.clear();
    d->cachedIdentifier.clear();
}

bool Ident::isEmpty() const
{
    return d_func()->msgIdList.isEmpty();
}

bool Ident::parse(const char *&scursor, const char *const send, bool isCRLF)
{
    Q_D(Ident);
    // msg-id   := "<" id-left "@" id-right ">"
    // id-left  := dot-atom-text / no-fold-quote / local-part
    // id-right := dot-atom-text / no-fold-literal / domain
    //
    // equivalent to:
    // msg-id   := angle-addr

    d->msgIdList.clear();
    d->cachedIdentifier.clear();

    while (scursor != send) {
        eatCFWS(scursor, send, isCRLF);
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
        if (!parseAngleAddr(scursor, send, maybeMsgId, isCRLF)) {
            return false;
        }
        d->msgIdList.append(maybeMsgId);

        eatCFWS(scursor, send, isCRLF);
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

void Ident::fromIdent(const Ident* ident)
{
    d_func()->encCS = ident->d_func()->encCS;
    d_func()->msgIdList = ident->d_func()->msgIdList;
    d_func()->cachedIdentifier = ident->d_func()->cachedIdentifier;
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
kmime_mk_trivial_ctor_with_dptr(SingleIdent, Ident)
kmime_mk_dptr_ctor(SingleIdent, Ident)
//@endcond

QByteArray SingleIdent::identifier() const
{
    if (d_func()->msgIdList.isEmpty()) {
      return {};
    }

    if (d_func()->cachedIdentifier.isEmpty()) {
        const Types::AddrSpec &addr = d_func()->msgIdList.first();
        if (!addr.isEmpty()) {
            const QString asString = addr.asString();
            if (!asString.isEmpty()) {
                d_func()->cachedIdentifier = asString.toLatin1();// FIXME: change parsing to use QByteArrays
            }
        }
    }

    return d_func()->cachedIdentifier;
}

void SingleIdent::setIdentifier(const QByteArray &id)
{
    Q_D(SingleIdent);
    d->msgIdList.clear();
    d->cachedIdentifier.clear();
    appendIdentifier(id);
}

bool SingleIdent::parse(const char *&scursor, const char *const send,
                        bool isCRLF)
{
    Q_D(SingleIdent);
    if (!Ident::parse(scursor, send, isCRLF)) {
        return false;
    }

    if (d->msgIdList.count() > 1) {
        KMIME_WARN << "more than one msg-id in header "
                   << "allowing only a single one!"
                   << Qt::endl;
    }
    return true;
}

//-----</SingleIdent>-------------------------

} // namespace Generics

//-----<ReturnPath>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ReturnPath, Generics::Address, Return-Path)
//@endcond

QByteArray ReturnPath::as7BitString(bool withHeaderType) const
{
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }
    rv += '<' + d_func()->mailbox.as7BitString(d_func()->encCS) + '>';
    return rv;
}

void ReturnPath::clear()
{
    Q_D(ReturnPath);
    d->mailbox.setAddress(Types::AddrSpec());
    d->mailbox.setName(QString());
}

bool ReturnPath::isEmpty() const
{
    const Q_D(ReturnPath);
    return !d->mailbox.hasAddress() && !d->mailbox.hasName();
}

bool ReturnPath::parse(const char *&scursor, const char *const send,
                       bool isCRLF)
{
    Q_D(ReturnPath);
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return false;
    }

    const char *oldscursor = scursor;

    Mailbox maybeMailbox;
    if (!parseMailbox(scursor, send, maybeMailbox, isCRLF)) {
        // mailbox parsing failed, but check for empty brackets:
        scursor = oldscursor;
        if (*scursor != '<') {
            return false;
        }
        scursor++;
        eatCFWS(scursor, send, isCRLF);
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
    eatCFWS(scursor, send, isCRLF);
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

Generic::Generic() : Generics::Unstructured(new GenericPrivate)
{
}

Generic::Generic(const char *t, int len) : Generics::Unstructured(new GenericPrivate)
{
    setType(t, len);
}

Generic::~Generic()
{
    Q_D(Generic);
    delete d;
    d_ptr = nullptr;
}

void Generic::clear()
{
    Q_D(Generic);
    delete[] d->type;
    d->type = nullptr;
    Unstructured::clear();
}

bool Generic::isEmpty() const
{
    return d_func()->type == nullptr || Unstructured::isEmpty();
}

const char *Generic::type() const
{
    return d_func()->type;
}

void Generic::setType(const char *type, int len)
{
    Q_D(Generic);
    if (d->type) {
        delete[] d->type;
    }
    if (type) {
        const int l = (len < 0 ? strlen(type) : len) + 1;
        d->type = new char[l];
        qstrncpy(d->type, type, l);
    } else {
        d->type = nullptr;
    }
}

//-----<Generic>-------------------------------

//-----<MessageID>-----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name(MessageID, Generics::SingleIdent, Message-ID)
//@endcond

void MessageID::generate(const QByteArray &fqdn)
{
    setIdentifier('<' + uniqueString() + '@' + fqdn + '>');
}

//-----</MessageID>----------------------------

//-----<Control>-------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Control, Generics::Structured, Control)
//@endcond

QByteArray Control::as7BitString(bool withHeaderType) const
{
    const Q_D(Control);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    rv += d->name;
    if (!d->parameter.isEmpty()) {
        rv += ' ' + d->parameter;
    }
    return rv;
}

void Control::clear()
{
    Q_D(Control);
    d->name.clear();
    d->parameter.clear();
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

bool Control::parse(const char *&scursor, const char *const send, bool isCRLF)
{
    Q_D(Control);
    clear();
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return false;
    }
    const char *start = scursor;
    while (scursor != send && !isspace(*scursor)) {
        ++scursor;
    }
    d->name = QByteArray(start, scursor - start);
    eatCFWS(scursor, send, isCRLF);
    d->parameter = QByteArray(scursor, send - scursor);
    return true;
}

//-----</Control>------------------------------

//-----<MailCopiesTo>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(MailCopiesTo,
        Generics::AddressList, Mail-Copies-To)
//@endcond

QByteArray MailCopiesTo::as7BitString(bool withHeaderType) const
{
    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }
    if (!AddressList::isEmpty()) {
        rv += AddressList::as7BitString(false);
    } else {
        if (d_func()->alwaysCopy) {
            rv += "poster";
        } else if (d_func()->neverCopy) {
            rv += "nobody";
        }
    }
    return rv;
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

void MailCopiesTo::clear()
{
    Q_D(MailCopiesTo);
    AddressList::clear();
    d->alwaysCopy = false;
    d->neverCopy = false;
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
    clear();
    d->alwaysCopy = true;
}

bool MailCopiesTo::neverCopy() const
{
    return d_func()->neverCopy;
}

void MailCopiesTo::setNeverCopy()
{
    Q_D(MailCopiesTo);
    clear();
    d->neverCopy = true;
}

bool MailCopiesTo::parse(const char  *&scursor, const char *const send,
                         bool isCRLF)
{
    Q_D(MailCopiesTo);
    clear();
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
    return AddressList::parse(scursor, send, isCRLF);
}

//-----</MailCopiesTo>-------------------------

//-----<Date>----------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Date, Generics::Structured, Date)
//@endcond

QByteArray Date::as7BitString(bool withHeaderType) const
{
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }
    //QT5 fix port to QDateTime Qt::RFC2822Date is not enough we need to fix it. We need to use QLocale("C") + add "ddd, ";
    //rv += d_func()->dateTime.toString(  Qt::RFC2822Date ).toLatin1();
    rv += QLocale::c().toString(d_func()->dateTime, QStringLiteral("ddd, ")).toLatin1();
    rv += d_func()->dateTime.toString(Qt::RFC2822Date).toLatin1();

    return rv;
}

void Date::clear() {
    Q_D(Date);
    d->dateTime = QDateTime();
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

int Date::ageInDays() const {
    const QDate today = QDate::currentDate();
    return dateTime().date().daysTo(today);
}

bool Date::parse(const char *&scursor, const char *const send, bool isCRLF) {
    Q_D(Date);
    const char *start = scursor;
    bool result = parseDateTime(scursor, send, d->dateTime, isCRLF);
    if (!result) {
        result = parseQDateTime(start, send, d->dateTime, isCRLF);
    }
    return result;
}

//-----</Date>---------------------------------

//-----<Newsgroups>----------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Newsgroups, Generics::Structured, Newsgroups)
kmime_mk_trivial_ctor_with_name(FollowUpTo, Newsgroups, Followup-To)
//@endcond

QByteArray Newsgroups::as7BitString(bool withHeaderType) const {
    const Q_D(Newsgroups);
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    for (int i = 0; i < d->groups.count(); ++i) {
        rv += d->groups[ i ];
        if (i != d->groups.count() - 1) {
            rv += ',';
        }
    }
    return rv;
}

void Newsgroups::fromUnicodeString(const QString & s, const QByteArray & b) {
    Q_UNUSED(b)
    Q_D(Newsgroups);
    from7BitString(s.toUtf8());
    d->encCS = cachedCharset("UTF-8");
}

QString Newsgroups::asUnicodeString() const {
    return QString::fromUtf8(as7BitString(false));
}

void Newsgroups::clear() {
    Q_D(Newsgroups);
    d->groups.clear();
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

bool Newsgroups::parse(const char *&scursor, const char *const send, bool isCRLF) {
    Q_D(Newsgroups);
    clear();
    while (true) {
      eatCFWS(scursor, send, isCRLF);
      if (scursor != send && *scursor == ',') {
        ++scursor;
      }
      eatCFWS(scursor, send, isCRLF);
      if (scursor == send) {
        return true;
      }
      const char *start = scursor;
      while (scursor != send && !isspace(*scursor) && *scursor != ',') {
        ++scursor;
      }
      QByteArray group(start, scursor - start);
      d->groups.append(group);
    }
    return true;
}

//-----</Newsgroups>---------------------------

//-----<Lines>---------------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(Lines, Generics::Structured, Lines)
//@endcond

QByteArray Lines::as7BitString(bool withHeaderType) const {
    if (isEmpty()) {
      return {};
    }

    QByteArray num;
    num.setNum(d_func()->lines);

    if (withHeaderType) {
        return typeIntro() + num;
    }
    return num;
}

QString Lines::asUnicodeString() const {
    if (isEmpty()) {
      return {};
    }
    return QString::number(d_func()->lines);
}

void Lines::clear() {
    Q_D(Lines);
    d->lines = -1;
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

bool Lines::parse(const char *&scursor, const char *const send, bool isCRLF) {
    Q_D(Lines);
    eatCFWS(scursor, send, isCRLF);
    if (parseDigits(scursor, send, d->lines)  == 0) {
        clear();
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

void ContentType::clear() {
    Q_D(ContentType);
    d->category = CCsingle;
    d->mimeType.clear();
    Parametrized::clear();
}

QByteArray ContentType::as7BitString(bool withHeaderType) const {
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    rv += mimeType();
    if (!Parametrized::isEmpty()) {
        rv += "; " + Parametrized::as7BitString(false);
    }

    return rv;
}

QByteArray ContentType::mimeType() const {
    Q_D(const ContentType);
    return d->mimeType;
}

QByteArray ContentType::mediaType() const {
    Q_D(const ContentType);
    const int pos = d->mimeType.indexOf('/');
    if (pos < 0) {
        return d->mimeType;
    } else {
        return d->mimeType.left(pos);
    }
}

QByteArray ContentType::subType() const {
    Q_D(const ContentType);
    const int pos = d->mimeType.indexOf('/');
    if (pos < 0) {
      return {};
    } else {
        return d->mimeType.mid(pos + 1);
    }
}

void ContentType::setMimeType(const QByteArray & mimeType) {
    Q_D(ContentType);
    d->mimeType = mimeType;

    if (isMultipart()) {
        d->category = CCcontainer;
    } else {
        d->category = CCsingle;
    }
}

bool ContentType::isMediatype(const char *mediatype) const {
    Q_D(const ContentType);
    const int len = strlen(mediatype);
    return qstrnicmp(d->mimeType.constData(), mediatype, len) == 0 &&
            (d->mimeType.at(len) == '/' || d->mimeType.size() == len);
}

bool ContentType::isSubtype(const char *subtype) const {
    Q_D(const ContentType);
    const int pos = d->mimeType.indexOf('/');
    if (pos < 0) {
        return false;
    }
    const int len = strlen(subtype);
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
    QByteArray ret = parameter(QStringLiteral("charset")).toLatin1();
    if (ret.isEmpty()) {
        //return the default-charset if necessary
        ret = Content::defaultCharset();
    }
    return ret;
}

void ContentType::setCharset(const QByteArray & s) {
    setParameter(QStringLiteral("charset"), QString::fromLatin1(s));
}

QByteArray ContentType::boundary() const {
    return parameter(QStringLiteral("boundary")).toLatin1();
}

void ContentType::setBoundary(const QByteArray & s) {
    setParameter(QStringLiteral("boundary"), QString::fromLatin1(s));
}

QString ContentType::name() const {
    return parameter(QStringLiteral("name"));
}

void ContentType::setName(const QString & s, const QByteArray & cs) {
    Q_D(ContentType);
    d->encCS = cs;
    setParameter(QStringLiteral("name"), s);
}

QByteArray ContentType::id() const {
    return parameter(QStringLiteral("id")).toLatin1();
}

void ContentType::setId(const QByteArray & s) {
    setParameter(QStringLiteral("id"), QString::fromLatin1(s));
}

int ContentType::partialNumber() const {
    QByteArray p = parameter(QStringLiteral("number")).toLatin1();
    if (!p.isEmpty()) {
        return p.toInt();
    } else {
        return -1;
    }
}

int ContentType::partialCount() const {
    QByteArray p = parameter(QStringLiteral("total")).toLatin1();
    if (!p.isEmpty()) {
        return p.toInt();
    } else {
        return -1;
    }
}

contentCategory ContentType::category() const {
    return d_func()->category;
}

void ContentType::setCategory(contentCategory c) {
    Q_D(ContentType);
    d->category = c;
}

void ContentType::setPartialParams(int total, int number) {
    setParameter(QStringLiteral("number"), QString::number(number));
    setParameter(QStringLiteral("total"), QString::number(total));
}

bool ContentType::parse(const char *&scursor, const char *const send,
                        bool isCRLF) {
    Q_D(ContentType);
    // content-type: type "/" subtype *(";" parameter)
    clear();
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return false; // empty header
    }

    // type
    QPair<const char *, int> maybeMimeType;
    if (!parseToken(scursor, send, maybeMimeType, ParseTokenNoFlag)) {
        return false;
    }
    // subtype
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send || *scursor != '/') {
        return false;
    }
    scursor++;
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return false;
    }
    QPair<const char *, int> maybeSubType;
    if (!parseToken(scursor, send, maybeSubType, ParseTokenNoFlag)) {
        return false;
    }

    d->mimeType.reserve(maybeMimeType.second + maybeSubType.second + 1);
    d->mimeType = QByteArray(maybeMimeType.first, maybeMimeType.second).toLower()
                    + '/' + QByteArray(maybeSubType.first, maybeSubType.second).toLower();

    // parameter list
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        goto success; // no parameters
    }
    if (*scursor != ';') {
        return false;
    }
    scursor++;

    if (!Parametrized::parse(scursor, send, isCRLF)) {
        return false;
    }

    // adjust category
success:
    if (isMultipart()) {
        d->category = CCcontainer;
    } else {
        d->category = CCsingle;
    }
    return true;
}

//-----</Content-Type>-------------------------

//-----<ContentID>----------------------

kmime_mk_trivial_ctor_with_name_and_dptr(ContentID, SingleIdent, Content-ID)
kmime_mk_dptr_ctor(ContentID, SingleIdent)

bool ContentID::parse(const char *&scursor, const char *const send, bool isCRLF) {
    Q_D(ContentID);
    // Content-id := "<" contentid ">"
    // contentid := now whitespaces

    const char *origscursor = scursor;
    if (!SingleIdent::parse(scursor, send, isCRLF)) {
        scursor = origscursor;
        d->msgIdList.clear();
        d->cachedIdentifier.clear();

        while (scursor != send) {
            eatCFWS(scursor, send, isCRLF);
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

            eatCFWS(scursor, send, isCRLF);
            if (scursor == send) {
                return false;
            }

            // Save chars until '>''
            QByteArray result;
            if (!parseDotAtom(scursor, send, result, false)) {
                return false;
            }

            eatCFWS(scursor, send, isCRLF);
            if (scursor == send || *scursor != '>') {
                return false;
            }
            scursor++;
            // /Almost parseAngleAddr

            maybeContentId.localPart = QString::fromLatin1(result); // FIXME: just use QByteArray instead of AddrSpec in msgIdList?
            d->msgIdList.append(maybeContentId);

            eatCFWS(scursor, send, isCRLF);
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

typedef struct {
    const char *s;
    int e;
} encTableType;

static const encTableType encTable[] = {
    { "7Bit", CE7Bit },
    { "8Bit", CE8Bit },
    { "quoted-printable", CEquPr },
    { "base64", CEbase64 },
    { "x-uuencode", CEuuenc },
    { "binary", CEbinary },
    { nullptr, 0}
};

void ContentTransferEncoding::clear() {
    Q_D(ContentTransferEncoding);
    d->decoded = true;
    d->cte = CE7Bit;
    Token::clear();
}

contentEncoding ContentTransferEncoding::encoding() const {
    return d_func()->cte;
}

void ContentTransferEncoding::setEncoding(contentEncoding e) {
    Q_D(ContentTransferEncoding);
    d->cte = e;

    for (int i = 0; encTable[i].s != nullptr; ++i) {
        if (d->cte == encTable[i].e) {
            setToken(encTable[i].s);
            break;
        }
    }
}

bool ContentTransferEncoding::isDecoded() const {
    return d_func()->decoded;
}

void ContentTransferEncoding::setDecoded(bool decoded) {
    Q_D(ContentTransferEncoding);
    d->decoded = decoded;
}

bool ContentTransferEncoding::needToEncode() const {
    const Q_D(ContentTransferEncoding);
    return d->decoded && (d->cte == CEquPr || d->cte == CEbase64);
}

bool ContentTransferEncoding::parse(const char  *&scursor,
                                    const char *const send, bool isCRLF) {
    Q_D(ContentTransferEncoding);
    clear();
    if (!Token::parse(scursor, send, isCRLF)) {
        return false;
    }

    // TODO: error handling in case of an unknown encoding?
    for (int i = 0; encTable[i].s != nullptr; ++i) {
        if (qstricmp(token().constData(), encTable[i].s) == 0) {
            d->cte = (contentEncoding)encTable[i].e;
            break;
        }
    }
    d->decoded = (d->cte == CE7Bit || d->cte == CE8Bit);
    return true;
}

//-----</ContentTransferEncoding>---------------------------

//-----<ContentDisposition>--------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name_and_dptr(ContentDisposition,
        Generics::Parametrized, Content-Disposition)
//@endcond

QByteArray ContentDisposition::as7BitString(bool withHeaderType) const {
    if (isEmpty()) {
      return {};
    }

    QByteArray rv;
    if (withHeaderType) {
        rv += typeIntro();
    }

    if (d_func()->disposition == CDattachment) {
        rv += "attachment";
    } else if (d_func()->disposition == CDinline) {
        rv += "inline";
    } else {
      return {};
    }

    if (!Parametrized::isEmpty()) {
        rv += "; " + Parametrized::as7BitString(false);
    }

    return rv;
}

bool ContentDisposition::isEmpty() const {
    return d_func()->disposition == CDInvalid;
}

void ContentDisposition::clear() {
    Q_D(ContentDisposition);
    d->disposition = CDInvalid;
    Parametrized::clear();
}

contentDisposition ContentDisposition::disposition() const {
    return d_func()->disposition;
}

void ContentDisposition::setDisposition(contentDisposition disp) {
    Q_D(ContentDisposition);
    d->disposition = disp;
}

QString KMime::Headers::ContentDisposition::filename() const {
    return parameter(QStringLiteral("filename"));
}

void ContentDisposition::setFilename(const QString & filename) {
    setParameter(QStringLiteral("filename"), filename);
}

bool ContentDisposition::parse(const char  *&scursor, const char *const send,
                                bool isCRLF) {
    Q_D(ContentDisposition);
    clear();

    // token
    QByteArray token;
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return false;
    }

    QPair<const char *, int> maybeToken;
    if (!parseToken(scursor, send, maybeToken, ParseTokenNoFlag)) {
        return false;
    }

    token = QByteArray(maybeToken.first, maybeToken.second).toLower();
    if (token == "inline") {
        d->disposition = CDinline;
    } else if (token == "attachment") {
        d->disposition = CDattachment;
    } else {
        return false;
    }

    // parameter list
    eatCFWS(scursor, send, isCRLF);
    if (scursor == send) {
        return true; // no parameters
    }

    if (*scursor != ';') {
        return false;
    }
    scursor++;

    return Parametrized::parse(scursor, send, isCRLF);
}

//-----</ContentDisposition>-------------------------

//@cond PRIVATE
kmime_mk_trivial_ctor_with_name(Subject, Generics::Unstructured, Subject)
//@endcond

Base *createHeader(const QByteArray & type) {
    return HeaderFactory::createHeader(type);
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
