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

/**
 * Extract the charset embedded in the parameter list if there is one.
 *
 * @since 4.5
 */
[[nodiscard]] bool parseParameterListWithCharset(const char *&scursor, const char *const send,
                              KMime::Headers::ParameterMap &result,
                              QByteArray &charset, bool isCRLF = false);
}

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KMime::HeaderParsing::ParseTokenFlags)
