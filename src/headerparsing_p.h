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

