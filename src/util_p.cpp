/*
  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <config-kmime.h>

#include "kmime_debug.h"
#include "util_p.h"

#include <QByteArray>
#include <QChar>
#include <QString>

#include <cctype>

using namespace KMime;

qsizetype KMime::findHeaderLineEnd(QByteArrayView src, qsizetype &dataBegin, bool *folded)
{
    auto end = dataBegin;
    auto len = src.length() - 1;

    if (folded) {
        *folded = false;
    }

    if (dataBegin < 0) {
        // Not found
        return -1;
    }

    if (dataBegin > len) {
        // No data available
        return len + 1;
    }

    // If the first line contains nothing, but the next line starts with a space
    // or a tab, that means a stupid mail client has made the first header field line
    // entirely empty, and has folded the rest to the next line(s).
    if (src.at(end) == '\n' && end + 1 < len && (src[end + 1] == ' ' || src[end + 1] == '\t')) {
        // Skip \n and first whitespace
        dataBegin += 2;
        end += 2;
    }

    if (src.at(end) != '\n') { // check if the header is not empty
        while (true) {
            end = src.indexOf('\n', end + 1);
            if (end == -1 || end == len) {
                // end of string
                break;
            } else if (src[end + 1] == ' ' || src[end + 1] == '\t'
                       || (src[end + 1] == '=' && end + 3 <= len
                           && ((src[end + 2] == '0' && src[end + 3] == '9') || (src[end + 2] == '2' && src[end + 3] == '0')))) {
                // next line is header continuation or starts with =09/=20 (bug #86302)
                if (folded) {
                    *folded = true;
                }
            } else {
                // end of header (no header continuation)
                break;
            }
        }
    }

    if (end < 0) {
        end = len + 1; // take the rest of the string
    }
    return end;
}

#if !HAVE_STRCASESTR
#ifdef WIN32
#define strncasecmp _strnicmp
#endif
static const char *strcasestr(const char *haystack, const char *needle)
{
    /* Copied from libreplace as part of qtwebengine 5.5.1 */
    const char *s;
    size_t nlen = strlen(needle);
    for (s = haystack; *s; s++) {
        if (toupper(*needle) == toupper(*s) && strncasecmp(s, needle, nlen) == 0) {
            return (char *)((uintptr_t)s);
        }
    }
    return NULL;
}
#endif

qsizetype KMime::indexOfHeader(const QByteArray &src, const QByteArray &name, qsizetype &end, qsizetype &dataBegin, bool *folded)
{
    QByteArray n = name;
    n.append(':');
    qsizetype begin = -1;

    if (qstrnicmp(n.constData(), src.constData(), n.length()) == 0) {
        begin = 0;
    } else {
        n.prepend('\n');
        const char *p = strcasestr(src.constData(), n.constData());
        if (!p) {
            begin = -1;
        } else {
            begin = p - src.constData();
            ++begin;
        }
    }

    if (begin > -1) { // there is a header with the given name
        dataBegin = begin + name.length() + 1; // skip the name
        // skip the usual space after the colon
        if (dataBegin < src.length() && src.at(dataBegin) == ' ') {
            ++dataBegin;
        }
        end = findHeaderLineEnd(src, dataBegin, folded);
        return begin;

    } else {
        end = -1;
        dataBegin = -1;
        return -1; // header not found
    }
}

QByteArray KMime::extractHeader(const QByteArray &src, const QByteArray &name)
{
    qsizetype begin;
    qsizetype end;
    bool folded;
    QByteArray result;

    if (src.isEmpty() || indexOfHeader(src, name, end, begin, &folded) < 0) {
        return result;
    }

    if (begin >= 0) {
        if (!folded) {
            result = src.mid(begin, end - begin);
        } else {
            if (end > begin) {
                result = unfoldHeader(src.constData() + begin, end - begin);
            }
        }
    }
    return result;
}

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
            } else if (foldEnd && *(foldEnd - 1) == '\n' && *foldEnd == '=' && foldEnd + 2 < (header + headerSize - 1)
                       && ((*(foldEnd + 1) == '0' && *(foldEnd + 2) == '9') || (*(foldEnd + 1) == '2' && *(foldEnd + 2) == '0'))) {
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

namespace
{
// state machine used by foldHeader()
struct HeaderContext {
    unsigned int isEscapePair : 1;
    unsigned int isQuotedStr : 1;

    HeaderContext()
    {
        isEscapePair = isQuotedStr = 0;
    }

    void push(char c)
    {
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
    auto pos = header.indexOf(':') + 1;
    if (pos <= 0 || pos >= header.length()) {
        return header;
    }

    // prepare for mutating header
    QByteArray hdr = header;

    // There are positions that are eligible for inserting FWS but discouraged
    // (e.g. existing white space within a quoted string), and there are
    // positions which are recommended for inserting FWS (e.g. after comma
    // separator of an address list).
    auto eligible = pos;
    auto recommended = pos;

    // reflects start position of "current line" in byte array
    qsizetype start = 0;

    HeaderContext ctx;

    for (; true; ++pos) {
        if (pos - start > maxLen && eligible) {
            // Fold line preferably at recommended position, at eligible position
            // otherwise.
            const auto fws = recommended ? recommended : eligible;
            hdr.insert(fws, '\n');
            // We started a new line, so reset.
            if (eligible <= fws) {
                eligible = 0;
            } else {
                ++eligible; // LF
            }
            recommended = 0;
            start = fws + 1 /* LF */;
            continue;
        }

        if (pos >= hdr.length()) {
            break;
        }

        // account for already inserted FWS
        // (NOTE: we are not caring about broken ones here)
        if (hdr[pos] == '\n') {
            recommended = eligible = 0;
            start = pos + 1 /* LF */;
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

namespace
{
template<typename StringType, typename CharType>
void removeQuotesGeneric(StringType &str)
{
    bool inQuote = false;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == CharType('"')) {
            str.remove(i, 1);
            i--;
            inQuote = !inQuote;
        } else {
            if (inQuote && (str[i] == CharType('\\'))) {
                str.remove(i, 1);
            }
        }
    }
}
}

void KMime::removeQuotes(QByteArray &str)
{
    removeQuotesGeneric<QByteArray, char>(str);
}

void KMime::removeQuotes(QString &str)
{
    removeQuotesGeneric<QString, QLatin1Char>(str);
}

namespace
{
template<class StringType, class CharConverterType>
void addQuotes_impl(StringType &str, bool forceQuotes)
{
    constexpr const char reservedCharacters[] = R"(""(),.:;<=>@[\])"; // sorted!

    bool needsQuotes = false;
    for (qsizetype i = 0; i < str.length(); i++) {
        const auto cur = str.at(i);
        const auto it = std::lower_bound(std::begin(reservedCharacters), std::end(reservedCharacters), cur, [](char lhs, auto rhs) {
            return CharConverterType(lhs) < rhs;
        });
        if (it != std::end(reservedCharacters) && CharConverterType(*it) == cur) {
            needsQuotes = true;
        }
        if (cur == CharConverterType('\\') || cur == CharConverterType('\"')) {
            str.insert(i, CharConverterType('\\'));
            i++;
        }
    }

    if (needsQuotes || forceQuotes) {
        str.insert(0, CharConverterType('\"'));
        str.append(CharConverterType('\"'));
    }
}
}

void KMime::addQuotes(QByteArray &str, bool forceQuotes)
{
    addQuotes_impl<QByteArray, char>(str, forceQuotes);
}

void KMime::addQuotes(QString &str, bool forceQuotes)
{
    addQuotes_impl<QString, QLatin1Char>(str, forceQuotes);
}

QString KMime::balanceBidiState(const QString &input)
{
    const int LRO = 0x202D;
    const int RLO = 0x202E;
    const int LRE = 0x202A;
    const int RLE = 0x202B;
    const int PDF = 0x202C;

    QString result = input;

    int openDirChangers = 0;
    int numPDFsRemoved = 0;
    for (int i = 0; i < input.length(); i++) {
        const ushort &code = input.at(i).unicode();
        if (code == LRO || code == RLO || code == LRE || code == RLE) {
            openDirChangers++;
        } else if (code == PDF) {
            if (openDirChangers > 0) {
                openDirChangers--;
            } else {
                // One PDF too much, remove it
                qCWarning(KMIME_LOG) << "Possible Unicode spoofing (unexpected PDF) detected in" << input;
                result.remove(i - numPDFsRemoved, 1);
                numPDFsRemoved++;
            }
        }
    }

    if (openDirChangers > 0) {
        qCWarning(KMIME_LOG) << "Possible Unicode spoofing detected in" << input;

        // At PDF chars to the end until the correct state is restored.
        // As a special exception, when encountering quoted strings, place the PDF before
        // the last quote.
        for (int i = openDirChangers; i > 0; i--) {
            if (result.endsWith(QLatin1Char('"'))) {
                result.insert(result.length() - 1, QChar(PDF));
            } else {
                result += QChar(PDF);
            }
        }
    }

    return result;
}

QString KMime::removeBidiControlChars(const QString &input)
{
    const int LRO = 0x202D;
    const int RLO = 0x202E;
    const int LRE = 0x202A;
    const int RLE = 0x202B;
    QString result = input;
    result.remove(QChar(LRO));
    result.remove(QChar(RLO));
    result.remove(QChar(LRE));
    result.remove(QChar(RLE));
    return result;
}
