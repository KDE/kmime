/*  -*- c++ -*-
    kmime_header_parsing.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kmime_export.h"
#include "types.h"

#include <QDateTime>
#include <QString>
#include <QStringList>

namespace KMime
{

/*!
 * \namespace KMime::HeaderParsing
 * \inmodule KMime
 * \inheaderfile KMime/HeaderParsing
 */
namespace Headers
{
class Base;
}

/*!
 * Line endings to use.
 *
 * \value LF
 * \value CRLF
 *
 * \since 26.04
 */
enum class NewlineType {
    LF,
    CRLF,
};

/*!
  Low-level MIME header parsing functions.
  These are only exported for KAlarm legacy code, use the header classes directly
  whenever possible.
*/
namespace HeaderParsing
{

/*!
 * Policy for parseAtom().
 *
 * \value Allow7BitOnly Stop parsing if an 8-bit character is encountered
 * \value Allow8Bit Accept 8-bit characters
 *
 * \since 26.04
 */
enum class ParsingPolicy {
    Allow7BitOnly,
    Allow8Bit,
};

//
// The parsing squad:
//

/*! You may or may not have already started parsing into the
    atom. This function will go on where you left off.
 */
[[nodiscard]] KMIME_EXPORT bool parseAtom(const char *&scursor,
                                          const char *const send,
                                          QByteArrayView &result,
                                          ParsingPolicy parsingPolicy = ParsingPolicy::Allow7BitOnly);

/*!
  Eats comment-folding-white-space, skips whitespace, folding and comments
  (even nested ones) and stops at the next non-CFWS character.  After
  calling this function, you should check whether \a scursor == \a send
  (end of header reached).

  If a comment with unbalanced parentheses is encountered, \a scursor
  is being positioned on the opening '(' of the outmost comment.

  \a scursor pointer to the first character beyond the initial '=' of
  the input string.

  \a send pointer to end of input buffer.

  \a newline whether the input string is terminated with CRLF or LF.
*/
KMIME_EXPORT void eatCFWS(const char *&scursor, const char *const send,
                          NewlineType newline);

/*!
  Parses a single mailbox.

  RFC 2822, section 3.4 defines a mailbox as follows:
  \badcode
  mailbox := addr-spec / ([ display-name ] angle-addr)
  \endcode

  KMime also accepts the legacy format of specifying display names:
  \badcode
  mailbox := (addr-spec [ "(" display-name ")" ])
  / ([ display-name ] angle-addr)
  / (angle-addr "(" display-name ")")
  \endcode

  \a scursor pointer to the first character of the input string

  \a send pointer to end of input buffer

  \a result the parsing result

  \a newline whether the input string is terminated with CRLF or LF.
*/
KMIME_EXPORT bool parseMailbox(const char *&scursor, const char *const send,
                               Types::Mailbox &result, NewlineType newline = NewlineType::LF);

/*!
 */
[[nodiscard]] KMIME_EXPORT bool parseGroup(const char *&scursor,
                                           const char *const send,
                                           Types::Address &result,
                                           NewlineType newline = NewlineType::LF);

/*!
 */
[[nodiscard]] KMIME_EXPORT bool parseAddress(const char *&scursor,
                                             const char *const send,
                                             Types::Address &result,
                                             NewlineType newline = NewlineType::LF);

/*!
 */
[[nodiscard]] KMIME_EXPORT bool parseAddressList(const char *&scursor,
                                                 const char *const send,
                                                 QList<Types::Address> &result,
                                                 NewlineType newline = NewlineType::LF);

/*! Parses the first header contained the given data.
 *
 *  If a header is found \a head will be shortened to no longer
 *  include the corresponding data, ie. this method can be called
 *  iteratively on the same data until it returns \nullptr.
 *  \since 6.0
 */
[[nodiscard]] KMIME_EXPORT std::unique_ptr<KMime::Headers::Base> parseNextHeader(QByteArrayView &head);

/*!
 * Extract the header header and the body from a complete content.
 *
 * Internally, it will simply look for the first newline and use that as a
 * separator between the header and the body.
 *
 * \a content the complete mail
 *
 * \a header return value for the extracted header
 *
 * \a body return value for the extracted body
 * \since 4.6
 */
KMIME_EXPORT void extractHeaderAndBody(const QByteArray &content,
                                       QByteArray &header, QByteArray &body);

} // namespace HeaderParsing

} // namespace KMime
