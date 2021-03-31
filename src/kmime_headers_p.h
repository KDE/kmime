/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMap>
//@cond PRIVATE

#define kmime_mk_empty_private( subclass, base ) \
    class subclass##Private : public base##Private {};

namespace KMime
{

namespace Headers
{

// Note that this entire class hierarchy has no virtual dtor, in order to not
// have a second set of vtables, as this is a rather high-volume class.
// This means it's not enough to just delete d_ptr in the base class, but
// we need to delete it with the exact sub-class it was created.

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
    QVector<Types::Mailbox> mailboxList;
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
    QMap<QString, QString> parameterHash;
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
    ContentTransferEncodingPrivate() :
        Generics::TokenPrivate(),
        cte(CE7Bit),
        decoded(true)
    {}
    contentEncoding cte;
    bool decoded;
};

class ContentTypePrivate : public Generics::ParametrizedPrivate
{
public:
    ContentTypePrivate() :
        Generics::ParametrizedPrivate(),
        category(CCsingle)
    {}
    QByteArray mimeType;
    contentCategory category;
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
    QVector<QByteArray> groups;
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

