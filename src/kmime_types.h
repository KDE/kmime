/*  -*- c++ -*-
    kmime_header_types.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QString>

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
using AddrSpecList = QList<AddrSpec>;

/**
  Represents an (email address, display name) pair according RFC 2822,
  section 3.4.
*/
class KMIME_EXPORT Mailbox
{
public:
  typedef QList<Mailbox> List;

  /**
    Returns a string representation of the email address, without
    the angle brackets.
  */
  Q_REQUIRED_RESULT QByteArray address() const;

  Q_REQUIRED_RESULT AddrSpec addrSpec() const;

  /**
    Returns the display name.
  */
  Q_REQUIRED_RESULT QString name() const;

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
  Q_REQUIRED_RESULT bool hasAddress() const;

  /**
    Returns true if this mailbox has a display name.
  */
  Q_REQUIRED_RESULT bool hasName() const;

  /**
   * Describes how display names should be quoted
   * @since 4.5
   */
  // AK_REVIEW: remove this enum
  enum Quoting {
    QuoteNever, ///< Don't quote display names at all. Such an unquoted display
                ///< name can not
    ///  be machine-processed anymore in some cases, for example when it
    ///  contains commas, like in "Lastname, Firstname".
    QuoteWhenNecessary, ///< Only quote display names when they contain
                        ///< characters that need to be
    ///  quoted, like commas or quote signs.
    QuoteAlways ///< Always quote the display name
  };

  /**
   * Overloaded method that gives more control over the quoting of the display
   * name
   * @param quoting describes how the display name should be quoted
   * @since 4.5
   */
  Q_REQUIRED_RESULT QString prettyAddress(Quoting quoting = QuoteNever) const;

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
  Q_REQUIRED_RESULT QByteArray as7BitString(const QByteArray &encCharset) const;

  /**
   * Returns a list of mailboxes from an unicode string.
   *
   * @since 5.14
   */
  Q_REQUIRED_RESULT static QList<Mailbox>
  listFromUnicodeString(const QString &s);

  /**
   * Returns a list of mailboxes from an encoded 7bit string.
   *
   * @since 5.14
   */
  Q_REQUIRED_RESULT static QList<Mailbox>
  listFrom7BitString(const QByteArray &s);

  /**
   * Returns a unicode string representing the given list of mailboxes.
   *
   * @since 5.15
   */
  Q_REQUIRED_RESULT static QString
  listToUnicodeString(const QList<Mailbox> &mailboxes);

private:
    QString mDisplayName;
    AddrSpec mAddrSpec;
};

typedef QList<Mailbox> MailboxList;

struct KMIME_EXPORT Address {
    QString displayName;
    MailboxList mailboxList;
};
typedef QList<Address> AddressList;

} // namespace KMime::Types

} // namespace KMime

Q_DECLARE_TYPEINFO(KMime::Types::Mailbox, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::Address, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::AddrSpec, Q_RELOCATABLE_TYPE);


