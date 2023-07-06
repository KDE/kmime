/*
  kmime_codecs.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001 the KMime authors.
  See file AUTHORS for details

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmime_codecs.h"
#include "kmime_debug.h"

#include <QStringDecoder>
#include <QStringEncoder>

namespace KMime {

static const char reservedCharacters[] = "\"()<>@,.;:\\[]=";

QByteArray encodeRFC2047String(const QString &src, const QByteArray &charset,
                               bool addressHeader, bool allow8BitHeaders)
{
    QByteArray result;
    int start = 0;
    int end = 0;
    bool nonAscii = false;
    bool useQEncoding = false;

    // fromLatin1() is safe here, codecForName() uses toLatin1() internally
    QStringEncoder codec(charset.constData());

    QByteArray usedCS;
    if (!codec.isValid()) {
        //no codec available => try local8Bit and hope the best ;-)
        codec = QStringEncoder(QStringEncoder::System);
        usedCS = codec.name();
    } else {
        if (charset.isEmpty()) {
            usedCS = codec.name();
        } else {
            usedCS = charset;
        }
    }

    QByteArray encoded8Bit = codec.encode(src);
    if (codec.hasError()) {
        usedCS = "utf-8";
        codec = QStringEncoder(usedCS.constData());
        encoded8Bit = codec.encode(src);
    }

    if (usedCS.contains("8859-")) {     // use "B"-Encoding for non iso-8859-x charsets
        useQEncoding = true;
    }

    if (allow8BitHeaders) {
        return encoded8Bit;
    }

    int encoded8BitLength = encoded8Bit.length();
    for (int i = 0; i < encoded8BitLength; i++) {
        if (encoded8Bit[i] == ' ') {   // encoding starts at word boundaries
            start = i + 1;
        }

        // encode escape character, for japanese encodings...
        if (((signed char)encoded8Bit[i] < 0) || (encoded8Bit[i] == '\033') ||
                (addressHeader && (strchr("\"()<>@,.;:\\[]=", encoded8Bit[i]) != nullptr))) {
            end = start;   // non us-ascii char found, now we determine where to stop encoding
            nonAscii = true;
            break;
        }
    }

    if (nonAscii) {
        while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
            // we encode complete words
            end++;
        }

        for (int x = end; x < encoded8Bit.length(); x++) {
            if (((signed char)encoded8Bit[x] < 0) || (encoded8Bit[x] == '\033') ||
                    (addressHeader && (strchr(reservedCharacters, encoded8Bit[x]) != nullptr))) {
                end = x;     // we found another non-ascii word

                while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
                    // we encode complete words
                    end++;
                }
            }
        }

        result = encoded8Bit.left(start) + "=?" + usedCS;

        if (useQEncoding) {
            result += "?Q?";

            char hexcode; // "Q"-encoding implementation described in RFC 2047
            for (int i = start; i < end; i++) {
                char c = encoded8Bit[i];
                if (c == ' ') {   // make the result readable with not MIME-capable readers
                    result += '_';
                } else {
                    if (((c >= 'a') && (c <= 'z')) ||        // paranoid mode, encode *all* special chars to avoid problems
                            ((c >= 'A') && (c <= 'Z')) ||        // with "From" & "To" headers
                            ((c >= '0') && (c <= '9'))) {
                        result += c;
                    } else {
                        result += '=';                 // "stolen" from KMail ;-)
                        hexcode = ((c & 0xF0) >> 4) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                        hexcode = (c & 0x0F) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                    }
                }
            }
        } else {
            result += "?B?" + encoded8Bit.mid(start, end - start).toBase64();
        }

        result += "?=";
        result += encoded8Bit.right(encoded8Bit.length() - end);
    } else {
        result = encoded8Bit;
    }

    return result;
}

QByteArray encodeRFC2047Sentence(const QString &src, const QByteArray &charset)
{
    QByteArray result;
    const QChar *ch = src.constData();
    const int length = src.length();
    int pos = 0;
    int wordStart = 0;

    //qCDebug(KMIME_LOG) << "Input:" << src;
    // Loop over all characters of the string.
    // When encountering a split character, RFC-2047-encode the word before it, and add it to the result.
    while (pos < length) {
        //qCDebug(KMIME_LOG) << "Pos:" << pos << "Result:" << result << "Char:" << ch->toLatin1();
        const bool isAscii = ch->unicode() < 127;
        const bool isReserved = (strchr(reservedCharacters, ch->toLatin1()) != nullptr);
        if (isAscii && isReserved) {
            const int wordSize = pos - wordStart;
            if (wordSize > 0) {
                const QString word = src.mid(wordStart, wordSize);
                result += encodeRFC2047String(word, charset);
            }

            result += ch->toLatin1();
            wordStart = pos + 1;
        }
        ch++;
        pos++;
    }

    // Encode the last word
    const int wordSize = pos - wordStart;
    if (wordSize > 0) {
        const QString word = src.mid(wordStart, pos - wordStart);
        result += encodeRFC2047String(word, charset);
    }

    return result;
}

//-----------------------------------------------------------------------------
QByteArray encodeRFC2231String(const QString &str, const QByteArray &charset)
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
            const QByteArray especials = "()<>@,;:\"/[]?.= \033";
            int len = especials.length();
            for (int i = 0; i < len; i++) {
                if (*l == especials[i]) {
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

//-----------------------------------------------------------------------------
QString decodeRFC2231String(const QByteArray &str, QByteArray &usedCS, const QByteArray &defaultCS,
                            bool forceCS)
{
    int p = str.indexOf('\'');
    if (p < 0) {
        QStringDecoder codec(defaultCS.constData());
        if (!codec.isValid()) {
            codec = QStringDecoder(QStringDecoder::System);
        }
        return codec.decode(str);
    }

    QByteArray charset = str.left(p);

    QByteArray st = str.mid(str.lastIndexOf('\'') + 1);

    char ch;
    char ch2;
    p = 0;
    while (p < st.length()) {
        if (st.at(p) == 37) {
            // Only try to decode the percent-encoded character if the percent sign
            // is really followed by two other characters, see testcase at bug 163024
            if (p + 2 < st.length()) {
                ch = st.at(p + 1) - 48;
                if (ch > 16) {
                    ch -= 7;
                }
                ch2 = st.at(p + 2) - 48;
                if (ch2 > 16) {
                    ch2 -= 7;
                }
                st[p] = ch * 16 + ch2;
                st.remove(p + 1, 2);
            }
        }
        p++;
    }
    qCDebug(KMIME_LOG) << "Got pre-decoded:" << st;
    QStringDecoder charsetcodec(charset.constData());
    if (!charsetcodec.isValid() || forceCS) {
        charsetcodec = QStringDecoder(defaultCS.constData());
    }
    if (!charsetcodec.isValid()) {
        charsetcodec = QStringDecoder(QStringDecoder::System);
    }

    usedCS = charsetcodec.name();
    return charsetcodec.decode(st);
}

}
