/*
    kmime_message.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "content.h"
#include "headers.h"
#include "kmime_export.h"

#include <QMetaType>
#include <QSharedPointer>

namespace KMime
{

class MessagePrivate;

/**
 * Represents a (email) message.
 *
 * Sample how to create a multipart message:
 * \code
 * // Set the multipart message.
 * Message *m = new Message;
 * Headers::ContentType *ct = m->contentType();
 * ct->setMimeType( "multipart/mixed" );
 * ct->setBoundary( multiPartBoundary() );
 * Headers::ContentTransferEncoding *cte = m->contentTransferEncoding();
 * cte->setEncoding(Headers::CE7Bit);
 *
 * // Set the headers.
 * m->from()->fromUnicodeString("some@mailaddy.com");
 * m->to()->fromUnicodeString("someother@mailaddy.com");
 * m->cc()->fromUnicodeString("some@mailaddy.com");
 * m->date()->setDateTime(QDateTime::currentLocalDateTime());
 * m->subject()->fromUnicodeString("My Subject");
 *
 * // Set the first multipart, the body message.
 * KMime::Content *b = new KMime::Content;
 * b->contentType()->setMimeType( "text/plain" );
 * b->setBody( "Some text..." );
 *
 * // Set the second multipart, the attachment.
 * KMime::Content *a = new KMime::Content;
 * KMime::Headers::ContentDisposition *d = new KMime::Headers::ContentDisposition( attachMessage );
 * d->setFilename( "cal.ics" );
 * d->setDisposition( KMime::Headers::CDattachment );
 * a->contentType()->setMimeType( "text/plain" );
 * a->setHeader( d );
 * a->setBody( "Some text in the attachment..." );
 *
 * // Attach the both multiparts and assemble the message.
 * m->appendContent( b );
 * m->appendContent( a );
 * m->assemble();
 * \endcode
 */
class KMIME_EXPORT Message : public Content
{
public:
    /**
      A shared pointer to a message object.
    */
    typedef QSharedPointer<Message> Ptr;
    /**
      Creates an empty Message.
    */
    Message();

    /**
      Destroys this Message.
    */
    ~Message() override;

    /**
      Returns the Message-ID header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::MessageID *messageID(bool create = true);
    /**
      Returns the Message-ID header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::MessageID *messageID() const;

    /**
      Returns the Subject header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Subject *subject(bool create = true);
    /**
      Returns the Subject header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Subject *subject() const;

    /**
      Returns the Date header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Date *date(bool create = true);
    /**
      Returns the Date header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Date *date() const;

    /**
      Returns the From header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::From *from(bool create = true);
    /**
      Returns the From header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::From *from() const;

    /**
      Returns the Organization header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Organization *organization(bool create = true);
    /**
      Returns the Organization header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Organization *organization() const;

    /**
      Returns the Reply-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::ReplyTo *replyTo(bool create = true);
    /**
      Returns the Reply-To header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::ReplyTo *replyTo() const;

    /**
      Returns the To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::To *to(bool create = true);
    /**
      Returns the To header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::To *to() const;

    /**
      Returns the Cc header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Cc *cc(bool create = true);
    /**
      Returns the Cc header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Cc *cc() const;

    /**
      Returns the Bcc header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Bcc *bcc(bool create = true);
    /**
      Returns the Bcc header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Bcc *bcc() const;

    /**
      Returns the References header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::References *references(bool create = true);
    /**
      Returns the References header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::References *references() const;

    /**
      Returns the User-Agent header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::UserAgent *userAgent(bool create = true);
    /**
      Returns the User-Agent header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::UserAgent *userAgent() const;

    /**
      Returns the In-Reply-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::InReplyTo *inReplyTo(bool create = true);
    /**
      Returns the In-Reply-To header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::InReplyTo *inReplyTo() const;

    /**
      Returns the Sender header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Sender *sender(bool create = true);
    /**
      Returns the Sender header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Sender *sender() const;

    /**
      Returns the first main body part of a given type, taking multipart/mixed
      and multipart/alternative nodes into consideration.
      Eg. \c bodyPart("text/html") will return a html content object if that is
      provided in a multipart/alternative node, but not if it's the non-first
      child node of a multipart/mixed node (ie. an attachment).
      @param type The mimetype of the body part, if not given, the first
      body part will be returned, regardless of it's type.
    */
    [[nodiscard]] Content *mainBodyPart(const QByteArray &type = QByteArray());
    [[nodiscard]] const Content *mainBodyPart(const QByteArray &type = QByteArray()) const;

    /**
      Returns the MIME type used for Messages
    */
    [[nodiscard]] static QString mimeType();

protected:
    QByteArray assembleHeaders() override;

private:
    Q_DECLARE_PRIVATE(Message)

}; // class Message

} // namespace KMime

Q_DECLARE_METATYPE(KMime::Message *)
Q_DECLARE_METATYPE(KMime::Message::Ptr)
