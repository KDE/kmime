/*
    kmime_message.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kmime_export.h"
#include "content.h"
#include "headers.h"

#include <QMetaType>
#include <QSharedPointer>

namespace KMime
{

class MessagePrivate;

/*!
 * \class KMime::Message
 * \inmodule KMime
 * \inheaderfile KMime/Message
 *
 * \brief Represents a (email) message.
 *
 * Sample how to create a multipart message:
 * \code
 * // Set the multipart message.
 * auto m = std::make_shared<KMime::Message>();
 * auto ct = m->contentType();
 * ct->setMimeType("multipart/mixed");
 * ct->setBoundary(KMime::multiPartBoundary());
 * auto cte = m->contentTransferEncoding();
 * cte->setEncoding(KMime::Headers::CE7Bit);
 *
 * // Set the headers.
 * m->from()->fromUnicodeString("some@mailaddy.com");
 * m->to()->fromUnicodeString("someother@mailaddy.com");
 * m->cc()->fromUnicodeString("some@mailaddy.com");
 * m->date()->setDateTime(QDateTime::currentDateTime());
 * m->subject()->fromUnicodeString("My Subject");
 *
 * // Set the first multipart, the body message.
 * auto b = std::make_unique<KMime::Content>();
 * b->contentType()->setMimeType("text/plain");
 * b->setBody("Some text...");
 *
 * // Set the second multipart, the attachment.
 * auto a = std::make_unique<KMime::Content>();
 * auto d = std::make_unique<KMime::Headers::ContentDisposition>();
 * d->setFilename("cal.ics");
 * d->setDisposition(KMime::Headers::CDattachment);
 * a->contentType()->setMimeType("text/plain");
 * a->setHeader(std::move(d));
 * a->setBody("Some text in the attachment...");
 *
 * // Attach the both multiparts and assemble the message.
 * m->appendContent(std::move(b));
 * m->appendContent(std::move(a));
 * m->assemble();
 * \endcode
 */
class KMIME_EXPORT Message : public Content
{
public:
    /*!
      Creates an empty Message.
    */
    Message();

    ~Message() override;

    /*!
      Returns the Message-ID header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::MessageID *messageID(CreatePolicy create = Create);
    /*!
      Returns the Message-ID header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::MessageID *messageID() const;

    /*!
      Returns the Subject header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Subject *subject(CreatePolicy create = Create);
    /*!
      Returns the Subject header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Subject *subject() const;

    /*!
      Returns the Date header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Date *date(CreatePolicy create = Create);
    /*!
      Returns the Date header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Date *date() const;

    /*!
      Returns the From header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::From *from(CreatePolicy create = Create);
    /*!
      Returns the From header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::From *from() const;

    /*!
      Returns the Organization header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Organization *organization(CreatePolicy create = Create);
    /*!
      Returns the Organization header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Organization *organization() const;

    /*!
      Returns the Reply-To header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::ReplyTo *replyTo(CreatePolicy create = Create);
    /*!
      Returns the Reply-To header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::ReplyTo *replyTo() const;

    /*!
      Returns the To header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::To *to(CreatePolicy create = Create);
    /*!
      Returns the To header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::To *to() const;

    /*!
      Returns the Cc header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Cc *cc(CreatePolicy create = Create);
    /*!
      Returns the Cc header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Cc *cc() const;

    /*!
      Returns the Bcc header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Bcc *bcc(CreatePolicy create = Create);
    /*!
      Returns the Bcc header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Bcc *bcc() const;

    /*!
      Returns the References header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::References *references(CreatePolicy create = Create);
    /*!
      Returns the References header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::References *references() const;

    /*!
      Returns the User-Agent header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::UserAgent *userAgent(CreatePolicy create = Create);
    /*!
      Returns the User-Agent header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::UserAgent *userAgent() const;

    /*!
      Returns the In-Reply-To header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::InReplyTo *inReplyTo(CreatePolicy create = Create);
    /*!
      Returns the In-Reply-To header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::InReplyTo *inReplyTo() const;

    /*!
      Returns the Sender header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Sender *sender(CreatePolicy create = Create);
    /*!
      Returns the Sender header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Sender *sender() const;

    /*!
      Returns the first main body part of a given type, taking multipart/mixed
      and multipart/alternative nodes into consideration.

      Eg. \c bodyPart("text/html") will return a html content object if that is
      provided in a multipart/alternative node, but not if it's the non-first
      child node of a multipart/mixed node (ie. an attachment).

      \a type The mimetype of the body part, if not given, the first
      body part will be returned, regardless of it's type.
    */
    [[nodiscard]] Content *mainBodyPart(const QByteArray &type = QByteArray());
    [[nodiscard]] const Content *mainBodyPart(const QByteArray &type = QByteArray()) const;

    /*!
      Returns the MIME type used for Messages
    */
    [[nodiscard]] static QString mimeType();

    void assemble() override;
    [[nodiscard]] std::unique_ptr<Content> clone() const override;
private:
    Q_DECLARE_PRIVATE(Message)

}; // class Message

} // namespace KMime

Q_DECLARE_METATYPE(KMime::Message*)
Q_DECLARE_METATYPE(std::shared_ptr<KMime::Message>)

