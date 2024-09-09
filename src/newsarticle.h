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

/** NNTP news article. */
class KMIME_EXPORT NewsArticle : public Message
{
public:
    /**
      A shared pointer to a news article.
    */
    typedef QSharedPointer<NewsArticle> Ptr;

    ///@cond PRIVATE
    // needed for Akonadi polymorphic payload support
    typedef Message SuperClass;
    ///@endcond

    /**
      Creates a NewsArticle object.
    */
    NewsArticle();

    /**
      Destroys this NewsArticle.
    */
    ~NewsArticle() override;

    /**
      Returns the Control header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Control *control(bool create = true);
    /**
      Returns the Control header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Control *control() const;

    /**
      Returns the Supersedes header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Supersedes *supersedes(bool create = true);
    /**
      Returns the Supersedes header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Supersedes *supersedes() const;

    /**
      Returns the Mail-Copies-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::MailCopiesTo *mailCopiesTo(bool create = true);
    /**
      Returns the Mail-Copies-To header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::MailCopiesTo *mailCopiesTo() const;

    /**
      Returns the Newsgroups header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Newsgroups *newsgroups(bool create = true);
    /**
      Returns the Newsgroups header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Newsgroups *newsgroups() const;

    /**
      Returns the Follow-Up-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::FollowUpTo *followUpTo(bool create = true);
    /**
      Returns the Follow-Up-To header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::FollowUpTo *followUpTo() const;

    /**
      Returns the Lines header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Lines *lines(bool create = true);
    /**
      Returns the Lines header.
      Can be @c nullptr if the header doesn't exist.
      @since 24.08
    */
    [[nodiscard]] const KMime::Headers::Lines *lines() const;

protected:
    QByteArray assembleHeaders() override;
}; // class NewsArticle

} // namespace KMime
