/*
  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_util_p.h"

#include <QByteArray>
#include <QChar>

#include <cctype>
#include <cstdlib>

using namespace KMime;

QByteArray KMime::unfoldHeader(const char *header, size_t headerSize)
{
    QByteArray result;
    if (headerSize == 0) {
        return result;
    }

    // unfolding skips characters so result will be at worst headerSize long
    result.reserve(headerSize);

    const char *end = header + headerSize;
    const char *pos = header;
    const char *foldBegin = nullptr;
    const char *foldMid = nullptr;
    const char *foldEnd = nullptr;
    while ((foldMid = strchr(pos, '\n')) && foldMid < end) {
        foldBegin = foldEnd = foldMid;
        // find the first space before the line-break
        while (foldBegin > header) {
            if (!QChar::isSpace(*(foldBegin - 1))) {
                break;
            }
            --foldBegin;
        }
        // find the first non-space after the line-break
        while (foldEnd <= end - 1) {
            if (QChar::isSpace(*foldEnd)) {
                ++foldEnd;
            } else if (foldEnd && *(foldEnd - 1) == '\n' &&
                       *foldEnd == '=' && foldEnd + 2 < (header + headerSize - 1) &&
                       ((*(foldEnd + 1) == '0' &&
                         *(foldEnd + 2) == '9') ||
                        (*(foldEnd + 1) == '2' &&
                         *(foldEnd + 2) == '0'))) {
                // bug #86302: malformed header continuation starting with =09/=20
                foldEnd += 3;
            } else {
                break;
            }
        }

        result.append(pos, foldBegin - pos);
        if (foldBegin != pos && foldEnd < end - 1) {
            result += ' ';
        }
        pos = foldEnd;
    }
    if (end > pos) {
        result.append(pos, end - pos);
    }
    return result;
}

QByteArray KMime::unfoldHeader(const QByteArray &header)
{
    return unfoldHeader(header.constData(), header.size());
}

namespace {
// state machine used by foldHeader()
struct HeaderContext {
    unsigned int isEscapePair : 1;
    unsigned int isQuotedStr : 1;

    HeaderContext() {
        isEscapePair = isQuotedStr = 0;
    }

    void push(char c) {
        if (c == '\"' && !isEscapePair) {
            ++isQuotedStr;
        } else if (c == '\\' || isEscapePair) {
            ++isEscapePair;
        }
    }
};
}

QByteArray KMime::foldHeader(const QByteArray &header)
{
    // RFC 5322 section 2.1.1. "Line Length Limits" says:
    //
    // "Each line of characters MUST be no more than 998 characters, and
    //  SHOULD be no more than 78 characters, excluding the CRLF."
    const int maxLen = 78;

    if (header.length() <= maxLen) {
        return header;
    }

    // fast forward to header body
    int pos = header.indexOf(':') + 1;
    if (pos <= 0 || pos >= header.length()) {
        return header;
    }

    // prepare for mutating header
    QByteArray hdr = header;

    // There are positions that are eligible for inserting FWS but discouraged
    // (e.g. existing white space within a quoted string), and there are
    // positions which are recommended for inserting FWS (e.g. after comma
    // separator of an address list).
    int eligible = pos;
    int recommended = pos;

    // reflects start position of "current line" in byte array
    int start = 0;

    HeaderContext ctx;

    for (; true; ++pos) {
        if (pos - start > maxLen && eligible) {
            // Fold line preferably at recommended position, at eligible position
            // otherwise.
            const int fws = recommended ? recommended : eligible;
            hdr.insert(fws, '\n');
            // We started a new line, so reset.
            if (eligible <= fws) {
                eligible = 0;
            } else {
                ++eligible; // LF
            }
            recommended = 0;
            start = fws + 1/* LF */;
            continue;
        }

        if (pos >= hdr.length()) {
            break;
        }

        // account for already inserted FWS
        // (NOTE: we are not caring about broken ones here)
        if (hdr[pos] == '\n') {
            recommended = eligible = 0;
            start = pos + 1/* LF */;
        }

        // Any white space character position is eligible for folding, except of
        // escape pair (i.e. BSP WSP must not be folded).
        if (hdr[pos] == ' ' && !ctx.isEscapePair && hdr[pos - 1] != '\n') {
            eligible = pos;
            if ((hdr[pos - 1] == ',' || hdr[pos - 1] == ';') && !ctx.isQuotedStr) {
                recommended = pos;
            }
        }

        ctx.push(hdr[pos]);
    }

    return hdr;
}
