/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

//@cond PRIVATE

#include <QSharedPointer>

namespace KMime
{
class Content;
class Message;
using MessagePtr = QSharedPointer<Message>;

class ContentPrivate
{
public:
    explicit ContentPrivate() = default;
    ~ContentPrivate()
    {
        qDeleteAll(multipartContents);
        multipartContents.clear();
    }

    bool parseUuencoded(Content *q);
    bool parseYenc(Content *q);
    bool parseMultipart(Content *q);
    void clearBodyMessage();

    /**
      Returns whether the Content containing this header needs to be encoded
      (i.e., if decoded() is true and encoding() is base64 or quoted-printable).
    */
    [[nodiscard]] bool needToEncode(const Content *q) const;

    [[nodiscard]] bool decodeText(const Content *q);

    // This one returns the normal multipartContents for multipart contents, but returns
    // a list with just bodyAsMessage in it for contents that are encapsulated messages.
    // That makes it possible to handle encapsulated messages in a transparent way.
    QList<Content *> contents() const;

    QByteArray head;
    QByteArray body;
    QByteArray frozenBody;
    QByteArray preamble;
    QByteArray epilogue;
    Content *parent = nullptr;

    QList<Content *> multipartContents;
    MessagePtr bodyAsMessage;

    QList<Headers::Base *> headers;

    bool frozen : 1 = false;
    // Indicates whether body has content transfer encoding applied or not
    mutable bool m_decoded : 1 = true;
};

}

//@endcond
