/*
    kmime_parsers.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QByteArray>
#include <QList>

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

    [[nodiscard]] bool parse();
    [[nodiscard]] QList<QByteArray> parts() const { return m_parts; }
    [[nodiscard]] QByteArray preamble() const { return m_preamble; }
    [[nodiscard]] QByteArray epilouge() const { return m_epilouge; }

  private:
    const QByteArray m_src;
    const QByteArray m_boundary;
    QByteArray m_preamble;
    QByteArray m_epilouge;
    QList<QByteArray> m_parts;
};

/** Helper-class: abstract base class of all parsers for
    non-mime binary data (uuencoded, yenc)
    @internal
*/
class NonMimeParser
{
public:
    [[nodiscard]] bool isPartial() const {
      return (m_partNr > -1 && m_totalNr > -1 && m_totalNr != 1);
    }
    [[nodiscard]] int partialNumber() const { return m_partNr; }
    [[nodiscard]] int partialCount() const { return m_totalNr; }
    [[nodiscard]] bool hasTextPart() const { return (m_text.length() > 1); }
    [[nodiscard]] QByteArray textPart() const { return m_text; }
    [[nodiscard]] QList<QByteArray> binaryParts() const { return m_bins; }
    [[nodiscard]] QList<QByteArray> filenames() const { return m_filenames; }
    [[nodiscard]] QList<QByteArray> mimeTypes() const { return m_mimeTypes; }

protected:
    explicit NonMimeParser(const QByteArray &src);
    ~NonMimeParser();

    QByteArray m_src, m_text;
    QList<QByteArray> m_bins, m_filenames, m_mimeTypes;
    int m_partNr, m_totalNr;
};

/** Helper-class: tries to extract the data from a possibly
    uuencoded message
    @internal
*/
class UUEncoded : public NonMimeParser
{
public:
    UUEncoded(const QByteArray &src, const QByteArray &head);

    [[nodiscard]] bool parse();

  private:
    QByteArray m_head;
};

/** Helper-class: tries to extract the data from a possibly
    yenc encoded message
    @internal
*/
class YENCEncoded : public NonMimeParser
{
public:
    explicit YENCEncoded(const QByteArray &src);

    [[nodiscard]] bool parse();

  private:
    static bool yencMeta(QByteArrayView src, QByteArrayView name, int *value);
};

} // namespace Parser

} // namespace KMime

