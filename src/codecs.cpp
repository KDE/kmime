/*
  kmime_codecs.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codecs_p.h"
#include "kmime_debug.h"

#include <KCodecs>

#include <QStringEncoder>

namespace KMime {

static const char reservedCharacters[] = "\"()<>@,.;:\\[]=";

QByteArray encodeRFC2047Sentence(QStringView src, const QByteArray &charset)
{
    QByteArray result;
    const QChar *ch = src.constData();
    const auto length = src.size();
    qsizetype pos = 0;
    qsizetype wordStart = 0;

    //qCDebug(KMIME_LOG) << "Input:" << src;
    // Loop over all characters of the string.
    // When encountering a split character, RFC-2047-encode the word before it, and add it to the result.
    while (pos < length) {
        //qCDebug(KMIME_LOG) << "Pos:" << pos << "Result:" << result << "Char:" << ch->toLatin1();
        const bool isAscii = ch->unicode() < 127;
        const bool isReserved = (strchr(reservedCharacters, ch->toLatin1()) != nullptr);
        if (isAscii && isReserved) {
            const auto wordSize = pos - wordStart;
            if (wordSize > 0) {
                const auto word = src.mid(wordStart, wordSize);
                result += KCodecs::encodeRFC2047String(word, charset);
            }

            result += ch->toLatin1();
            wordStart = pos + 1;
        }
        ch++;
        pos++;
    }

    // Encode the last word
    const auto wordSize = pos - wordStart;
    if (wordSize > 0) {
        const auto word = src.mid(wordStart, pos - wordStart);
        result += KCodecs::encodeRFC2047String(word, charset);
    }

    return result;
}

//-----------------------------------------------------------------------------
QByteArray encodeRFC2231String(QStringView str, const QByteArray &charset)
{
    if (str.isEmpty()) {
      return {};
    }

    QStringEncoder codec(charset.constData());
    QByteArray latin;
    if (charset == "us-ascii") {
        latin = str.toLatin1();
    } else if (codec.isValid()) {
        latin = codec.encode(str);
    } else {
        latin = str.toLocal8Bit();
    }

    char *l;
    for (l = latin.data(); *l; ++l) {
        if (((*l & 0xE0) == 0) || (*l & 0x80)) {
            // *l is control character or 8-bit char
            break;
        }
    }
    if (!*l) {
        return latin;
    }

    QByteArray result = charset + "''";
    for (l = latin.data(); *l; ++l) {
        bool needsQuoting = (*l & 0x80) || (*l == '%');
        if (!needsQuoting) {
            constexpr const char especials[] = "()<>@,;:\"/[]?.= \033";
            for (const auto especial :especials) {
                if (*l == especial) {
                    needsQuoting = true;
                    break;
                }
            }
        }
        if (needsQuoting) {
            result += '%';
            unsigned char hexcode;
            hexcode = ((*l & 0xF0) >> 4) + 48;
            if (hexcode >= 58) {
                hexcode += 7;
            }
            result += hexcode;
            hexcode = (*l & 0x0F) + 48;
            if (hexcode >= 58) {
                hexcode += 7;
            }
            result += hexcode;
        } else {
            result += *l;
        }
    }
    return result;
}

}
