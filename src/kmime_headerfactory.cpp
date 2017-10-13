/*
    kmime_header_factory.cpp

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

using namespace KMime;
using namespace KMime::Headers;

#define mk_header(hdr) \
    if (qstrnicmp(type, hdr ::staticType(), typeLen) == 0) \
        return new hdr

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
