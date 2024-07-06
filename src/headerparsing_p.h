/*
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QList>

class QByteArray;
template <typename K, typename V> class QMap;

namespace KMime
{

namespace Headers
{
class Base;
}
namespace HeaderParsing
{

[[nodiscard]] QList<KMime::Headers::Base *> parseHeaders(const QByteArray &head);

[[nodiscard]] bool parseParameterList(const char *&scursor, const char *const send,
                   QMap<QString, QString> &result, bool isCRLF = false);

/**
 * Extract the charset embedded in the parameter list if there is one.
 *
 * @since 4.5
 */
[[nodiscard]] bool parseParameterListWithCharset(const char *&scursor, const char *const send,
                              QMap<QString, QString> &result,
                              QByteArray &charset, bool isCRLF = false);
}

}

