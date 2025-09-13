/*
    kmime_parsers.cpp

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "parsers_p.h"
#include "util_p.h"

#include <QMimeDatabase>
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
    qsizetype pos1 = 0;
    qsizetype pos2 = 0;
    auto blen = b.length();

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
        if ((pos1 + 1) < m_src.length() && m_src[pos1] == '-' && m_src[pos1 + 1] == '-') {
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
                if (pos1 != pos2) { // skip entirely empty parts
                    part = m_src.mid(pos1, pos2 - pos1 - 1);   // pos2 - 1 (\n) is part of the boundary (see RFC 2046, section 5.1.1)
                    m_parts.append(part);
                }
                pos2 += blen; //pos2 points now to the first character after the boundary
                if ((pos2 + 1) < m_src.length() && m_src[pos2] == '-' && m_src[pos2 + 1] == '-') { //end-boundary
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

UUEncoded::UUEncoded(const QByteArray &src, const QByteArray &head) :
    NonMimeParser(src), m_head(head)
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

                    // partial version of the check below, for things that can be evaluated
                    // while this loop is still counting
                    if ((lineCount - MCount) > 10) {
                        success = false;
                        break;
                    }
                }
            }

            //printf("lineCount=%d , MCount=%d\n", lineCount, MCount);
            if (MCount == 0 || (lineCount - MCount) > 10 || ((!containsBegin || !containsEnd) && (MCount < 15))) {
                // harder check for split-articles
                success = false;
                break; //too many "non-M-Lines" found, we give up
            }

            const auto subject = KMime::extractHeader(m_head, "Subject");
            if ((!containsBegin || !containsEnd) && !subject.isNull()) {
                // message may be split up => parse subject
                const QRegularExpression subjectRegex(QStringLiteral("[0-9]+/[0-9]+"));
                const auto match = subjectRegex.match(QLatin1StringView(subject));
                pos = match.capturedStart(0);
                len = match.capturedLength(0);
                if (pos != -1) {
                    tmp = subject.mid(pos, len);
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
            QMimeDatabase db;
            m_mimeTypes.append(db.mimeTypeForFile(QString::fromUtf8(fileName), QMimeDatabase::MatchExtension).name().toUtf8());
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

bool YENCEncoded::yencMeta(QByteArrayView src, QByteArrayView name, int *value)
{
    for (qsizetype idx = 0; idx < src.size() - name.size() - 2;) {
        idx = src.indexOf(name, idx);
        if (idx < 0 || idx >= src.size() - name.size() - 2) {
            return false;
        }
        idx += name.size();
        if (src[idx] != '=') {
            continue;
        }
        ++idx;
        auto endIdx = idx;
        for (; endIdx < src.size() && std::isdigit(src[endIdx]); ++endIdx) {}
        if (endIdx <= idx) {
            continue;
        }

        *value = src.mid(idx, endIdx - idx).toInt();
        return true;
    }

    return false;
}

bool YENCEncoded::parse()
{
    qsizetype currentPos = 0;
    bool success = true;
    while (success) {
        qsizetype beginPos = currentPos;
        qsizetype yencStart = currentPos;
        bool containsPart = false;
        QByteArrayView fileName;

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
            auto meta = QByteArrayView(m_src).mid(beginPos, yencStart - beginPos);
            qsizetype namePos = meta.indexOf("name=");
            if (namePos == -1) {
                success = false;
                break;
            }
            qsizetype eolPos = meta.indexOf('\r', namePos);
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
            qsizetype pos = yencStart;
            qsizetype len = m_src.length();
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
            meta = QByteArrayView(m_src).mid(pos, eolPos - pos);
            if (!yencMeta(meta, "size", &totalSize)) {
                success = false;
                break;
            }
            if (totalSize != yencSize) {
                success = false;
                break;
            }

            m_filenames.append(fileName.toByteArray());
            QMimeDatabase db;
            m_mimeTypes.append(db.mimeTypeForFile(QString::fromUtf8(fileName), QMimeDatabase::MatchExtension).name().toUtf8());
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
    if (!m_bins.isEmpty()) {
        m_text.append(m_src.right(m_src.length() - currentPos));
        return true;
    }

    return false;
}

} // namespace Parser

} // namespace KMime
