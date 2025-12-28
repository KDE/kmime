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

/*!
 * \namespace KMime::Types
 * \inmodule KMime
 * \inheaderfile KMime/Types
 *
 * \brief Basic data types defined in RFC 2822.
 */
namespace Types
{

/*!
 * \class KMime::Types::AddrSpec
 * \inmodule KMime
 * \inheaderfile KMime/Types
 *
 * \brief Represents an addr-spec according to RFC 2822 §3.4.1.
 *
 * That's what you might commonly call an email address (but addressing
 * emails is slightly more complex than that).
 */
struct KMIME_EXPORT AddrSpec {
    /*!
     *
     */
    [[nodiscard]] QString asString() const;

    /*!
     * This is the same as asString(), except it decodes IDNs for display
     */
    [[nodiscard]] QString asPrettyString() const;

    /*!
     *
     */
    [[nodiscard]] bool isEmpty() const;

    /*!
     *
     */
    QString localPart;

    /*!
     *
     */
    QString domain;
};

/*!
  \class KMime::Types::Mailbox
  \inmodule KMime
  \inheaderfile KMime/Types

  \brief Represents an (email address, display name) pair according RFC 2822,
  section 3.4.
*/
class KMIME_EXPORT Mailbox
{
public:
  /*!
    Returns a string representation of the email address, without
    the angle brackets.
  */
  [[nodiscard]] QByteArray address() const;

  /*!
   */
  [[nodiscard]] AddrSpec addrSpec() const;

  /*!
    Returns the display name.
  */
  [[nodiscard]] QString name() const;

  /*!
    Sets the email address.
  */
  void setAddress(const AddrSpec &addr);

  /*!
    Sets the email address.
  */
  void setAddress(const QByteArray &addr);

  /*!
    Sets the name.
  */
  void setName(const QString &name);

  /*!
    Sets the name based on a 7bit encoded string.
  */
  void setNameFrom7Bit(const QByteArray &name,
                       const QByteArray &defaultCharset = QByteArray());

  /*!
    Returns true if this mailbox has an address.
  */
  [[nodiscard]] bool hasAddress() const;

  /*!
    Returns true if this mailbox has a display name.
  */
  [[nodiscard]] bool hasName() const;

  /*!
   * Describes how display names should be quoted
   *
   * \value QuoteNever Don't quote display names at all. Such an unquoted display name can not be machine-processed anymore in some cases, for example when it contains commas, like in "Lastname, Firstname"
   * \value QuoteWhenNecessary Only quote display names when they contain characters that need to be quoted, like commas or quote signs
   * \value QuoteAlways Always quote the display name
   *
   * \since 4.5
   */
  enum Quoting {
    QuoteNever,
    QuoteWhenNecessary,
    QuoteAlways
  };

  /*!
   * Overloaded method that gives more control over the quoting of the display
   * name
   *
   * \a quoting describes how the display name should be quoted
   * \since 4.5
   */
  [[nodiscard]] QString prettyAddress(Quoting quoting = QuoteNever) const;

  /*!
    Parses the given unicode string.
  */
  void fromUnicodeString(QStringView s);

  /*!
    Parses the given 7bit encoded string.

    \a s The given 7bit encoded string
  */
  void from7BitString(QByteArrayView s);

  /*!
    Returns a 7bit transport encoded representation of this mailbox.

    \a encCharset The charset used for encoding.
  */
  [[nodiscard]] QByteArray as7BitString(const QByteArray &encCharset) const;

  /*!
   * Returns a list of mailboxes from an unicode string.
   *
   * \since 5.14
   */
  [[nodiscard]] static QList<Mailbox> listFromUnicodeString(QStringView s);

  /*!
   * Returns a list of mailboxes from an encoded 7bit string.
   *
   * \since 5.14
   */
  [[nodiscard]] static QList<Mailbox> listFrom7BitString(QByteArrayView s);

  /*!
   * Returns a unicode string representing the given list of mailboxes.
   *
   * \since 5.15
   */
  [[nodiscard]] static QString
  listToUnicodeString(const QList<Mailbox> &mailboxes);

private:
    QString mDisplayName;
    AddrSpec mAddrSpec;
};

/*!
  \class KMime::Types::Address
  \inmodule KMime
  \inheaderfile KMime/Types

  \brief Represents an address as defined in RFC 2822 §3.4.

  That is, a mailbox or a named group, ie. a named list
  of mailboxes.
*/
class KMIME_EXPORT Address {
public:
    /*!
     * The display name, in case this is a named group.
     * \since 25.12 (previously a public member with the same name)
     */
    [[nodiscard]] QString displayName() const;
    /*!
     * Set the group name.
     *
     * This strips bidi control characters.
     * \since 25.12
     */
    void setDisplayName(const QString &displayName);

    /*! Either a single mailbox or the mailboxes in the named group. */
    QList<Mailbox> mailboxList;
private:
    QString m_displayName;
};

} // namespace KMime::Types

} // namespace KMime

Q_DECLARE_TYPEINFO(KMime::Types::Mailbox, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::Address, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(KMime::Types::AddrSpec, Q_RELOCATABLE_TYPE);


