/*
    kmime_message.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_message.h"
#include "kmime_util_p.h"

using namespace KMime;

namespace KMime
{

Message::Message() : Content()
{
}

Message::~Message() = default;

QByteArray Message::assembleHeaders()
{
    // Create the mandatory fields (RFC5322) if they do not exist already.
    date(true);
    from(true);

    // Make sure the mandatory MIME-Version field (RFC2045) is present and valid.
    auto *mimeVersion = header<Headers::MIMEVersion>(true);
    mimeVersion->from7BitString("1.0");

    // Assemble all header fields.
    return Content::assembleHeaders();
}

Content *Message::mainBodyPart(const QByteArray &type)
{
    KMime::Content *c = this;
    while (c) {
        // not a multipart message
        const KMime::Headers::ContentType *const contentType = c->contentType();
        if (!contentType->isMultipart()) {
            if (contentType->mimeType() == type || type.isEmpty()) {
                return c;
            }
            return nullptr;
        }

        // empty multipart
        if (c->contents().isEmpty()) {
            return nullptr;
        }

        // multipart/alternative
        if (contentType->subType() == "alternative") {
            if (type.isEmpty()) {
                return c->contents().at(0);
            }
            const auto contents = c->contents();
            for (Content *c1 : contents) {
                if (c1->contentType()->mimeType() == type) {
                    return c1;
                }
            }
            return nullptr;
        }

        c = c->contents().at(0);
    }

    return nullptr;
}

QString Message::mimeType()
{
    return QStringLiteral("message/rfc822");
}

// @cond PRIVATE
#define kmime_mk_header_accessor( type, method ) \
    Headers::type *Message::method( bool create ) { \
        return header<Headers::type>( create ); \
    }

kmime_mk_header_accessor(MessageID, messageID)
kmime_mk_header_accessor(Subject, subject)
kmime_mk_header_accessor(Date, date)
kmime_mk_header_accessor(Organization, organization)
kmime_mk_header_accessor(From, from)
kmime_mk_header_accessor(ReplyTo, replyTo)
kmime_mk_header_accessor(To, to)
kmime_mk_header_accessor(Cc, cc)
kmime_mk_header_accessor(Bcc, bcc)
kmime_mk_header_accessor(References, references)
kmime_mk_header_accessor(UserAgent, userAgent)
kmime_mk_header_accessor(InReplyTo, inReplyTo)
kmime_mk_header_accessor(Sender, sender)

#undef kmime_mk_header_accessor
// @endcond

}

