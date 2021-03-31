/*
    kmime_newsarticle.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kmime_export.h"
#include "kmime_message.h"

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
      Returns the Supersedes header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Supersedes *supersedes(bool create = true);

    /**
      Returns the Mail-Copies-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::MailCopiesTo *mailCopiesTo(bool create = true);

    /**
      Returns the Newsgroups header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Newsgroups *newsgroups(bool create = true);

    /**
      Returns the Follow-Up-To header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::FollowUpTo *followUpTo(bool create = true);

    /**
      Returns the Lines header.
      @param create If true, create the header if it doesn't exist yet.
    */
    KMime::Headers::Lines *lines(bool create = true);

protected:
    QByteArray assembleHeaders() override;
}; // class NewsArticle

} // namespace KMime

