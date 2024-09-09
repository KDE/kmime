/*
  kmime_util.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "util.h"
#include "util_p.h"

#include "charfreq_p.h"
#include "headerparsing.h"
#include "kmime_debug.h"
#include "message.h"
#include "warning_p.h"

#include <QCoreApplication>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

using namespace KMime;

namespace KMime
{

QList<QByteArray> c_harsetCache;

QByteArray cachedCharset(const QByteArray &name)
{
    for (const QByteArray &charset : std::as_const(c_harsetCache)) {
        if (qstricmp(name.data(), charset.data()) == 0) {
            return charset;
        }
    }

    c_harsetCache.append(name.toUpper());
    // qCDebug(KMIME_LOG) << "KNMimeBase::cachedCharset() number of cs" << c_harsetCache.count();
    return c_harsetCache.last();
}

bool isUsAscii(QStringView s)
{
    return std::all_of(s.begin(), s.end(), [](QChar c) {
        return c.unicode() < 128;
    });
}

QString nameForEncoding(Headers::contentEncoding enc)
{
    switch (enc) {
    case Headers::CE7Bit:
        return QStringLiteral("7bit");
    case Headers::CE8Bit:
        return QStringLiteral("8bit");
    case Headers::CEquPr:
        return QStringLiteral("quoted-printable");
    case Headers::CEbase64:
        return QStringLiteral("base64");
    case Headers::CEuuenc:
        return QStringLiteral("uuencode");
    case Headers::CEbinary:
        return QStringLiteral("binary");
    default:
        return QStringLiteral("unknown");
    }
}

QList<Headers::contentEncoding> encodingsForData(QByteArrayView data)
{
    QList<Headers::contentEncoding> allowed;
    CharFreq cf(data);

    switch (cf.type()) {
    case CharFreq::SevenBitText:
        allowed << Headers::CE7Bit;
        [[fallthrough]];
    case CharFreq::EightBitText:
        allowed << Headers::CE8Bit;
        [[fallthrough]];
    case CharFreq::SevenBitData:
        if (cf.printableRatio() > 5.0 / 6.0) {
            // let n be the length of data and p the number of printable chars.
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
    0x00,
    0x00,
    0x00,
    0x00,
    0x5F,
    0x35,
    0xFF,
    0xC5,
    0x7F,
    0xFF,
    0xFF,
    0xE3,
    0xFF,
    0xFF,
    0xFF,
    0xFE,
};

// all except tspecials, CTLs, SPACE.
const uchar tTextMap[16] = {
    0x00,
    0x00,
    0x00,
    0x00,
    0x5F,
    0x36,
    0xFF,
    0xC0,
    0x7F,
    0xFF,
    0xFF,
    0xE3,
    0xFF,
    0xFF,
    0xFF,
    0xFE,
};

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
        // qCDebug(KMIME_LOG) << pos;
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

QByteArray CRLFtoLF(const QByteArray &s)
{
    if (!s.contains("\r\n")) {
        return s;
    }

    QByteArray ret = s;
    ret.replace("\r\n", "\n");
    return ret;
}

QByteArray LFtoCRLF(const QByteArray &s)
{
    const auto firstNewline = s.indexOf('\n');
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

bool isCryptoPart(const Content *content)
{
    const auto ct = content->contentType();
    if (!ct || !ct->isMediatype("application")) {
        return false;
    }

    const QByteArray lowerSubType = ct->subType().toLower();
    if (lowerSubType == "pgp-encrypted" || lowerSubType == "pgp-signature" || lowerSubType == "pkcs7-mime" || lowerSubType == "x-pkcs7-mime"
        || lowerSubType == "pkcs7-signature" || lowerSubType == "x-pkcs7-signature") {
        return true;
    }

    if (lowerSubType == "octet-stream") {
        const auto cd = content->contentDisposition();
        if (!cd) {
            return false;
        }
        const auto fileName = cd->filename().toLower();
        return fileName == QLatin1StringView("msg.asc") || fileName == QLatin1StringView("encrypted.asc");
    }

    return false;
}

bool isAttachment(const Content *content)
{
    if (!content) {
        return false;
    }

    const auto contentType = content->contentType();
    // multipart/* is never an attachment itself, message/rfc822 always is
    if (contentType) {
        if (contentType->isMultipart()) {
            return false;
        }
        if (contentType->isMimeType("message/rfc822")) {
            return true;
        }
    }

    // the main body part is not an attachment
    if (content->parent()) {
        const auto top = content->topLevel();
        if (content == top->textContent()) {
            return false;
        }
    }

    // ignore crypto parts
    if (isCryptoPart(content)) {
        return false;
    }

    // content type or content disposition having a file name set looks like an attachment
    const auto contentDisposition = content->contentDisposition();
    if (contentDisposition && !contentDisposition->filename().isEmpty()) {
        return true;
    }

    if (contentType && !contentType->name().isEmpty()) {
        return true;
    }

    // "attachment" content disposition is otherwise a good indicator though
    if (contentDisposition && contentDisposition->disposition() == Headers::CDattachment) {
        return true;
    }

    return false;
}

bool hasAttachment(const Content *content)
{
    if (!content) {
        return false;
    }

    if (isAttachment(content)) {
        return true;
    }

    // OK, content itself is not an attachment. Now we deal with multiparts
    const auto ct = content->contentType();
    if (ct && ct->isMultipart() && !ct->isSubtype("related")) { // && !ct->isSubtype("alternative")) {
        const auto contents = content->contents();
        for (Content *child : contents) {
            if (hasAttachment(child)) {
                return true;
            }
        }
    }
    return false;
}

bool hasInvitation(const Content *content)
{
    if (!content) {
        return false;
    }

    if (isInvitation(content)) {
        return true;
    }

    // OK, content itself is not an invitation. Now we deal with multiparts
    if (auto ct = content->contentType(); ct && ct->isMultipart()) {
        const auto contents = content->contents();
        for (Content *child : contents) {
            if (hasInvitation(child)) {
                return true;
            }
        }
    }
    return false;
}

bool isSigned(const Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (!contentType) {
        return false;
    }
    if (contentType->isSubtype("signed") || contentType->isSubtype("pgp-signature") || contentType->isSubtype("pkcs7-signature")
        || contentType->isSubtype("x-pkcs7-signature") || message->mainBodyPart("multipart/signed") || message->mainBodyPart("application/pgp-signature")
        || message->mainBodyPart("application/pkcs7-signature") || message->mainBodyPart("application/x-pkcs7-signature")) {
        return true;
    }
    return false;
}

bool isEncrypted(const Message *message)
{
    if (!message) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = message->contentType();
    if (!contentType) {
        return false;
    }
    if (contentType->isSubtype("encrypted") || contentType->isSubtype("pgp-encrypted") || contentType->isSubtype("pkcs7-mime")
        || contentType->isSubtype("x-pkcs7-mime") || message->mainBodyPart("multipart/encrypted") || message->mainBodyPart("application/pgp-encrypted")
        || message->mainBodyPart("application/pkcs7-mime") || message->mainBodyPart("application/x-pkcs7-mime")) {
        return true;
    }

    return false;
}

bool isInvitation(const Content *content)
{
    if (!content) {
        return false;
    }

    const KMime::Headers::ContentType *const contentType = content->contentType();
    return contentType && contentType->isMediatype("text") && contentType->isSubtype("calendar");
}

} // namespace KMime
