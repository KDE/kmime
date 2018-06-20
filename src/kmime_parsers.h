/*
    kmime_parsers.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
    See file AUTHORS for details

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
#ifndef __KMIME_PARSERS__
#define __KMIME_PARSERS__

#include <QByteArray>
#include <QVector>

namespace KMime
{

namespace Parser
{

/** Helper-class: splits a multipart-message into single
    parts as described in RFC 2046
    @internal
*/
class MultiPart
{
public:
    MultiPart(const QByteArray &src, const QByteArray &boundary);

    Q_REQUIRED_RESULT bool parse();
    Q_REQUIRED_RESULT QVector<QByteArray> parts() const
    {
        return m_parts;
    }
    Q_REQUIRED_RESULT QByteArray preamble() const
    {
        return m_preamble;
    }
    Q_REQUIRED_RESULT QByteArray epilouge() const
    {
        return m_epilouge;
    }

private:
    QByteArray m_src, m_boundary, m_preamble, m_epilouge;
    QVector<QByteArray> m_parts;
};

/** Helper-class: abstract base class of all parsers for
    non-mime binary data (uuencoded, yenc)
    @internal
*/
class NonMimeParser
{
public:
    explicit NonMimeParser(const QByteArray &src);
    virtual ~NonMimeParser();
    virtual bool parse() = 0;
    Q_REQUIRED_RESULT bool isPartial() const
    {
        return (m_partNr > -1 && m_totalNr > -1 && m_totalNr != 1);
    }
    Q_REQUIRED_RESULT int partialNumber() const
    {
        return m_partNr;
    }
    Q_REQUIRED_RESULT int partialCount() const
    {
        return m_totalNr;
    }
    Q_REQUIRED_RESULT bool hasTextPart() const
    {
        return (m_text.length() > 1);
    }
    Q_REQUIRED_RESULT QByteArray textPart() const
    {
        return m_text;
    }
    Q_REQUIRED_RESULT QVector<QByteArray> binaryParts() const
    {
        return m_bins;
    }
    Q_REQUIRED_RESULT QVector<QByteArray> filenames() const
    {
        return m_filenames;
    }
    Q_REQUIRED_RESULT QVector<QByteArray> mimeTypes() const
    {
        return m_mimeTypes;
    }

protected:
    static QByteArray guessMimeType(const QByteArray &fileName);

    QByteArray m_src, m_text;
    QVector<QByteArray> m_bins, m_filenames, m_mimeTypes;
    int m_partNr, m_totalNr;
};

/** Helper-class: tries to extract the data from a possibly
    uuencoded message
    @internal
*/
class UUEncoded : public NonMimeParser
{
public:
    UUEncoded(const QByteArray &src, const QByteArray &subject);

    Q_REQUIRED_RESULT bool parse() override;

private:
    QByteArray m_subject;
};

/** Helper-class: tries to extract the data from a possibly
    yenc encoded message
    @internal
*/
class YENCEncoded : public NonMimeParser
{
public:
    explicit YENCEncoded(const QByteArray &src);

    Q_REQUIRED_RESULT bool parse() override;

private:
    static bool yencMeta(QByteArray &src, const QByteArray &name, int *value);
};

} // namespace Parser

} // namespace KMime

#endif // __KMIME_PARSERS__
