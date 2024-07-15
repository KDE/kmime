/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "headers.h"
#include "types.h"

#include <QByteArray>
#include <QList>
#include <QString>

#include <map>

//@cond PRIVATE

#define kmime_mk_empty_private( subclass, base ) \
    class subclass##Private : public base##Private {};

namespace KMime
{

namespace Headers
{

// case-insensitive QByteArray comparator supporting heterogenous lookup
// between QByteArray and QByteArrayView
struct CaseInsitiveByteArrayLess {
    using is_transparent = bool;
    template <typename T1, typename T2>
    bool operator()(T1 &&lhs, T2 &&rhs) const
    {
        return qstricmp(lhs.data(), rhs.data()) < 0;
    }
};

using ParameterMap = std::map<QByteArray, QString, CaseInsitiveByteArrayLess>;

// Note that this entire class hierarchy has no virtual dtor, in order to not
// have a second set of vtables, as this is a rather high-volume class.
// This means it's not enough to just delete d_ptr in the base class, but
// we need to delete it with the exact subclass it was created.

class BasePrivate
{
public:
    QByteArray encCS;
};

namespace Generics
{

class UnstructuredPrivate : public BasePrivate
{
public:
    QString decoded;
};

kmime_mk_empty_private(Structured, Base)
kmime_mk_empty_private(Address, Structured)

class MailboxListPrivate : public AddressPrivate
{
public:
  QList<Types::Mailbox> mailboxList;
};

kmime_mk_empty_private(SingleMailbox, MailboxList)

class AddressListPrivate : public AddressPrivate
{
public:
    KMime::Types::AddressList addressList;
};

class IdentPrivate : public AddressPrivate
{
public:
    KMime::Types::AddrSpecList msgIdList;
    mutable QByteArray cachedIdentifier;
};

kmime_mk_empty_private(SingleIdent, Ident)

class TokenPrivate : public StructuredPrivate
{
public:
    QByteArray token;
};

class PhraseListPrivate : public StructuredPrivate
{
public:
    QStringList phraseList;
};

class DotAtomPrivate : public StructuredPrivate
{
public:
    QByteArray dotAtom;
};

class ParametrizedPrivate : public StructuredPrivate
{
public:
    ParameterMap parameterHash;
};

} // namespace Generics

class ReturnPathPrivate : public Generics::AddressPrivate
{
public:
    Types::Mailbox mailbox;
};

class MailCopiesToPrivate : public Generics::AddressListPrivate
{
public:
    MailCopiesToPrivate() :
        Generics::AddressListPrivate(),
        alwaysCopy(false),
        neverCopy(false)
    {}
    bool alwaysCopy;
    bool neverCopy;
};

class ContentTransferEncodingPrivate : public Generics::TokenPrivate
{
public:
    contentEncoding cte = CE7Bit;
};

class ContentTypePrivate : public Generics::ParametrizedPrivate
{
public:
    ContentTypePrivate() :
        Generics::ParametrizedPrivate()
    {}
    QByteArray mimeType;
};

class ContentDispositionPrivate : public Generics::ParametrizedPrivate
{
public:
    ContentDispositionPrivate() :
        Generics::ParametrizedPrivate(),
        disposition(CDInvalid)
    {}
    contentDisposition disposition;
};

class GenericPrivate : public Generics::UnstructuredPrivate
{
public:
    GenericPrivate() :
        Generics::UnstructuredPrivate()
    {}
    ~GenericPrivate()
    {
        delete[] type;
    }

    char *type = nullptr;
};

class ControlPrivate : public Generics::StructuredPrivate
{
public:
    QByteArray name;
    QByteArray parameter;
};

class DatePrivate : public Generics::StructuredPrivate
{
public:
    QDateTime dateTime;
};

class NewsgroupsPrivate : public Generics::StructuredPrivate
{
public:
  QList<QByteArray> groups;
};

class LinesPrivate : public Generics::StructuredPrivate
{
public:
    LinesPrivate() :
        Generics::StructuredPrivate(),
        lines(-1)
    {}
    int lines;
};

kmime_mk_empty_private(ContentID, Generics::SingleIdent)
}

}

#undef kmime_mk_empty_private

//@endcond

