/*
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KMIME_HEADER_PARSING_P_H
#define KMIME_HEADER_PARSING_P_H

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

#endif
