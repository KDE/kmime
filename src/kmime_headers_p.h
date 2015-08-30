/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef KMIME_HEADERS_P_H
#define KMIME_HEADERS_P_H

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
    QString dotAtom;
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
        Generics::UnstructuredPrivate(),
        type(0)
    {}
    ~GenericPrivate()
    {
        delete[] type;
    }

    char *type;
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

#endif
