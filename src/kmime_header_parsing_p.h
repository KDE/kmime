/*
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QVector>

class QByteArray;

namespace KMime
{

namespace Headers
{
class Base;
}
namespace HeaderParsing
{

Q_REQUIRED_RESULT QVector<KMime::Headers::Base *> parseHeaders(const QByteArray &head);

}

}

