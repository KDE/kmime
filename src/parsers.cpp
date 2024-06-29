/*
    kmime_parsers.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "parsers_p.h"

#include <QRegularExpression>

#include <cctype>

using namespace KMime::Parser;

namespace KMime
{
namespace Parser
{

MultiPart::MultiPart(const QByteArray &src, const QByteArray &boundary)
    : m_src(src)
    , m_boundary(boundary)
{
}

bool MultiPart::parse()
{
    QByteArray b = "--" + m_boundary;
    QByteArray part;
    int pos1 = 0;
    int pos2 = 0;
    int blen = b.length();

    m_parts.clear();

    //find the first valid boundary
    while (true) {
        if ((pos1 = m_src.indexOf(b, pos1)) == -1 || pos1 == 0 ||
                m_src[pos1 - 1] == '\n') { //valid boundary found or no boundary at all
            break;
        }
        pos1 += blen; //boundary found but not valid => skip it;
    }

    if (pos1 > -1) {
        pos1 += blen;
        if (m_src[pos1] == '-' && m_src[pos1 + 1] == '-') {
            // the only valid boundary is the end-boundary
            // this message is *really* broken
            pos1 = -1; //we give up
        } else if ((pos1 - blen) > 1) {     //preamble present
            m_preamble = m_src.left(pos1 - blen - 1);
        }
    }

    while (pos1 > -1 && pos2 > -1) {

        //skip the rest of the line for the first boundary - the message-part starts here
        if ((pos1 = m_src.indexOf('\n', pos1)) > -1) {
            //now search the next linebreak
            //now find the next valid boundary
            pos2 = ++pos1; //pos1 and pos2 point now to the beginning of the next line after the boundary
            while (true) {
                if ((pos2 = m_src.indexOf(b, pos2)) == -1 ||
                        m_src[pos2 - 1] == '\n') { //valid boundary or no more boundaries found
                    break;
                }
                pos2 += blen; //boundary is invalid => skip it;
            }

            if (pos2 == -1) {   // no more boundaries found
                part = m_src.mid(pos1, m_src.length() - pos1);   //take the rest of the string
                m_parts.append(part);
                pos1 = -1;
                pos2 = -1; //break;
            } else {
                part = m_src.mid(pos1, pos2 - pos1 - 1);   // pos2 - 1 (\n) is part of the boundary (see RFC 2046, section 5.1.1)
                m_parts.append(part);
                pos2 += blen; //pos2 points now to the first character after the boundary
                if (m_src[pos2] == '-' && m_src[pos2 + 1] == '-') { //end-boundary
                    pos1 = pos2 + 2; //pos1 points now to the character directly after the end-boundary

                    if ((pos1 = m_src.indexOf('\n', pos1)) > -1) {       //skip the rest of this line
                        //everything after the end-boundary is considered as the epilouge
                        m_epilouge = m_src.mid(pos1 + 1, m_src.length() - pos1 - 1);
                    }
                    pos1 = -1;
                    pos2 = -1; //break
                } else {
                    pos1 = pos2; //the search continues ...
                }
            }
        }
    }

    return !m_parts.isEmpty();
}

//=============================================================================

NonMimeParser::NonMimeParser(const QByteArray &src) :
    m_src(src), m_partNr(-1), m_totalNr(-1)
{
}

NonMimeParser::~NonMimeParser() = default;

/**
 * try to guess the mimetype from the file-extension
 */

QByteArray NonMimeParser::guessMimeType(const QByteArray &fileName)
{
    QByteArray tmp;
    QByteArray mimeType;

    if (!fileName.isEmpty()) {
        int pos = fileName.lastIndexOf('.');
        if (pos++ != -1) {
            tmp = fileName.mid(pos, fileName.length() - pos).toUpper();
            if (tmp == "JPG" || tmp == "JPEG") {
                mimeType = QByteArrayLiteral("image/jpeg");
            } else if (tmp == "GIF") {
                mimeType = QByteArrayLiteral("image/gif");
            } else if (tmp == "PNG") {
                mimeType = QByteArrayLiteral("image/png");
            } else if (tmp == "TIFF" || tmp == "TIF") {
                mimeType = QByteArrayLiteral("image/tiff");
            } else if (tmp == "XPM") {
                mimeType = QByteArrayLiteral("image/x-xpixmap");
            } else if (tmp == "XBM") {
                mimeType = QByteArrayLiteral("image/x-xbitmap");
            } else if (tmp == "BMP") {
                mimeType = QByteArrayLiteral("image/bmp");
            } else if (tmp == "TXT" ||
                       tmp == "ASC" ||
                       tmp == "H" ||
                       tmp == "C" ||
                       tmp == "CC" ||
                       tmp == "CPP") {
                mimeType = QByteArrayLiteral("text/plain");
            } else if (tmp == "HTML" || tmp == "HTM") {
                mimeType = QByteArrayLiteral("text/html");
            } else {
                mimeType = QByteArrayLiteral("application/octet-stream");
            }
        } else {
            mimeType = QByteArrayLiteral("application/octet-stream");
        }
    } else {
        mimeType = QByteArrayLiteral("application/octet-stream");
    }

    return mimeType;
}

//==============================================================================

[[nodiscard]] static qsizetype findUuencodeBeginMarker(const QByteArray &s, qsizetype startPos)
{
    auto idx = startPos;
    while (true) {
        idx = s.indexOf("begin ", idx);
        if (idx < 0 || idx + 9 >= s.size()) {
            break;
        }
        if (std::isdigit(s[idx + 6]) && std::isdigit(s[idx + 7]) && std::isdigit(s[idx + 8])) {
            return idx;
        }
        idx += 6;
    }
    return -1;
}

UUEncoded::UUEncoded(const QByteArray &src, const QByteArray &subject) :
    NonMimeParser(src), m_subject(subject)
{}

bool UUEncoded::parse()
{
    qsizetype currentPos = 0;
    bool success = true;
    bool firstIteration = true;

    while (success) {
        qsizetype beginPos = currentPos;
        qsizetype uuStart = currentPos;
        qsizetype endPos = 0;
        int lineCount = 0;
        int MCount = 0;
        qsizetype pos = 0;
        qsizetype len = 0;
        bool containsBegin = false;
        bool containsEnd = false;
        QByteArray tmp;
        QByteArray fileName;

        if ((beginPos = findUuencodeBeginMarker(m_src, currentPos)) > -1 &&
                (beginPos == 0 || m_src.at(beginPos - 1) == '\n')) {
            containsBegin = true;
            uuStart = m_src.indexOf('\n', beginPos);
            if (uuStart == -1) {  //no more line breaks found, we give up
                success = false;
                break;
            } else {
                uuStart++; //points now at the beginning of the next line
            }
        } else {
            beginPos = currentPos;
        }

        if (!containsBegin || (endPos = m_src.indexOf("\nend", (uuStart > 0) ? uuStart - 1 : 0)) == -1) {
            endPos = m_src.length(); //no end found
        } else {
            containsEnd = true;
        }

        if ((containsBegin && containsEnd) || firstIteration) {

            //printf("beginPos=%d , uuStart=%d , endPos=%d\n", beginPos, uuStart, endPos);
            //all lines in a uuencoded text start with 'M'
            for (auto idx = uuStart; idx < endPos; idx++) {
                if (m_src.at(idx) == '\n') {
                    lineCount++;
                    if (idx + 1 < endPos && m_src.at(idx + 1) == 'M') {
                        idx++;
                        MCount++;
                    }
                }
            }

            //printf("lineCount=%d , MCount=%d\n", lineCount, MCount);
            if (MCount == 0 || (lineCount - MCount) > 10 ||
                    ((!containsBegin || !containsEnd) && (MCount < 15))) {
                // harder check for split-articles
                success = false;
                break; //too many "non-M-Lines" found, we give up
            }

            if ((!containsBegin || !containsEnd) && !m_subject.isNull()) {
                // message may be split up => parse subject
                const QRegularExpression subjectRegex(QStringLiteral("[0-9]+/[0-9]+"));
                const auto match = subjectRegex.match(QLatin1StringView(m_subject));
                pos = match.capturedStart(0);
                len = match.capturedLength(0);
                if (pos != -1) {
                    tmp = m_subject.mid(pos, len);
                    pos = tmp.indexOf('/');
                    m_partNr = tmp.left(pos).toInt();
                    m_totalNr = tmp.right(tmp.length() - pos - 1).toInt();
                } else {
                    success = false;
                    break; //no "part-numbers" found in the subject, we give up
                }
            }

            //everything before "begin" is text
            if (beginPos > 0) {
                m_text.append(m_src.mid(currentPos, beginPos - currentPos));
            }

            if (containsBegin) {
                //everything between "begin ### " and the next LF is considered as the filename
                fileName = m_src.mid(beginPos + 10, uuStart - beginPos - 11);
            } else {
                fileName = "";
            }
            m_filenames.append(fileName);
            //everything between "begin" and "end" is uuencoded
            m_bins.append(m_src.mid(uuStart, endPos - uuStart + 1));
            m_mimeTypes.append(guessMimeType(fileName));
            firstIteration = false;

            auto next = m_src.indexOf('\n', endPos + 1);
            if (next == -1) {   //no more line breaks found, we give up
                success = false;
                break;
            } else {
                next++; //points now at the beginning of the next line
            }
            currentPos = next;

        } else {
            success = false;
        }
    }

    // append trailing text part of the article (only
    if (!m_bins.isEmpty() || isPartial()) {
        m_text.append(m_src.right(m_src.length() - currentPos));
        return true;
    }

    return false;
}

//==============================================================================

YENCEncoded::YENCEncoded(const QByteArray &src) :
    NonMimeParser(src)
{
}

bool YENCEncoded::yencMeta(QByteArray &src, const QByteArray &name, int *value)
{
    bool found = false;
    QByteArray sought = name + '=';

    int iPos = src.indexOf(sought);
    if (iPos > -1) {
        int pos1 = src.indexOf(' ', iPos);
        int pos2 = src.indexOf('\r', iPos);
        int pos3 = src.indexOf('\t', iPos);
        int pos4 = src.indexOf('\n', iPos);
        if (pos2 >= 0 && (pos1 < 0 || pos1 > pos2)) {
            pos1 = pos2;
        }
        if (pos3 >= 0 && (pos1 < 0 || pos1 > pos3)) {
            pos1 = pos3;
        }
        if (pos4 >= 0 && (pos1 < 0 || pos1 > pos4)) {
            pos1 = pos4;
        }
        iPos = src.lastIndexOf('=', pos1) + 1;
        if (iPos < pos1) {
            char c = src.at(iPos);
            if (c >= '0' && c <= '9') {
                found = true;
                *value = src.mid(iPos, pos1 - iPos).toInt();
            }
        }
    }
    return found;
}

bool YENCEncoded::parse()
{
    int currentPos = 0;
    bool success = true;
    while (success) {
        int beginPos = currentPos;
        int yencStart = currentPos;
        bool containsPart = false;
        QByteArray fileName;

        if ((beginPos = m_src.indexOf("=ybegin ", currentPos)) > -1 &&
                (beginPos == 0 || m_src.at(beginPos - 1) == '\n')) {
            yencStart = m_src.indexOf('\n', beginPos);
            if (yencStart == -1) {   // no more line breaks found, give up
                success = false;
                break;
            } else {
                yencStart++;
                if (m_src.indexOf("=ypart", yencStart) == yencStart) {
                    containsPart = true;
                    yencStart = m_src.indexOf('\n', yencStart);
                    if (yencStart == -1) {
                        success = false;
                        break;
                    }
                    yencStart++;
                }
            }
            // Try to identify yenc meta data

            // Filenames can contain any embedded chars until end of line
            QByteArray meta = m_src.mid(beginPos, yencStart - beginPos);
            int namePos = meta.indexOf("name=");
            if (namePos == -1) {
                success = false;
                break;
            }
            int eolPos = meta.indexOf('\r', namePos);
            if (eolPos == -1) {
                eolPos = meta.indexOf('\n', namePos);
            }
            if (eolPos == -1) {
                success = false;
                break;
            }
            fileName = meta.mid(namePos + 5, eolPos - (namePos + 5));

            // Other metadata is integer
            int yencLine;
            if (!yencMeta(meta, "line", &yencLine)) {
                success = false;
                break;
            }
            int yencSize;
            if (!yencMeta(meta, "size", &yencSize)) {
                success = false;
                break;
            }

            int partBegin;
            int partEnd;
            if (containsPart) {
                if (!yencMeta(meta, "part", &m_partNr)) {
                    success = false;
                    break;
                }
                if (!yencMeta(meta, "begin", &partBegin) ||
                        !yencMeta(meta, "end", &partEnd)) {
                    success = false;
                    break;
                }
                if (!yencMeta(meta, "total", &m_totalNr)) {
                    m_totalNr = m_partNr + 1;
                }
                if (yencSize == partEnd - partBegin + 1) {
                    m_totalNr = 1;
                } else {
                    yencSize = partEnd - partBegin + 1;
                }
            }

            // We have a valid yenc header; now we extract the binary data
            int totalSize = 0;
            int pos = yencStart;
            int len = m_src.length();
            bool lineStart = true;
            int lineLength = 0;
            bool containsEnd = false;
            QByteArray binary;
            binary.resize(yencSize);
            while (pos < len) {
                int ch = m_src.at(pos);
                if (ch < 0) {
                    ch += 256;
                }
                if (ch == '\r') {
                    if (lineLength != yencLine && totalSize != yencSize) {
                        break;
                    }
                    pos++;
                } else if (ch == '\n') {
                    lineStart = true;
                    lineLength = 0;
                    pos++;
                } else {
                    if (ch == '=') {
                        if (pos + 1 < len) {
                            ch = m_src.at(pos + 1);
                            if (lineStart && ch == 'y') {
                                containsEnd = true;
                                break;
                            }
                            pos += 2;
                            ch -= 64 + 42;
                            if (ch < 0) {
                                ch += 256;
                            }
                            if (totalSize >= yencSize) {
                                break;
                            }
                            binary[totalSize++] = ch;
                            lineLength++;
                        } else {
                            break;
                        }
                    } else {
                        ch -= 42;
                        if (ch < 0) {
                            ch += 256;
                        }
                        if (totalSize >= yencSize) {
                            break;
                        }
                        binary[totalSize++] = ch;
                        lineLength++;
                        pos++;
                    }
                    lineStart = false;
                }
            }

            if (!containsEnd) {
                success = false;
                break;
            }
            if (totalSize != yencSize) {
                success = false;
                break;
            }

            // pos now points to =yend; get end data
            eolPos = m_src.indexOf('\n', pos);
            if (eolPos == -1) {
                success = false;
                break;
            }
            meta = m_src.mid(pos, eolPos - pos);
            if (!yencMeta(meta, "size", &totalSize)) {
                success = false;
                break;
            }
            if (totalSize != yencSize) {
                success = false;
                break;
            }

            m_filenames.append(fileName);
            m_mimeTypes.append(guessMimeType(fileName));
            m_bins.append(binary);

            //everything before "begin" is text
            if (beginPos > 0) {
                m_text.append(m_src.mid(currentPos, beginPos - currentPos));
            }
            currentPos = eolPos + 1;

        } else {
            success = false;
        }
    }

    // append trailing text part of the article
    m_text.append(m_src.right(m_src.length() - currentPos));

    return !m_bins.isEmpty();
}

} // namespace Parser

} // namespace KMime
