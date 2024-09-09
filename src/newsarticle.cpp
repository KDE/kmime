/*
    kmime_newsarticle.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "newsarticle.h"
#include "util_p.h"

using namespace KMime;

namespace KMime
{

NewsArticle::NewsArticle() = default;
NewsArticle::~NewsArticle() = default;

QByteArray NewsArticle::assembleHeaders()
{
    // Create the mandatory Lines: field.
    lines(true);

    // Assemble all header fields.
    return Message::assembleHeaders();
}

// @cond PRIVATE
#define kmime_mk_header_accessor(type, method)                                                                                                                 \
    Headers::type *NewsArticle::method(bool create)                                                                                                            \
    {                                                                                                                                                          \
        return header<Headers::type>(create);                                                                                                                  \
    }                                                                                                                                                          \
    const Headers::type *NewsArticle::method() const                                                                                                           \
    {                                                                                                                                                          \
        return header<Headers::type>();                                                                                                                        \
    }

// clang-format off
kmime_mk_header_accessor(Control, control)
kmime_mk_header_accessor(Lines, lines)
kmime_mk_header_accessor(Supersedes, supersedes)
kmime_mk_header_accessor(MailCopiesTo, mailCopiesTo)
kmime_mk_header_accessor(Newsgroups, newsgroups)
kmime_mk_header_accessor(FollowUpTo, followUpTo)
// clang-format on

#undef kmime_mk_header_accessor
// @endcond

} // namespace KMime
