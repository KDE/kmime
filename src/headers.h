/*  -*- c++ -*-
    kmime_headers.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 the KMime authors.
    See file AUTHORS for details
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/*!
  @file
  This file is part of the API for handling \ MIME data and
  defines the various header classes:
   - header's base class defining the common interface
   - generic base classes for different types of fields
   - incompatible, Structured-based field classes
   - compatible, Unstructured-based field classes

  \brief
  Defines the various headers classes.

  @authors the KMime authors (see AUTHORS file),
  Volker Krause \<vkrause@kde.org\>
*/

#pragma once

#include "kmime_export.h"
#include "headerparsing.h"

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace KMime
{

class Content;

namespace Headers
{

class BasePrivate;

/*!
  Various possible values for the "Content-Transfer-Encoding" header.
*/
enum contentEncoding {
    CE7Bit,              ///< 7bit
    CE8Bit,              ///< 8bit
    CEquPr,              ///< quoted-printable
    CEbase64,            ///< base64
    CEuuenc,             ///< uuencode
    CEbinary             ///< binary
};

/*!
  Various possible values for the "Content-Disposition" header.
*/
enum contentDisposition {
    CDInvalid,           ///< Default, invalid value
    CDinline,            ///< inline
    CDattachment,        ///< attachment
    CDparallel           ///< parallel (invalid, do not use)
};


// internal macro to generate default constructors
#define kmime_mk_trivial_ctor( subclass )                               \
    public:                                                               \
    subclass();                           \
    ~subclass() override; \
    using KMime::Headers::Base::as7BitString; // TODO drop this when the deprecated as7BitString overload is removed

#define kmime_mk_dptr_ctor( subclass ) \
    protected: \
    explicit subclass( subclass##Private *d );

#define kmime_mk_trivial_ctor_with_name( subclass )     \
    kmime_mk_trivial_ctor( subclass )                     \
    [[nodiscard]] const char *type() const override;                           \
    [[nodiscard]] static const char *staticType();
//@endcond

//
//
// HEADER'S BASE CLASS. DEFINES THE COMMON INTERFACE
//
//

/*! Baseclass of all header-classes. It represents a
    header-field as described in RFC-822.  */
class KMIME_EXPORT Base
{
public:
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    /*!
      A vector of headers.
    */
  KMIME_DEPRECATED typedef QList<KMime::Headers::Base *> List;
#endif
  /*!
    Creates an empty header.
  */
  Base();

  /*!
    Destructor.
  */
  virtual ~Base();

  /*!
    Parses the given string. Take care of RFC2047-encoded strings.
    \a s The encoded header data.
  */
  virtual void from7BitString(QByteArrayView s) = 0;

  /*!
    Returns the encoded header content.
    This does not include the header type or separating colon.
    \since 26.04 (previously with an additional bool argument to include the header type).
  */
  [[nodiscard]] virtual QByteArray as7BitString() const = 0;
  /*!
    Returns the encoded header.
    \a withHeaderType Specifies whether the header-type should be included.
  */
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("use as7BitString() instead")]]
  [[nodiscard]] QByteArray as7BitString(bool) const { return as7BitString(); }
#endif

  /*!
    Returns the charset that is used for RFC2047-encoding.
  */
  [[nodiscard]] QByteArray rfc2047Charset() const;

  /*!
    Sets the charset for RFC2047-encoding.
    \a cs The new charset used for RFC2047 encoding.
  */
  void setRFC2047Charset(const QByteArray &cs);

  /*!
    Parses the given Unicode representation of the header content.
    \a s The header data as Unicode string.
  */
  virtual void fromUnicodeString(const QString &s) = 0;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
  [[deprecated("call setRFC2047Charset for the second argument if different from UTF-8, otherwise remove second argument")]]
  inline void fromUnicodeString(const QString &s, const QByteArray &b)
  {
    setRFC2047Charset(b);
    fromUnicodeString(s);
  }
#endif

  /*!
    Returns the decoded content of the header without the header-type.

    Note The return value of this method should only be used when showing an
    address to the user. It is not guaranteed that fromUnicodeString(
    asUnicodeString(), ... ) will return the original string.
  */
  [[nodiscard]] virtual QString asUnicodeString() const = 0;

  /*!
    Checks if this header contains any data.
  */
  [[nodiscard]] virtual bool isEmpty() const = 0;

  /*!
    Returns the type of this header (e.g. "From").
  */
  [[nodiscard]] virtual const char *type() const;

  /*!
    Checks if this header is of type \a t.
  */
  [[nodiscard]] bool is(QByteArrayView t) const;

protected:

    BasePrivate *d_ptr;
    kmime_mk_dptr_ctor(Base)
    //@endcond

private:
    Q_DECLARE_PRIVATE(Base)
    Q_DISABLE_COPY(Base)
};

//
//
// GENERIC BASE CLASSES FOR DIFFERENT TYPES OF FIELDS
//
//

namespace Generics
{

class UnstructuredPrivate;

/*!
  Abstract base class for unstructured header fields
  (e.g. "Subject", "Comment", "Content-description").

  Features: Decodes the header according to RFC2047, incl. RFC2231
  extensions to encoded-words.

  Subclasses need only re-implement \a const \a char* \a type().
*/

// known issues:
// - uses old decodeRFC2047String function, instead of our own...

class KMIME_EXPORT Unstructured : public Base
{

    kmime_mk_dptr_ctor(Unstructured)
    //@endcond
public:
    Unstructured();
    ~Unstructured() override;

    void from7BitString(QByteArrayView s) override;
    [[nodiscard]] QByteArray as7BitString() const override;

    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;
    [[nodiscard]] QString asUnicodeString() const override;

    [[nodiscard]] bool isEmpty() const override;

private:
    Q_DECLARE_PRIVATE(Unstructured)
};

class StructuredPrivate;

/*!
  \brief
  Base class for structured header fields.

  This is the base class for all structured header fields.
  It contains parsing methods for all basic token types found in rfc2822.

  @section Parsing

  At the basic level, there are tokens & tspecials (rfc2045),
  atoms & specials, quoted-strings, domain-literals (all rfc822) and
  encoded-words (rfc2047).

  As a special token, we have the comment. It is one of the basic
  tokens defined in rfc822, but it's parsing relies in part on the
  basic token parsers (e.g. comments may contain encoded-words).
  Also, most upper-level parsers (notably those for phrase and
  dot-atom) choose to ignore any comment when parsing.

  Then there are the real composite tokens, which are made up of one
  or more of the basic tokens (and semantically invisible comments):
  phrases (rfc822 with rfc2047) and dot-atoms (rfc2822).

  This finishes the list of supported token types. Subclasses will
  provide support for more higher-level tokens, where necessary,
  using these parsers.

  @author Marc Mutz <mutz@kde.org>
*/

class KMIME_EXPORT Structured : public Base
{
public:
    Structured();
    ~Structured() override;

    void from7BitString(QByteArrayView s) override;
    [[nodiscard]] QString asUnicodeString() const override;
    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;

protected:
    /*!
      This method parses the raw header and needs to be implemented in
      every sub-class.

      \a scursor Pointer to the start of the data still to parse.
      \a send Pointer to the end of the data.
      \a newline whether the input string is terminated with CRLF or LF.
    */
    virtual bool parse(const char *&scursor, const char *const send,
                       NewlineType newline = NewlineType::LF) = 0;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the NewlineType overload instead")]]
    inline bool parse(const char *&scursor, const char *const send, bool isCRLF) {
        return parse(scursor, send, isCRLF ? NewlineType::CRLF : NewlineType::LF);
    }
#endif


    kmime_mk_dptr_ctor(Structured)
    //@endcond

private:
    Q_DECLARE_PRIVATE(Structured)
};

class MailboxListPrivate;

/*!
  Base class for headers that deal with (possibly multiple)
  addresses, but don't allow groups.

  \sa RFC 2822, section 3.4
*/
class KMIME_EXPORT MailboxList : public Structured
{

    kmime_mk_trivial_ctor(MailboxList)
    kmime_mk_dptr_ctor(MailboxList)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;
    [[nodiscard]] QString asUnicodeString() const override;

    [[nodiscard]] bool isEmpty() const override;

    /*!
      Adds an address to this header.

      \a mbox A Mailbox object specifying the address.
    */
    void addAddress(const Types::Mailbox &mbox);

    /*!
      Adds an address to this header.
      \a address The actual email address, with or without angle brackets.
      \a displayName An optional name associated with the address.
    */
    void addAddress(const QByteArray &address,
                    const QString &displayName = QString());

    /*!
      Returns a list of all addresses in this header, regardless of groups.
    */
    [[nodiscard]] QList<QByteArray> addresses() const;

    /*!
      Returns a list of all display names associated with the addresses in
      this header. The address is added for addresses that do not have
      a display name.
    */
    [[nodiscard]] QStringList displayNames() const;

    /*!
      Returns a single string for user-facing display of this mailbox list.
      This is equivalent to displayNames().join(", ").
      \since 5.14
    */
    [[nodiscard]] QString displayString() const;

    /*!
      Returns a list of mailboxes listed in this header.
    */
    [[nodiscard]] QList<Types::Mailbox> mailboxes() const;

    /*!
      Sets the mailboxes listed in this header, replacing the current content.
      \since 24.12
    */
    void setMailboxes(const QList<Types::Mailbox> &mailboxes);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(MailboxList)
};

class SingleMailboxPrivate;

/*!
   Base class for headers that deal with exactly one mailbox
   (e.g. Sender).
*/
class KMIME_EXPORT SingleMailbox : public Structured
{

    kmime_mk_trivial_ctor(SingleMailbox)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;
    [[nodiscard]] QString asUnicodeString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the mailbox in this header.
      \since 25.04
    */
    [[nodiscard]] Types::Mailbox mailbox() const;

    /*!
      Sets the mailboxes in this header, replacing the current content.
      \since 25.04
    */
    void setMailbox(const Types::Mailbox &mailbox);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(SingleMailbox)
};

class AddressListPrivate;

/*!
  Base class for headers that deal with (possibly multiple)
  addresses, allowing groups.

  Note Groups are parsed but not represented in the API yet. All addresses in
  groups are listed as if they would not be part of a group.

  @todo Add API for groups?

  \sa RFC 2822, section 3.4
*/
class KMIME_EXPORT AddressList : public Structured
{

    kmime_mk_trivial_ctor(AddressList)
    kmime_mk_dptr_ctor(AddressList)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;
    [[nodiscard]] QString asUnicodeString() const override;

    [[nodiscard]] bool isEmpty() const override;

    /*!
      Adds an address to this header.

      \a mbox A Mailbox object specifying the address.
    */
    void addAddress(const Types::Mailbox &mbox);

    /*!
      Adds an address to this header.
      \a address The actual email address, with or without angle brackets.
      \a displayName An optional name associated with the address.
    */
    void addAddress(const QByteArray &address, const QString &displayName = QString());

    /*!
      Returns a list of all addresses in this header, regardless of groups.
    */
    [[nodiscard]] QList<QByteArray> addresses() const;

    /*!
      Returns a list of all display names associated with the addresses in this header.
      The address is added for addresses that don't have a display name.
    */
    [[nodiscard]] QStringList displayNames() const;

    /*!
      Returns a single string for user-facing display of this address list.
      This is equivalent to displayNames().join(", ").
      \since 5.14
    */
    [[nodiscard]] QString displayString() const;

    /*!
      Returns a list of mailboxes listed in this header.
    */
    [[nodiscard]] QList<Types::Mailbox> mailboxes() const;

    /*!
      Sets the list of addresses listed in this header, replacing the existing content.
      \since 24.12
    */
    void setAddressList(const QList<Types::Address> &addresses);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(AddressList)
};

class IdentPrivate;

/*!
  Base class for headers which deal with a list of msg-id's.

  \sa RFC 2822, section 3.6.4
*/
class KMIME_EXPORT Ident : public Structured
{

    kmime_mk_trivial_ctor(Ident)
    kmime_mk_dptr_ctor(Ident)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Initialize this identifier by copying the data from \a ident.
     */
    void fromIdent(const Ident &ident);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("Use fromIdent(const Ident&) instead")]]
    void fromIdent(const Ident *ident) {
        fromIdent(*ident);
    }
#endif

    /*!
      Returns the list of identifiers contained in this header.
      Note
      - Identifiers are not enclosed in angle-brackets.
      - Identifiers are listed in the same order as in the header.
    */
    [[nodiscard]] QList<QByteArray> identifiers() const;

    /*!
      Appends a new identifier to this header.
      \a id The identifier to append, with or without angle-brackets.
    */
    void appendIdentifier(const QByteArray &id);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Ident)
};

class SingleIdentPrivate;

/*!
  Base class for headers which deal with a single msg-id.

  \sa RFC 2822, section 3.6.4
*/
class KMIME_EXPORT SingleIdent : public Structured
{

    kmime_mk_trivial_ctor(SingleIdent)
    kmime_mk_dptr_ctor(SingleIdent)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the identifier contained in this header.
      Note The identifiers is not enclosed in angle-brackets.
    */
    [[nodiscard]] QByteArray identifier() const;

    /*!
      Sets the identifier.
      \a id The new identifier with or without angle-brackets.
    */
    void setIdentifier(const QByteArray &id);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(SingleIdent)
};

class TokenPrivate;

/*!
  Base class for headers which deal with a single atom.
*/
class KMIME_EXPORT Token : public Structured
{

    kmime_mk_trivial_ctor(Token)
    kmime_mk_dptr_ctor(Token)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the token.
    */
    [[nodiscard]] QByteArray token() const;

    /*!
      Sets the token to \a t,
    */
    void setToken(const QByteArray &t);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Token)
};

class PhraseListPrivate;

/*!
  Base class for headers containing a list of phrases.
*/
class KMIME_EXPORT PhraseList : public Structured
{

    kmime_mk_trivial_ctor(PhraseList)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] QString asUnicodeString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the list of phrases contained in this header.
    */
    [[nodiscard]] QStringList phrases() const;

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(PhraseList)
};

class DotAtomPrivate;

/*!
  Base class for headers containing a dot atom.
*/
class KMIME_EXPORT DotAtom : public Structured
{

    kmime_mk_trivial_ctor(DotAtom)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] QString asUnicodeString() const override;
    [[nodiscard]] bool isEmpty() const override;

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(DotAtom)
};

class ParametrizedPrivate;

/*!
  Base class for headers containing a parameter list such as "Content-Type".
*/
class KMIME_EXPORT Parametrized : public Structured
{

    kmime_mk_trivial_ctor(Parametrized)
    kmime_mk_dptr_ctor(Parametrized)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;

    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the value of the specified parameter.
      \a key The parameter name.
    */
    [[nodiscard]] QString parameter(QByteArrayView key) const;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use QByteArrayView as argument")]] [[nodiscard]] inline QString parameter(const QString &key) const
    {
        return parameter(QByteArrayView(key.toUtf8()));
    }
    [[deprecated("use QByteArrayView as argument")]] [[nodiscard]] inline QString parameter(QLatin1StringView key) const
    {
        return parameter(QByteArrayView(key));
    }
#endif
    // overload resolution helper, remove once the above deprecated overloads are removed
    template <std::size_t N>
    [[nodiscard]] inline QString parameter(const char (&key)[N]) const
    {
        return parameter(QByteArrayView(key, N));
    }

    /*!
      \a key the key of the parameter to check for
      Returns true if a parameter with the given \a key exists.
      \since 4.5
    */
    [[nodiscard]] bool hasParameter(QByteArrayView key) const;
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use QByteArrayView as argument")]] [[nodiscard]] inline bool hasParameter(const QString &key) const
    {
        return hasParameter(QByteArrayView(key.toUtf8()));
    }
    [[deprecated("use QByteArrayView as argument")]] [[nodiscard]] inline bool hasParameter(QLatin1StringView key) const
    {
        return hasParameter(QByteArrayView(key));
    }
#endif
    // overload resolution helper, remove once the above deprecated overloads are removed
    template <std::size_t N>
    [[nodiscard]] inline bool hasParameter(const char (&key)[N]) const {
        return hasParameter(QByteArrayView(key, N));
    }

    /*!
      Sets the parameter \a key to \a value.
      \a key The parameter name.
      \a value The new value for \a key.
    */
    void setParameter(const QByteArray &key, const QString &value);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use a QByteArray[Literal] key")]] inline void setParameter(const QString &key, const QString &value)
    {
        return setParameter(key.toUtf8(), value);
    }
#endif

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Parametrized)
};

} // namespace Generics

//
//
// INCOMPATIBLE, GSTRUCTURED-BASED FIELDS:
//
//

class ReturnPathPrivate;

/*!
  Represents the Return-Path header field.

  \sa RFC 2822, section 3.6.7
*/
class KMIME_EXPORT ReturnPath : public Generics::Structured
{

    kmime_mk_trivial_ctor_with_name(ReturnPath)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(ReturnPath)
};

// Address et al.:

// rfc(2)822 headers:
/*!
   Represent a "From" header.

   \sa RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT From : public Generics::MailboxList
{
    kmime_mk_trivial_ctor_with_name(From)
};

/*!
  Represents a "Sender" header.

  \sa RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT Sender : public Generics::SingleMailbox
{
    kmime_mk_trivial_ctor_with_name(Sender)
};

/*!
  Represents a "To" header.

  \sa RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT To : public Generics::AddressList
{
    kmime_mk_trivial_ctor_with_name(To)
};

/*!
  Represents a "Cc" header.

  \sa RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT Cc : public Generics::AddressList
{
    kmime_mk_trivial_ctor_with_name(Cc)
};

/*!
  Represents a "Bcc" header.

  \sa RFC 2822, section 3.6.3.
*/
class KMIME_EXPORT Bcc : public Generics::AddressList
{
    kmime_mk_trivial_ctor_with_name(Bcc)
};

/*!
  Represents a "ReplyTo" header.

  \sa RFC 2822, section 3.6.2.
*/
class KMIME_EXPORT ReplyTo : public Generics::AddressList
{
    kmime_mk_trivial_ctor_with_name(ReplyTo)
};

class MailCopiesToPrivate;

/*!
  Represents a "Mail-Copies-To" header.

  \sa http://www.newsreaders.com/misc/mail-copies-to.html
*/
class KMIME_EXPORT MailCopiesTo : public Generics::AddressList
{

    kmime_mk_trivial_ctor_with_name(MailCopiesTo)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] QString asUnicodeString() const override;

    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns true if a mail copy was explicitly requested.
    */
    [[nodiscard]] bool alwaysCopy() const;

    /*!
      Sets the header to "poster".
    */
    void setAlwaysCopy();

    /*!
      Returns true if a mail copy was explicitly denied.
    */
    [[nodiscard]] bool neverCopy() const;

    /*!
      Sets the header to "never".
    */
    void setNeverCopy();

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(MailCopiesTo)
};

class ContentTransferEncodingPrivate;

/*!
  Represents a "Content-Transfer-Encoding" header.

  \sa RFC 2045, section 6.
*/
class KMIME_EXPORT ContentTransferEncoding : public Generics::Token
{

    kmime_mk_trivial_ctor_with_name(ContentTransferEncoding)
    //@endcond
public:
    [[nodiscard]] bool isEmpty() const override;
    [[nodiscard]] QByteArray as7BitString() const override;

    /*!
      Returns the encoding specified in this header.
    */
    [[nodiscard]] contentEncoding encoding() const;

    /*!
      Sets the encoding to \a e.
    */
    void setEncoding(contentEncoding e);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(ContentTransferEncoding)
};

/*!
  Represents a "Keywords" header.

  \sa RFC 2822, section 3.6.5.
*/
class KMIME_EXPORT Keywords : public Generics::PhraseList
{
    kmime_mk_trivial_ctor_with_name(Keywords)
};

// DotAtom:

/*!
  Represents a "MIME-Version" header.

  \sa RFC 2045, section 4.
*/
class KMIME_EXPORT MIMEVersion : public Generics::DotAtom
{
    kmime_mk_trivial_ctor_with_name(MIMEVersion)
};

// Ident:

/*!
  Represents a "Message-ID" header.

  \sa RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT MessageID : public Generics::SingleIdent
{

    kmime_mk_trivial_ctor_with_name(MessageID)
    //@endcond
public:
    /*!
      Generate a message identifier.
      \a fqdn A fully qualified domain name.
    */
    void generate(const QByteArray &fqdn);
};

class ContentIDPrivate;

/*!
  Represents a "Content-ID" header.
*/
class KMIME_EXPORT ContentID : public Generics::SingleIdent
{

    kmime_mk_trivial_ctor_with_name(ContentID)
    kmime_mk_dptr_ctor(ContentID)
    //@endcond

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;
private:
    Q_DECLARE_PRIVATE(ContentID)
};

/*!
  Represents a "Supersedes" header.
*/
class KMIME_EXPORT Supersedes : public Generics::SingleIdent
{
    kmime_mk_trivial_ctor_with_name(Supersedes)
};

/*!
  Represents a "In-Reply-To" header.

  \sa RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT InReplyTo : public Generics::Ident
{
    kmime_mk_trivial_ctor_with_name(InReplyTo)
};

/*!
  Represents a "References" header.

  \sa RFC 2822, section 3.6.4.
*/
class KMIME_EXPORT References : public Generics::Ident
{
    kmime_mk_trivial_ctor_with_name(References)
};

class ContentTypePrivate;

/*!
  Represents a "Content-Type" header.

  \sa RFC 2045, section 5.
*/
class KMIME_EXPORT ContentType : public Generics::Parametrized
{

    kmime_mk_trivial_ctor_with_name(ContentType)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the mimetype.
    */
    [[nodiscard]] QByteArray mimeType() const;

    /*!
      Returns the media type (first part of the mimetype).
    */

    [[nodiscard]] QByteArray mediaType() const;

    /*!
      Returns the mime sub-type (second part of the mimetype).
    */
    [[nodiscard]] QByteArray subType() const;

    /*!
      Sets the mimetype.
      \a mimeType The new mimetype.
    */
    void setMimeType(const QByteArray &mimeType);

    /*!
      Tests if the media type equals \a mediatype.
    */
    [[nodiscard]] bool isMediatype(const char *mediatype) const;

    /*!
      Tests if the mime sub-type equals \a subtype.
    */
    [[nodiscard]] bool isSubtype(const char *subtype) const;

    /*!
      Tests if the mime type is \a mimeType.
    */
    [[nodiscard]] bool isMimeType(const char *mimeType) const;

    /*!
      Returns true if the associated MIME entity is a text.
    */
    [[nodiscard]] bool isText() const;

    /*!
      Returns true if the associated MIME entity is a plain text.
    */
    [[nodiscard]] bool isPlainText() const;

    /*!
      Returns true if the associated MIME entity is a HTML file.
    */
    [[nodiscard]] bool isHTMLText() const;

    /*!
      Returns true if the associated MIME entity is an image.
    */
    [[nodiscard]] bool isImage() const;

    /*!
      Returns true if the associated MIME entity is a multipart container.
    */
    [[nodiscard]] bool isMultipart() const;

    /*!
      Returns true if the associated MIME entity contains partial data.
      \sa partialNumber(), partialCount()
    */
    [[nodiscard]] bool isPartial() const;

    /*!
      Returns the charset for the associated MIME entity.
    */
    [[nodiscard]] QByteArray charset() const;

    /*!
      Sets the charset.
    */
    void setCharset(const QByteArray &s);

    /*!
      Returns the boundary (for multipart containers).
    */
    [[nodiscard]] QByteArray boundary() const;

    /*!
      Sets the multipart container boundary.
    */
    void setBoundary(const QByteArray &s);

    /*!
      Returns the name of the associated MIME entity.
    */
    [[nodiscard]] QString name() const;

    /*!
      Sets the name to \a s.
    */
    void setName(const QString &s);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("call setRFC2047Charset for the second argument if different from UTF-8, otherwise remove second argument")]]
    inline void setName(const QString &s, const QByteArray &cs)
    {
        setRFC2047Charset(cs);
        setName(s);
    }
#endif

    /*!
      Returns the identifier of the associated MIME entity.
    */
    [[nodiscard]] QByteArray id() const;

    /*!
      Sets the identifier.
    */
    void setId(const QByteArray &s);

    /*!
      Returns the position of this part in a multi-part set.
      \sa isPartial(), partialCount()
    */
    [[nodiscard]] int partialNumber() const;
    /*!
      Sets the position of this part in a multi-part set.
      \sa partialCount()
      \since 25.12
    */
    void setPartialNumber(int number);

    /*!
      Returns the total number of parts in a multi-part set.
      \sa isPartial(), partialNumber()
    */
    [[nodiscard]] int partialCount() const;
    /*!
      Sets the total number of parts in a multi-part set.
      \sa partialCount()
      \since 25.12
    */
    void setPartialCount(int total);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    /*!
      Sets parameters of a partial MIME entity.
      \a total The total number of entities in the multi-part set.
      \a number The number of this entity in a multi-part set.
      \deprecated since 25.12, use setPartialCount/setPartialNumber instead.
    */
    [[deprecated("Use setPartialCount/setPartialNumber instead")]]
    void setPartialParams(int total, int number) {
        setPartialNumber(number);
        setPartialCount(total);
    }
#endif

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(ContentType)
};

class ContentDispositionPrivate;

/*!
  Represents a "Content-Disposition" header.

  \sa RFC 2183
*/
class KMIME_EXPORT ContentDisposition : public Generics::Parametrized
{

    kmime_mk_trivial_ctor_with_name(ContentDisposition)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the content disposition.
    */
    [[nodiscard]] contentDisposition disposition() const;

    /*!
      Sets the content disposition.
      \a disp The new content disposition.
    */
    void setDisposition(contentDisposition disp);

    /*!
      Returns the suggested filename for the associated MIME part.
      This is just a convenience function, it is equivalent to calling
      parameter( "filename" );
    */
    [[nodiscard]] QString filename() const;

    /*!
      Sets the suggested filename for the associated MIME part.
      This is just a convenience function, it is equivalent to calling
      setParameter( "filename", filename );
      \a filename The filename.
    */
    void setFilename(const QString &filename);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(ContentDisposition)
};

//
//
// COMPATIBLE GUNSTRUCTURED-BASED FIELDS:
//
//

class GenericPrivate;

/*!
  Represents an arbitrary header, that can contain any header-field.
  Adds a type over Unstructured.
  \sa Unstructured
*/
class KMIME_EXPORT Generic : public Generics::Unstructured
{
public:
    Generic();
    Generic(const char *t, qsizetype len = -1);
    ~Generic() override;

    [[nodiscard]] bool isEmpty() const override;

    [[nodiscard]] const char *type() const override;

    void setType(const char *type, qsizetype len = -1);

private:
    Q_DECLARE_PRIVATE(Generic)
};

/*!
  Represents a "Subject" header.

  \sa RFC 2822, section 3.6.5.
*/
class KMIME_EXPORT Subject : public Generics::Unstructured
{

    kmime_mk_trivial_ctor_with_name(Subject)
    //@endcond
};

/*!
  Represents a "Organization" header.
*/
class KMIME_EXPORT Organization : public Generics::Unstructured
{
    kmime_mk_trivial_ctor_with_name(Organization)
};

/*!
  Represents a "Content-Description" header.
*/
class KMIME_EXPORT ContentDescription : public Generics::Unstructured
{
    kmime_mk_trivial_ctor_with_name(ContentDescription)
};

/*!
  Represents a "Content-Location" header.
  \since 4.2
*/
class KMIME_EXPORT ContentLocation : public Generics::Unstructured
{
    kmime_mk_trivial_ctor_with_name(ContentLocation)
};

class ControlPrivate;

/*!
  Represents a "Control" header.

  \sa RFC 1036, section 3.
*/
class KMIME_EXPORT Control : public Generics::Structured
{

    kmime_mk_trivial_ctor_with_name(Control)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the control message type.
    */
    [[nodiscard]] QByteArray controlType() const;

    /*!
      Returns the control message parameter.
    */
    [[nodiscard]] QByteArray parameter() const;

    /*!
      Returns true if this is a cancel control message.
      \sa RFC 1036, section 3.1.
    */
    [[nodiscard]] bool isCancel() const;

    /*!
      Changes this header into a cancel control message for the given message-id.
      \a msgid The message-id of the article that should be canceled.
    */
    void setCancel(const QByteArray &msgid);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Control)
};

class DatePrivate;

/*!
  Represents a "Date" header.

  \sa RFC 2822, section 3.3.
*/
class KMIME_EXPORT Date : public Generics::Structured
{

    kmime_mk_trivial_ctor_with_name(Date)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the date contained in this header.
    */
    [[nodiscard]] QDateTime dateTime() const;

    /*!
      Sets the date.
    */
    void setDateTime(const QDateTime &dt);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Date)
};

class NewsgroupsPrivate;

/*!
  Represents a "Newsgroups" header.

  \sa RFC 1036, section 2.1.3.
*/
class KMIME_EXPORT Newsgroups : public Generics::Structured
{

    kmime_mk_trivial_ctor_with_name(Newsgroups)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    void fromUnicodeString(const QString &s) override;
    using Base::fromUnicodeString;
    [[nodiscard]] QString asUnicodeString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the list of newsgroups.
    */
    [[nodiscard]] QList<QByteArray> groups() const;

    /*!
      Sets the newsgroup list.
    */
    void setGroups(const QList<QByteArray> &groups);

    /*!
      Returns true if this message has been cross-posted, i.e. if it has been
      posted to multiple groups.
    */
    [[nodiscard]] bool isCrossposted() const;

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Newsgroups)
};

/*!
  Represents a "Followup-To" header.

  \sa RFC 1036, section 2.2.3.
*/
class KMIME_EXPORT FollowUpTo : public Newsgroups
{

    kmime_mk_trivial_ctor_with_name(FollowUpTo)
    //@endcond
};

class LinesPrivate;

/*!
  Represents a "Lines" header.

  \sa RFC 1036, section 2.2.12.
*/
class KMIME_EXPORT Lines : public Generics::Structured
{

    kmime_mk_trivial_ctor_with_name(Lines)
    //@endcond
public:
    [[nodiscard]] QByteArray as7BitString() const override;
    [[nodiscard]] QString asUnicodeString() const override;
    [[nodiscard]] bool isEmpty() const override;

    /*!
      Returns the number of lines, undefined if isEmpty() returns true.
    */
    [[nodiscard]] int numberOfLines() const;

    /*!
      Sets the number of lines to \a lines.
    */
    void setNumberOfLines(int lines);

protected:
    bool parse(const char *&scursor, const char *const send, NewlineType newline = NewlineType::LF) override;

private:
    Q_DECLARE_PRIVATE(Lines)
};

/*!
  Represents a "User-Agent" header.
*/
class KMIME_EXPORT UserAgent : public Generics::Unstructured
{
    kmime_mk_trivial_ctor_with_name(UserAgent)
};

/*! Creates a header based on \a type. If \a type is a known header type,
 * the right object type will be created, otherwise a null pointer is returned. */
[[nodiscard]] KMIME_EXPORT std::unique_ptr<Base> createHeader(QByteArrayView type);

}  //namespace Headers

}  //namespace KMime

// undefine code generation macros again
#undef kmime_mk_trivial_ctor
#undef kmime_mk_dptr_ctor
#undef kmime_mk_trivial_ctor_with_name

Q_DECLARE_METATYPE(KMime::Headers::To*)
Q_DECLARE_METATYPE(KMime::Headers::Cc*)
Q_DECLARE_METATYPE(KMime::Headers::Bcc*)

