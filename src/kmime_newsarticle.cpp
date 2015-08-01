/*
    kmime_newsarticle.cpp

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

#include "kmime_newsarticle.h"
#include "kmime_message_p.h"
#include "kmime_util_p.h"

using namespace KMime;

namespace KMime
{

class NewsArticlePrivate : public MessagePrivate
{
};

NewsArticle::NewsArticle()
    : Message(new NewsArticlePrivate)
{
}

NewsArticle::~NewsArticle()
{
}

QByteArray NewsArticle::assembleHeaders()
{
    // Create the mandatory Lines: field.
    lines(true);

    // Assemble all header fields.
    return Message::assembleHeaders();
}

// @cond PRIVATE
#define kmime_mk_header_accessor( type, method ) \
    Headers::type* NewsArticle::method( bool create ) { \
        return header<Headers::type>( create ); \
    }

kmime_mk_header_accessor(Control, control)
kmime_mk_header_accessor(Lines, lines)
kmime_mk_header_accessor(Supersedes, supersedes)
kmime_mk_header_accessor(MailCopiesTo, mailCopiesTo)
kmime_mk_header_accessor(Newsgroups, newsgroups)
kmime_mk_header_accessor(FollowUpTo, followUpTo)

#undef kmime_mk_header_accessor
// @endcond

} // namespace KMime
