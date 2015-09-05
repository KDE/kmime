/*  -*- c++ -*-
    kmime_header_types.h

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

#ifndef __KMIME_TYPES_H__
#define __KMIME_TYPES_H__

#include <QtCore/QString>
#include <QtCore/QVector>

#include "kmime_export.h"

namespace KMime
{

namespace Types
{

struct KMIME_EXPORT AddrSpec {
    QString asString() const;
    /*! This is the same as asString(), except it decodes IDNs for display */
    QString asPrettyString() const;
    bool isEmpty() const;
    QString localPart;
    QString domain;
};
typedef QVector<AddrSpec> AddrSpecList;

/**
  Represents an (email address, display name) pair according RFC 2822,
  section 3.4.
*/
class KMIME_EXPORT Mailbox
{
public:
    typedef QVector<Mailbox> List;

    /**
      Returns a string representation of the email address, without
      the angle brackets.
    */
    QByteArray address() const;

    AddrSpec addrSpec() const;

    /**
      Returns the display name.
    */
    QString name() const;

    /**
      Sets the email address.
    */
    void setAddress(const AddrSpec &addr);

    /**
      Sets the email address.
    */
    void setAddress(const QByteArray &addr);

    /**
      Sets the name.
    */
    void setName(const QString &name);

    /**
      Sets the name based on a 7bit encoded string.
    */
    void setNameFrom7Bit(const QByteArray &name,
                         const QByteArray &defaultCharset = QByteArray());

    /**
      Returns true if this mailbox has an address.
    */
    bool hasAddress() const;

    /**
      Returns true if this mailbox has a display name.
    */
    bool hasName() const;

    /**
      Returns a assembled display name / address string of the following form:
      "Display Name &lt;address&gt;". These are unicode strings without any
      transport encoding, ie. they are only suitable for displaying.
    */
    QString prettyAddress() const;

    /**
     * Describes how display names should be quoted
     * @since 4.5
     */
    //AK_REVIEW: remove this enum
    enum Quoting {
        QuoteNever,         ///< Don't quote display names at all. Such an unquoted display name can not
        ///  be machine-processed anymore in some cases, for example when it contains
        ///  commas, like in "Lastname, Firstname".
        QuoteWhenNecessary, ///< Only quote display names when they contain characters that need to be
        ///  quoted, like commas or quote signs.
        QuoteAlways         ///< Always quote the display name
    };

    /**
     * Overloaded method that gives more control over the quoting of the display name
     * @param quoting describes how the display name should be quoted
     * @since 4.5
     */
    // TODO: KDE5: BIC: remove other prettyAddress() overload, and make it None the default
    //                  parameter here
    //AK_REVIEW: replace by 'QString quotedAddress() const'
    QString prettyAddress(Quoting quoting) const;

    /**
      Parses the given unicode string.
    */
    void fromUnicodeString(const QString &s);

    /**
      Parses the given 7bit encoded string.
    */
    void from7BitString(const QByteArray &s);

    /**
      Returns a 7bit transport encoded representation of this mailbox.

      @param encCharset The charset used for encoding.
    */
    QByteArray as7BitString(const QByteArray &encCharset) const;

private:
    QString mDisplayName;
    AddrSpec mAddrSpec;
};

typedef QVector<Mailbox> MailboxList;

struct KMIME_EXPORT Address {
    QString displayName;
    MailboxList mailboxList;
};
typedef QVector<Address> AddressList;

} // namespace KMime::Types

} // namespace KMime

Q_DECLARE_TYPEINFO(KMime::Types::Mailbox, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::Address, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::AddrSpec, Q_MOVABLE_TYPE);

#endif // __KMIME_HEADER_PARSING_H__

