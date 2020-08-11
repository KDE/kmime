/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    Content *parent = nullptr;

    QVector<Content*> multipartContents;
    MessagePtr bodyAsMessage;

    QVector<Headers::Base*> headers;

    bool frozen : 1;
};

}

//@endcond

#endif
