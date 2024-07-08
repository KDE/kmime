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
