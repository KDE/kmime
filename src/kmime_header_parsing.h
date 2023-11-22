/*  -*- c++ -*-
    kmime_header_parsing.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kmime_export.h"
#include "kmime_types.h"

#include <QString>
#include <QPair>

#include <QDateTime>

template <typename K, typename V> class QMap;
#include <QStringList>

namespace KMime
{

namespace Headers
{
class Base;
}

namespace Types
{

} // namespace KMime::Types

namespace HeaderParsing
{

/**
  Parses the encoded word.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the decoded string the encoded work represented.
  @param language The language parameter according to RFC 2231, section 5.
  @param usedCS    the used charset is returned here
  @param defaultCS the charset to use in case the detected
                   one isn't known to us.

  @return true if the input string was successfully decode; false otherwise.
*/
[[nodiscard]] KMIME_EXPORT bool
parseEncodedWord(const char *&scursor, const char *const send, QString &result,
                 QByteArray &language, QByteArray &usedCS,
                 const QByteArray &defaultCS = QByteArray());

//
// The parsing squad:
//

/** You may or may not have already started parsing into the
    atom. This function will go on where you left off.
 */
[[nodiscard]] KMIME_EXPORT bool parseAtom(const char *&scursor,
                                          const char *const send,
                                          QByteArray &result,
                                          bool allow8Bit = false);

/**
 * More efficient overload, to avoid a copy of the substring
 */
[[nodiscard]] KMIME_EXPORT bool parseAtom(const char *&scursor,
                                          const char *const send,
                                          QPair<const char *, int> &result,
                                          bool allow8Bit = false);

enum ParseTokenFlag {
    ParseTokenNoFlag = 0,
    ParseTokenAllow8Bit = 1,
    ParseTokenRelaxedTText = 2
};
Q_DECLARE_FLAGS(ParseTokenFlags, ParseTokenFlag)

/** You may or may not have already started parsing into the
    token. This function will go on where you left off. */
[[nodiscard]] KMIME_EXPORT bool
parseToken(const char *&scursor, const char *const send, QByteArray &result,
           ParseTokenFlags flags = ParseTokenNoFlag);

[[nodiscard]] KMIME_EXPORT bool
parseToken(const char *&scursor, const char *const send,
           QPair<const char *, int> &result,
           ParseTokenFlags flags = ParseTokenNoFlag);

/** @p scursor must be positioned after the opening openChar. */
[[nodiscard]] KMIME_EXPORT bool
parseGenericQuotedString(const char *&scursor, const char *const send,
                         QString &result, bool isCRLF,
                         const char openChar = '"', const char closeChar = '"');

/** @p scursor must be positioned right after the opening '(' */
[[nodiscard]] KMIME_EXPORT bool
parseComment(const char *&scursor, const char *const send, QString &result,
             bool isCRLF = false, bool reallySave = true);

/**
  Parses a phrase.

  You may or may not have already started parsing into the phrase, but
  only if it starts with atext. If you setup this function to parse a
  phrase starting with an encoded-word or quoted-string, @p scursor has
  to point to the char introducing the encoded-word or quoted-string, resp.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the parsed string.

  @return true if the input phrase was successfully parsed; false otherwise.
*/
[[nodiscard]] KMIME_EXPORT bool parsePhrase(const char *&scursor,
                                            const char *const send,
                                            QString &result,
                                            bool isCRLF = false);

/**
  Parses into the initial atom.
  You may or may not have already started parsing into the initial
  atom, but not up to it's end.

  @param scursor pointer to the first character beyond the initial '=' of
  the input string.
  @param send pointer to end of input buffer.
  @param result the parsed string.

  @return true if the input phrase was successfully parsed; false otherwise.
*/
[[nodiscard]] KMIME_EXPORT bool parseDotAtom(const char *&scursor,
                                             const char *const send,
                                             QByteArray &result,
                                             bool isCRLF = false);

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

[[nodiscard]] KMIME_EXPORT bool parseDomain(const char *&scursor,
                                            const char *const send,
                                            QString &result,
                                            bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool
parseObsRoute(const char *&scursor, const char *const send, QStringList &result,
              bool isCRLF = false, bool save = false);

[[nodiscard]] KMIME_EXPORT bool parseAddrSpec(const char *&scursor,
                                              const char *const send,
                                              Types::AddrSpec &result,
                                              bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool parseAngleAddr(const char *&scursor,
                                               const char *const send,
                                               Types::AddrSpec &result,
                                               bool isCRLF = false);

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

[[nodiscard]] KMIME_EXPORT bool
parseParameterList(const char *&scursor, const char *const send,
                   QMap<QString, QString> &result, bool isCRLF = false);

/**
 * Extract the charset embedded in the parameter list if there is one.
 *
 * @since 4.5
 */
[[nodiscard]] KMIME_EXPORT bool
parseParameterListWithCharset(const char *&scursor, const char *const send,
                              QMap<QString, QString> &result,
                              QByteArray &charset, bool isCRLF = false);

/**
  Parses an integer number.
  @param scursor pointer to the first character of the input string
  @param send pointer to end of input buffer
  @param result the parsing result
  @returns The number of parsed digits (don't confuse with @p result!)
*/
[[nodiscard]] KMIME_EXPORT int parseDigits(const char *&scursor,
                                           const char *const send, int &result);

[[nodiscard]] KMIME_EXPORT bool
parseTime(const char *&scursor, const char *const send, int &hour, int &min,
          int &sec, long int &secsEastOfGMT, bool &timeZoneKnown,
          bool isCRLF = false);

[[nodiscard]] KMIME_EXPORT bool parseDateTime(const char *&scursor,
                                              const char *const send,
                                              QDateTime &result,
                                              bool isCRLF = false);
[[nodiscard]] KMIME_EXPORT bool parseQDateTime(const char *&scursor,
                                               const char *const send,
                                               QDateTime &result,
                                               bool isCRLF = false);

/**
 * Extracts and returns the first header that is contained in the given byte array.
 * The header will also be removed from the passed-in byte array head.
 *
 * @since 4.4
 */
[[nodiscard]] [[deprecated("use parseNextHeader")]] KMIME_EXPORT KMime::Headers::Base *
extractFirstHeader(QByteArray &head);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(KMime::HeaderParsing::ParseTokenFlags)


