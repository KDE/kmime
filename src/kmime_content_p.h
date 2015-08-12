/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMIME_CONTENT_P_H
#define KMIME_CONTENT_P_H

//@cond PRIVATE

#include <QSharedPointer>

namespace KMime
{
class Message;
typedef QSharedPointer<Message> MessagePtr;
}

namespace KMime
{

class ContentPrivate
{
public:
    explicit ContentPrivate() :
        parent(0),
        frozen(false)
    {
    }

    ~ContentPrivate()
    {
        qDeleteAll(multipartContents);
        multipartContents.clear();
    }

    bool parseUuencoded(Content *q);
    bool parseYenc(Content *q);
    bool parseMultipart(Content *q);
    void clearBodyMessage();

    bool decodeText(Content *q);

    // This one returns the normal multipartContents for multipart contents, but returns
    // a list with just bodyAsMessage in it for contents that are encapsulated messages.
    // That makes it possible to handle encapsulated messages in a transparent way.
    QVector<Content*> contents() const;

    QByteArray head;
    QByteArray body;
    QByteArray frozenBody;
    QByteArray preamble;
    QByteArray epilogue;
    Content *parent;

    QVector<Content*> multipartContents;
    MessagePtr bodyAsMessage;

    QVector<Headers::Base*> headers;

    bool frozen : 1;
};

}

//@endcond

#endif
