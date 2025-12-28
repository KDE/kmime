/*
    kmime_newsarticle.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kmime_export.h"
#include "message.h"

#include <QSharedPointer>

namespace KMime
{

/*!
 * \class KMime::NewsArticle
 * \inmodule KMime
 * \inheaderfile KMime/NewsArticle
 *
 * \brief NNTP news article.
 */
class KMIME_EXPORT NewsArticle : public Message
{
public:
    // needed for Akonadi polymorphic payload support
    typedef Message SuperClass;

    /*!
      Creates a NewsArticle object.
    */
    NewsArticle();

    ~NewsArticle() override;

    /*!
      Returns the Control header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Control *control(CreatePolicy create = Create);
    /*!
      Returns the Control header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Control *control() const;

    /*!
      Returns the Supersedes header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Supersedes *supersedes(CreatePolicy create = Create);
    /*!
      Returns the Supersedes header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Supersedes *supersedes() const;

    /*!
      Returns the Mail-Copies-To header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::MailCopiesTo *mailCopiesTo(CreatePolicy create = Create);
    /*!
      Returns the Mail-Copies-To header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::MailCopiesTo *mailCopiesTo() const;

    /*!
      Returns the Newsgroups header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Newsgroups *newsgroups(CreatePolicy create = Create);
    /*!
      Returns the Newsgroups header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Newsgroups *newsgroups() const;

    /*!
      Returns the Follow-Up-To header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::FollowUpTo *followUpTo(CreatePolicy create = Create);
    /*!
      Returns the Follow-Up-To header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::FollowUpTo *followUpTo() const;

    /*!
      Returns the Lines header.

      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Lines *lines(CreatePolicy create = Create);
    /*!
      Returns the Lines header.

      Can be \nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Lines *lines() const;

    void assemble() override;
    [[nodiscard]] std::unique_ptr<Content> clone() const override;
}; // class NewsArticle

} // namespace KMime

