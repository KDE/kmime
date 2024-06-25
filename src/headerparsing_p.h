/*
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

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

[[nodiscard]] QList<KMime::Headers::Base *>
parseHeaders(const QByteArray &head);
}

}

