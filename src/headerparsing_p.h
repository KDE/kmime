/*
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "headers_p.h"

#include <QList>

class QByteArray;

namespace KMime
{

namespace Headers
{
class Base;
}
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
[[nodiscard]] bool parseEncodedWord(const char *&scursor, const char *const send, QString &result,
                 QByteArray &language, QByteArray &usedCS, const QByteArray &defaultCS = QByteArray());

[[nodiscard]] QList<KMime::Headers::Base *> parseHeaders(const QByteArray &head);

enum ParseTokenFlag {
    ParseTokenNoFlag = 0,
    ParseTokenAllow8Bit = 1,
    ParseTokenRelaxedTText = 2
};
Q_DECLARE_FLAGS(ParseTokenFlags, ParseTokenFlag)

/** You may or may not have already started parsing into the
    token. This function will go on where you left off. */
[[nodiscard]] bool parseToken(const char *&scursor, const char *const send,
                              QByteArrayView &result, ParseTokenFlags flags = ParseTokenNoFlag);

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
[[nodiscard]] bool parseDotAtom(const char *&scursor, const char *const send,
                                QByteArrayView &result, bool isCRLF = false);

/** @p scursor must be positioned after the opening openChar. */
[[nodiscard]] bool parseGenericQuotedString(const char *&scursor, const char *const send,
                         QString &result, bool isCRLF,
                         const char openChar = '"', const char closeChar = '"');

/** @p scursor must be positioned right after the opening '(' */
[[nodiscard]] bool parseComment(const char *&scursor, const char *const send, QString &result,
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
[[nodiscard]] bool parsePhrase(const char *&scursor, const char *const send,
                               QString &result, bool isCRLF = false);

/**
 * Extract the charset embedded in the parameter list if there is one.
 *
 * @since 4.5
 */
[[nodiscard]] bool parseParameterListWithCharset(const char *&scursor, const char *const send,
                              KMime::Headers::ParameterMap &result,
                              QByteArray &charset, bool isCRLF = false);


[[nodiscard]] bool parseDomain(const char *&scursor, const char *const send,
                               QString &result, bool isCRLF = false);

[[nodiscard]] bool parseObsRoute(const char *&scursor, const char *const send, QStringList &result,
                                 bool isCRLF = false, bool save = false);

[[nodiscard]] bool parseAddrSpec(const char *&scursor, const char *const send,
                                 Types::AddrSpec &result, bool isCRLF = false);

[[nodiscard]] bool parseAngleAddr(const char *&scursor, const char *const send,
                                  Types::AddrSpec &result, bool isCRLF = false);

/**
  Parses an integer number.
  @param scursor pointer to the first character of the input string
  @param send pointer to end of input buffer
  @param result the parsing result
  @returns The number of parsed digits (don't confuse with @p result!)
*/
[[nodiscard]] int parseDigits(const char *&scursor, const char *const send, int &result);

[[nodiscard]] bool parseTime(const char *&scursor, const char *const send, int &hour, int &min,
          int &sec, long int &secsEastOfGMT, bool &timeZoneKnown,
          bool isCRLF = false);

[[nodiscard]] bool parseDateTime(const char *&scursor, const char *const send,
                                QDateTime &result, bool isCRLF = false);
[[nodiscard]] bool parseQDateTime(const char *&scursor, const char *const send,
                                QDateTime &result, bool isCRLF = false);

}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KMime::HeaderParsing::ParseTokenFlags)
