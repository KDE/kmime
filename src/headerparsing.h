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

namespace Headers
{
class Base;
}

namespace HeaderParsing
{

//
// The parsing squad:
//

/** You may or may not have already started parsing into the
    atom. This function will go on where you left off.
 */
[[nodiscard]] KMIME_EXPORT bool parseAtom(const char *&scursor,
                                          const char *const send,
                                          QByteArrayView &result,
                                          bool allow8Bit = false);

[[deprecated("Use the QByteArrayView overload")]] [[nodiscard]]
inline bool parseAtom(const char *&scursor, const char *const send, QByteArray &result, bool allow8Bit = false)
{
    QByteArrayView v;
    const auto r = parseAtom(scursor, send, v, allow8Bit);
    result = v.toByteArray();
    return r;
}

/**
  Eats comment-folding-white-space, skips whitespace, folding and comments
  (even nested ones) and stops at the next non-CFWS character.  After
  calling this function, you should check whether @p scursor == @p send
  (end of header reached).

  If a comment with unbalanced parentheses is encountered, @p scursor
  is being positioned on the opening '(' of the outmost comment.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param isCRLF true if input string is terminated with a CRLF.
*/
KMIME_EXPORT void eatCFWS(const char *&scursor, const char *const send,
                          bool isCRLF);

/**
  Parses a single mailbox.

  RFC 2822, section 3.4 defines a mailbox as follows:
  <pre>mailbox := addr-spec / ([ display-name ] angle-addr)</pre>

  KMime also accepts the legacy format of specifying display names:
  <pre>mailbox := (addr-spec [ "(" display-name ")" ])
  / ([ display-name ] angle-addr)
  / (angle-addr "(" display-name ")")</pre>

  @param scursor pointer to the first character of the input string
  @param send pointer to end of input buffer
  @param result the parsing result
  @param isCRLF true if input string is terminated with a CRLF.
*/
KMIME_EXPORT bool parseMailbox(const char *&scursor, const char *const send,
                               Types::Mailbox &result, bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool parseGroup(const char *&scursor,
                                           const char *const send,
                                           Types::Address &result,
                                           bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool parseAddress(const char *&scursor,
                                             const char *const send,
                                             Types::Address &result,
                                             bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool parseAddressList(const char *&scursor,
                                                 const char *const send,
                                                 Types::AddressList &result,
                                                 bool isCRLF = false);

/** Parses the first header contained the given data.
 *  If a header is found @p head will be shortened to no longer
 *  include the corresponding data, ie. this method can be called
 *  iteratively on the same data until it returns @c null.
 *  @since 6.0
 */
[[nodiscard]] KMIME_EXPORT std::unique_ptr<KMime::Headers::Base> parseNextHeader(QByteArrayView &head);

/**
 * Extract the header header and the body from a complete content.
 * Internally, it will simply look for the first newline and use that as a
 * separator between the header and the body.
 *
 * @param content the complete mail
 * @param header return value for the extracted header
 * @param body return value for the extracted body
 * @since 4.6
 */
KMIME_EXPORT void extractHeaderAndBody(const QByteArray &content,
                                       QByteArray &header, QByteArray &body);

} // namespace HeaderParsing

} // namespace KMime
