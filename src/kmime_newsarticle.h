/*
    kmime_newsarticle.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __KMIME_NEWSARTICLE_H__
#define __KMIME_NEWSARTICLE_H__

#include "kmime_export.h"
#include "kmime_message.h"
#include <supertrait.h>

#include <QSharedPointer>

namespace KMime
{

class KMIME_EXPORT NewsArticle : public Message
{
public:
    /**
      A shared pointer to a news article.
    */
    typedef QSharedPointer<NewsArticle> Ptr;

    /**
      Creates a NewsArticle object.
    */
    NewsArticle();

    /**
      Destroys this NewsArticle.
    */
    ~NewsArticle();

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

//@cond PRIVATE
// super class trait specialization
namespace Akonadi
{
template <> struct SuperClass<KMime::NewsArticle> : public SuperClassTrait<KMime::Message> {};
}
//@endcond

#endif // __KMIME_NEWSARTICLE_H__
