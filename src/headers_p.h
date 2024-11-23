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

class StructuredPrivate : public BasePrivate {};

class MailboxListPrivate : public StructuredPrivate
{
public:
  QList<Types::Mailbox> mailboxList;
};

class SingleMailboxPrivate : public StructuredPrivate {
public:
    Types::Mailbox mailbox;
};

class AddressListPrivate : public StructuredPrivate
{
public:
    KMime::Types::AddressList addressList;
};

class IdentPrivate : public StructuredPrivate
{
public:
    KMime::Types::AddrSpecList msgIdList;
};

class SingleIdentPrivate : public StructuredPrivate
{
public:
    KMime::Types::AddrSpec msgId;
    mutable QByteArray cachedIdentifier;
};

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

class ReturnPathPrivate : public Generics::StructuredPrivate
{
public:
    Types::Mailbox mailbox;
};

class MailCopiesToPrivate : public Generics::AddressListPrivate
{
public:
    bool alwaysCopy = false;
    bool neverCopy = false;
};

class ContentTransferEncodingPrivate : public Generics::TokenPrivate
{
public:
    contentEncoding cte = CE7Bit;
};

class ContentTypePrivate : public Generics::ParametrizedPrivate
{
public:
    QByteArray mimeType;
};

class ContentDispositionPrivate : public Generics::ParametrizedPrivate
{
public:
    contentDisposition disposition = CDInvalid;
};

class GenericPrivate : public Generics::UnstructuredPrivate
{
public:
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
    int lines = -1;
};

class ContentIDPrivate : public Generics::SingleIdentPrivate {};
}

}

#undef kmime_mk_empty_private

//@endcond

