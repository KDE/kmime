/*
  kmime_util.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_util.h"
#include "kmime_util_p.h"

#include "kmime_charfreq.h"
#include "kmime_debug.h"
#include "kmime_header_parsing.h"
#include "kmime_message.h"
#include "kmime_warning.h"

#include <config-kmime.h>
// #include <kdefakes.h> // for strcasestr

#include <KLocalizedString>
#include <KCharsets>
#include <QCoreApplication>

#include <QTextCodec>

#include <ctype.h>
#include <time.h>
#include <stdlib.h>

using namespace KMime;

namespace KMime
{

QVector<QByteArray> c_harsetCache;
bool u_seOutlookEncoding = false;

QByteArray cachedCharset(const QByteArray &name)
{
    for (const QByteArray &charset : qAsConst(c_harsetCache)) {
        if (qstricmp(name.data(), charset.data()) == 0) {
            return charset;
        }
    }

    c_harsetCache.append(name.toUpper());
    //qCDebug(KMIME_LOG) << "KNMimeBase::cachedCharset() number of cs" << c_harsetCache.count();
    return c_harsetCache.last();
}

bool isUsAscii(const QString &s)
{
    const uint sLength = s.length();
    for (uint i = 0; i < sLength; i++) {
        if (s.at(i).toLatin1() <= 0) {     // c==0: non-latin1, c<0: non-us-ascii
            return false;
        }
    }
    return true;
}

QString nameForEncoding(Headers::contentEncoding enc)
{
    switch (enc) {
    case Headers::CE7Bit: return QStringLiteral("7bit");
    case Headers::CE8Bit: return QStringLiteral("8bit");
    case Headers::CEquPr: return QStringLiteral("quoted-printable");
    case Headers::CEbase64: return QStringLiteral("base64");
    case Headers::CEuuenc: return QStringLiteral("uuencode");
    case Headers::CEbinary: return QStringLiteral("binary");
    default: return QStringLiteral("unknown");
    }
}

QVector<Headers::contentEncoding> encodingsForData(const QByteArray &data)
{
    QVector<Headers::contentEncoding> allowed;
    CharFreq cf(data);

    switch (cf.type()) {
    case CharFreq::SevenBitText:
        allowed << Headers::CE7Bit;
        Q_FALLTHROUGH();
    case CharFreq::EightBitText:
        allowed << Headers::CE8Bit;
        Q_FALLTHROUGH();
    case CharFreq::SevenBitData:
        if (cf.printableRatio() > 5.0 / 6.0) {
            // let n the length of data and p the number of printable chars.
            // Then base64 \approx 4n/3; qp \approx p + 3(n-p)
            // => qp < base64 iff p > 5n/6.
            allowed << Headers::CEquPr;
            allowed << Headers::CEbase64;
        } else {
            allowed << Headers::CEbase64;
            allowed << Headers::CEquPr;
        }
        break;
    case CharFreq::EightBitData:
        allowed << Headers::CEbase64;
        break;
    case CharFreq::None:
    default:
        Q_ASSERT(false);
    }

    return allowed;
}

// all except specials, CTLs, SPACE.
const uchar aTextMap[16] = {
    0x00, 0x00, 0x00, 0x00,
    0x5F, 0x35, 0xFF, 0xC5,
    0x7F, 0xFF, 0xFF, 0xE3,
    0xFF, 0xFF, 0xFF, 0xFE
};

// all except tspecials, CTLs, SPACE.
const uchar tTextMap[16] = {
    0x00, 0x00, 0x00, 0x00,
    0x5F, 0x36, 0xFF, 0xC0,
    0x7F, 0xFF, 0xFF, 0xE3,
    0xFF, 0xFF, 0xFF, 0xFE
};

void setUseOutlookAttachmentEncoding(bool violateStandard)
{
    u_seOutlookEncoding = violateStandard;
}

bool useOutlookAttachmentEncoding()
{
    return u_seOutlookEncoding;
}

QByteArray uniqueString()
{
    static const char chars[] = "0123456789abcdefghijklmnopqrstuvxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    time_t now;
    char p[11];
    int ran;
    unsigned int timeval;

    p[10] = '\0';
    now = time(nullptr);
    ran = 1 + (int)(1000.0 * rand() / (RAND_MAX + 1.0));
    timeval = (now / ran) + QCoreApplication::applicationPid();

    for (int i = 0; i < 10; i++) {
        int pos = (int)(61.0 * rand() / (RAND_MAX + 1.0));
        //qCDebug(KMIME_LOG) << pos;
        p[i] = chars[pos];
    }

    QByteArray ret;
    ret.setNum(timeval);
    ret += '.';
    ret += p;

    return ret;
}

QByteArray multiPartBoundary()
{
    return "nextPart" + uniqueString();
}

QByteArray unfoldHeader(const char *header, size_t headerSize)
{
    QByteArray result;
    if (headerSize == 0) {
        return result;
    }

    // unfolding skips characters so result will be at worst headerSize long
    result.reserve(headerSize);

    const char *end = header + headerSize;
    const char *pos = header, *foldBegin = nullptr, *foldMid = nullptr, *foldEnd = nullptr;
    while ((foldMid = strchr(pos, '\n')) && foldMid < end) {
        foldBegin = foldEnd = foldMid;
        // find the first space before the line-break
        while (foldBegin) {
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
        if (foldEnd < end - 1) {
            result += ' ';
        }
        pos = foldEnd;
    }
    if (end > pos) {
        result.append(pos, end - pos);
    }
    return result;
}

QByteArray unfoldHeader(const QByteArray &header)
{
    return unfoldHeader(header.constData(), header.size());
}

int findHeaderLineEnd(const QByteArray &src, int &dataBegin, bool *folded)
{
    int end = dataBegin;
    int len = src.length() - 1;

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
    if (src.at(end) == '\n' && end + 1 < len &&
            (src[end + 1] == ' ' || src[end + 1] == '\t')) {

        // Skip \n and first whitespace
        dataBegin += 2;
        end += 2;
    }

    if (src.at(end) != '\n') {      // check if the header is not empty
        while (true) {
            end = src.indexOf('\n', end + 1);
            if (end == -1 || end == len) {
                // end of string
                break;
            } else if (src[end + 1] == ' ' || src[end + 1] == '\t' ||
                       (src[end + 1] == '=' && end + 3 <= len &&
                        ((src[end + 2] == '0' && src[end + 3] == '9') ||
                         (src[end + 2] == '2' && src[end + 3] == '0')))) {
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
        end = len + 1; //take the rest of the string
    }
    return end;
}

#ifndef HAVE_STRCASESTR
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

int indexOfHeader(const QByteArray &src, const QByteArray &name, int &end, int &dataBegin, bool *folded)
{
    QByteArray n = name;
    n.append(':');
    int begin = -1;

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

    if (begin > -1) {       //there is a header with the given name
        dataBegin = begin + name.length() + 1; //skip the name
        // skip the usual space after the colon
        if (dataBegin < src.length() && src.at(dataBegin) == ' ') {
            ++dataBegin;
        }
        end = findHeaderLineEnd(src, dataBegin, folded);
        return begin;

    } else {
        end = -1;
        dataBegin = -1;
        return -1; //header not found
    }
}

QByteArray extractHeader(const QByteArray &src, const QByteArray &name)
{
    int begin, end;
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

QByteArray CRLFtoLF(const QByteArray &s)
{
    if (!s.contains("\r\n")) {
        return s;
    }

    QByteArray ret = s;
    ret.replace("\r\n", "\n");
    return ret;
}

QByteArray CRLFtoLF(const char *s)
{
    QByteArray ret = s;
    return CRLFtoLF(ret);
}

QByteArray LFtoCRLF(const QByteArray &s)
{
    const int firstNewline = s.indexOf('\n');
    if (firstNewline == -1) {
        return s;
    }
    if (firstNewline > 0 && s.at(firstNewline - 1) == '\r') {
        // We found \r\n already, don't change anything
        // This check assumes that input is consistent in terms of newlines,
        // but so did if (s.contains("\r\n")), too.
        return s;
    }

    QByteArray ret = s;
    ret.replace('\n', "\r\n");
    return ret;
}

QByteArray LFtoCRLF(const char *s)
{
    QByteArray ret = s;
    return LFtoCRLF(ret);
}

QByteArray CRtoLF(const QByteArray &s)
{
    const int firstNewline = s.indexOf('\r');
    if (firstNewline == -1) {
        return s;
    }
    if (firstNewline > 0 && (s.length() > firstNewline + 1) && s.at(firstNewline + 1) == '\n') {
        // We found \r\n already, don't change anything
        // This check assumes that input is consistent in terms of newlines,
        // but so did if (s.contains("\r\n")), too.
        return s;
    }

    QByteArray ret = s;
    ret.replace('\r', '\n');
    return ret;
}

QByteArray CRtoLF(const char *s)
{
    const QByteArray ret = s;
    return CRtoLF(ret);
}

namespace
{
template < typename StringType, typename CharType > void removeQuotesGeneric(StringType &str)
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

void removeQuotes(QByteArray &str)
{
    removeQuotesGeneric<QByteArray, char>(str);
}

void removeQuotes(QString &str)
{
    removeQuotesGeneric<QString, QLatin1Char>(str);
}

template<class StringType, class CharType, class CharConverterType, class StringConverterType, class ToString>
void addQuotes_impl(StringType &str, bool forceQuotes)
{
    bool needsQuotes = false;
    for (int i = 0; i < str.length(); i++) {
        const CharType cur = str.at(i);
        if (QString(ToString(str)).contains(QRegExp(QStringLiteral("\"|\\\\|=|\\]|\\[|:|;|,|\\.|,|@|<|>|\\)|\\(")))) {
            needsQuotes = true;
        }
        if (cur == CharConverterType('\\') || cur == CharConverterType('\"')) {
            str.insert(i, CharConverterType('\\'));
            i++;
        }
    }

    if (needsQuotes || forceQuotes) {
        str.insert(0, CharConverterType('\"'));
        str.append(StringConverterType("\""));
    }
}

void addQuotes(QByteArray &str, bool forceQuotes)
{
    addQuotes_impl<QByteArray, char, char, char *, QLatin1String>(str, forceQuotes);
}

void addQuotes(QString &str, bool forceQuotes)
{
    addQuotes_impl<QString, QChar, QLatin1Char, QLatin1String, QString>(str, forceQuotes);
}

KMIME_EXPORT QString balanceBidiState(const QString &input)
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

QString removeBidiControlChars(const QString &input)
{
    const int LRO = 0x202D;
    const int RLO = 0x202E;
    const int LRE = 0x202A;
    const int RLE = 0x202B;
    QString result = input;
    result.remove(LRO);
    result.remove(RLO);
    result.remove(LRE);
    result.remove(RLE);
    return result;
}

bool isCryptoPart(Content *content)
{
    auto ct = content->contentType(false);
    if (!ct || !ct->isMediatype("application")) {
        return false;
    }

    const QByteArray lowerSubType = ct->subType().toLower();
    if (lowerSubType == "pgp-encrypted" ||
        lowerSubType == "pgp-signature" ||
        lowerSubType == "pkcs7-mime" ||
        lowerSubType == "x-pkcs7-mime" ||
        lowerSubType == "pkcs7-signature" ||
        lowerSubType == "x-pkcs7-signature") {
        return true;
    }

    if (lowerSubType == "octet-stream") {
        auto cd = content->contentDisposition(false);
        if (!cd)
            return false;
        const auto fileName = cd->filename().toLower();
        return fileName == QLatin1String("msg.asc") || fileName == QLatin1String("encrypted.asc");
    }

    return false;
}

bool isAttachment(Content* content)
{
    if (!content) {
        return false;
    }

    const auto contentType = content->contentType(false);
    // multipart/* is never an attachment itself, message/rfc822 always is
    if (contentType) {
        if (contentType->isMultipart())
            return false;
        if (contentType->isMimeType("message/rfc822"))
            return true;
    }

    // the main body part is not an attachment
    if (content->parent()) {
        const auto top = content->topLevel();
        if (content == top->textContent())
            return false;
    }

    // ignore crypto parts
    if (isCryptoPart(content))
        return false;


    // content type or content disposition having a file name set looks like an attachment
    const auto contentDisposition = content->contentDisposition(false);
    if (contentDisposition && !contentDisposition->filename().isEmpty()) {
        return true;
    }

    if (contentType && !contentType->name().isEmpty()) {
        return true;
    }

    // "attachment" content disposition is otherwise a good indicator though
    if (contentDisposition && contentDisposition->disposition() == Headers::CDattachment)
        return true;

    return false;
}

bool hasAttachment(Content *content)
{
    if (!content)
        return false;

    if (isAttachment(content))
        return true;

    // Ok, content itself is not an attachment. now we deal with multiparts
    auto ct = content->contentType(false);
    if (ct && ct->isMultipart() && !ct->isSubtype("related")) {// && !ct->isSubtype("alternative")) {
        const auto contents = content->contents();
        for (Content *child : contents) {
            if (hasAttachment(child)) {
                return true;
            }
        }
    }
    return false;
}

bool hasInvitation(Content *content)
{
    if (!content) {
        return false;
    }

    if (isInvitation(content)) {
        return true;
    }

    // Ok, content itself is not an invitation. now we deal with multiparts
    if (content->contentType()->isMultipart()) {
        const auto contents = content->contents();
        for (Content *child : contents) {
            if (hasInvitation(child)) {
                return true;
            }
        }
    }
    return false;
}

bool isSigned(Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (contentType->isSubtype("signed") ||
            contentType->isSubtype("pgp-signature") ||
            contentType->isSubtype("pkcs7-signature") ||
            contentType->isSubtype("x-pkcs7-signature") ||
            message->mainBodyPart("multipart/signed") ||
            message->mainBodyPart("application/pgp-signature") ||
            message->mainBodyPart("application/pkcs7-signature") ||
            message->mainBodyPart("application/x-pkcs7-signature")) {
        return true;
    }
    return false;
}

bool isEncrypted(Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (contentType->isSubtype("encrypted") ||
            contentType->isSubtype("pgp-encrypted") ||
            contentType->isSubtype("pkcs7-mime") ||
            contentType->isSubtype("x-pkcs7-mime") ||
            message->mainBodyPart("multipart/encrypted") ||
            message->mainBodyPart("application/pgp-encrypted") ||
            message->mainBodyPart("application/pkcs7-mime") ||
            message->mainBodyPart("application/x-pkcs7-mime")) {
        return true;
    }

    return false;
}

bool isInvitation(Content *content)
{
    if (!content) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = content->contentType(false);

    if (contentType && contentType->isMediatype("text") && contentType->isSubtype("calendar")) {
        return true;
    }

    return false;
}

} // namespace KMime
