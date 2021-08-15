/*
    kmime_header_factory.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**
  @file
  This file is part of the API for handling MIME data and
  defines the HeaderFactory class.

  @brief
  Defines the HeaderFactory class.

  @authors Constantin Berzan \<exit3219@gmail.com\>
*/

#include "kmime_headerfactory_p.h"
#include "kmime_headers.h"

#include <algorithm>

using namespace KMime;
using namespace KMime::Headers;

#define mk_header(hdr) \
    if (qstrnicmp(type, hdr ::staticType(), std::max(typeLen, strlen(hdr::staticType()))) == 0) \
        return new hdr;

Headers::Base *HeaderFactory::createHeader(const char *type, size_t typeLen)
{
    Q_ASSERT(type && *type);
    switch (*type) {
        case 'b':
        case 'B':
            mk_header(Bcc);
            break;
        case 'c':
        case 'C':
            mk_header(Cc);
            mk_header(ContentDescription);
            mk_header(ContentDisposition);
            mk_header(ContentID);
            mk_header(ContentLocation);
            mk_header(ContentTransferEncoding);
            mk_header(ContentType);
            mk_header(Control);
            break;
        case 'd':
        case 'D':
            mk_header(Date);
            break;
        case 'f':
        case 'F':
            mk_header(FollowUpTo);
            mk_header(From);
            break;
        case 'i':
        case 'I':
            mk_header(InReplyTo);
            break;
        case 'k':
        case 'K':
            mk_header(Keywords);
            break;
        case 'l':
        case 'L':
            mk_header(Lines);
            break;
        case 'm':
        case 'M':
            mk_header(MailCopiesTo);
            mk_header(MessageID);
            mk_header(MIMEVersion);
            break;
        case 'n':
        case 'N':
            mk_header(Newsgroups);
            break;
        case 'o':
        case 'O':
            mk_header(Organization);
            break;
        case 'r':
        case 'R':
            mk_header(References);
            mk_header(ReplyTo);
            mk_header(ReturnPath);
            break;
        case 's':
        case 'S':
            mk_header(Sender);
            mk_header(Subject);
            mk_header(Supersedes);
            break;
        case 't':
        case 'T':
            mk_header(To);
            break;
        case 'u':
        case 'U':
            mk_header(UserAgent);
            break;
    }
    return nullptr;
}

#undef mk_header
