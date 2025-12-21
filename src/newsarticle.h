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

/*! NNTP news article. */
class KMIME_EXPORT NewsArticle : public Message
{
public:
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    /*!
      A shared pointer to a news article.
    */
    KMIME_DEPRECATED typedef std::shared_ptr<NewsArticle> Ptr;
#endif
    // needed for Akonadi polymorphic payload support
    typedef Message SuperClass;
    ///@endcond

    /*!
      Creates a NewsArticle object.
    */
    NewsArticle();

    /*!
      Destroys this NewsArticle.
    */
    ~NewsArticle() override;

    /*!
      Returns the Control header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Control *control(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::Control *control(bool create) {
        return control(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Control header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Control *control() const;

    /*!
      Returns the Supersedes header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Supersedes *supersedes(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::Supersedes *supersedes(bool create) {
        return supersedes(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Supersedes header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Supersedes *supersedes() const;

    /*!
      Returns the Mail-Copies-To header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::MailCopiesTo *mailCopiesTo(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::MailCopiesTo *mailCopiesTo(bool create) {
        return mailCopiesTo(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Mail-Copies-To header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::MailCopiesTo *mailCopiesTo() const;

    /*!
      Returns the Newsgroups header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Newsgroups *newsgroups(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::Newsgroups *newsgroups(bool create) {
        return newsgroups(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Newsgroups header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Newsgroups *newsgroups() const;

    /*!
      Returns the Follow-Up-To header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::FollowUpTo *followUpTo(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::FollowUpTo *followUpTo(bool create) {
        return followUpTo(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Follow-Up-To header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::FollowUpTo *followUpTo() const;

    /*!
      Returns the Lines header.
      \a create Whether to create the header if it doesn't exist yet.
    */
    KMime::Headers::Lines *lines(CreatePolicy create = Create);
#if KMIME_ENABLE_DEPRECATED_SINCE(6, 7)
    [[deprecated("use the CreatePolicy overload instead")]] inline KMime::Headers::Lines *lines(bool create) {
        return lines(create ? Create : DontCreate);
    }
#endif
    /*!
      Returns the Lines header.
      Can be nullptr if the header doesn't exist.
      \since 24.08
    */
    [[nodiscard]] const KMime::Headers::Lines *lines() const;

protected:
    QByteArray assembleHeaders() override;
}; // class NewsArticle

} // namespace KMime

