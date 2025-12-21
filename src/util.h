/*  -*- c++ -*-
    kmime_util.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kmime_export.h"
#include "headers.h"
#include "content.h"

#include <QList>
#include <QString>

namespace KMime
{

class Message;

/*!
  Checks whether \a s contains any non-us-ascii characters.
  \a s
*/
[[nodiscard]] KMIME_EXPORT bool isUsAscii(QStringView s);

/*!
  Returns a user-visible string for a contentEncoding, for example
  "quoted-printable" for CEquPr.
  \a enc the contentEncoding to return string for
  @ since 4.4
  TODO should they be i18n'ed?
*/
[[nodiscard]] KMIME_EXPORT QString nameForEncoding(KMime::Headers::contentEncoding enc);

/*!
  Returns a list of encodings that can correctly encode the \a data.
  \a data the data to check encodings for
  @ since 4.4
*/
[[nodiscard]] KMIME_EXPORT QList<KMime::Headers::contentEncoding> encodingsForData(QByteArrayView data);

/*!
  Constructs a random string (sans leading/trailing "--") that can
  be used as a multipart delimiter (ie. as \a boundary parameter
  to a multipart/... content-type).

  Returns the randomized string.
  \sa uniqueString
*/
[[nodiscard]] KMIME_EXPORT QByteArray multiPartBoundary();

/*!
  Converts all occurrences of "\r\n" (CRLF) in \a s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  \a s source string containing CRLF's

  Returns the string with CRLF's substituted for LF's
  \sa LFtoCRLF
*/
[[nodiscard]] KMIME_EXPORT QByteArray CRLFtoLF(const QByteArray &s);

/*!
  Converts all occurrences of "\n" (LF) in \a s to "\r\n" (CRLF).

  This function is expensive and should be used only if the mail
  will be transmitted as an RFC822 message later. All decode
  functions can cope with and all encode functions can optionally
  produce both line endings, which is much faster.

  \a s source string containing CRLF's

  Returns the string with CRLF's substituted for LF's
  \sa CRLFtoLF(const QByteArray&) LFtoCRLF
*/
[[nodiscard]] KMIME_EXPORT QByteArray LFtoCRLF(const QByteArray &s);

/*!
 * Returns whether or not the given MIME node is an attachment part.
 * \a content the MIME node to parse
 * \sa hasAttachment()
 */
[[nodiscard]] KMIME_EXPORT bool isAttachment(const Content *content);

/*!
 * Returns whether or not the given MIME node contains an attachment part. This function will
 *  recursively parse the MIME tree looking for a suitable attachment and return true if one is found.
 * \a content the MIME node to parse
 * \sa isAttachment()
 */
[[nodiscard]] KMIME_EXPORT bool hasAttachment(const Content *content);

/*!
 * Returns whether or not the given MIME node contains an invitation part. This function will
 *  recursively parse the MIME tree looking for a suitable invitation and return true if one is found.
 * \a content the MIME node to parse
 * \since 4.14.6
 */
[[nodiscard]] KMIME_EXPORT bool hasInvitation(const Content *content);

/*!
 * Returns whether or not the given \a message is partly or fully signed.
 *
 * \a message the message to check for being signed
 * \since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isSigned(const Message *message);

/*!
 * Returns whether or not the given \a message is partly or fully encrypted.
 *
 * \a message the message to check for being encrypted
 * \since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isEncrypted(const Message *message);

/*!
 * Determines if the MIME part \a content is a crypto part.
 * This is, is either an encrypted part or a signature part.
 */
[[nodiscard]] KMIME_EXPORT bool isCryptoPart(const Content *content);

/*!
 * Returns whether or not the given MIME \a content is an invitation
 * message of the iTIP protocol.
 *
 * \since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isInvitation(const Content *content);

} // namespace KMime

