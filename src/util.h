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

/**
  Checks whether @p s contains any non-us-ascii characters.
  @param s
*/
[[nodiscard]] KMIME_EXPORT bool isUsAscii(QStringView s);

/**
  Returns a user-visible string for a contentEncoding, for example
  "quoted-printable" for CEquPr.
  @param enc the contentEncoding to return string for
  @ since 4.4
  TODO should they be i18n'ed?
*/
[[nodiscard]] KMIME_EXPORT QString nameForEncoding(KMime::Headers::contentEncoding enc);

/**
  Returns a list of encodings that can correctly encode the @p data.
  @param data the data to check encodings for
  @ since 4.4
*/
[[nodiscard]] KMIME_EXPORT QList<KMime::Headers::contentEncoding> encodingsForData(QByteArrayView data);

/**
  Constructs a random string (sans leading/trailing "--") that can
  be used as a multipart delimiter (ie. as @p boundary parameter
  to a multipart/... content-type).

  @return the randomized string.
  @see uniqueString
*/
[[nodiscard]] KMIME_EXPORT QByteArray multiPartBoundary();

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substituted for LF's
  @see LFtoCRLF
*/
[[nodiscard]] KMIME_EXPORT QByteArray CRLFtoLF(const QByteArray &s);

/**
  Converts all occurrences of "\n" (LF) in @p s to "\r\n" (CRLF).

  This function is expensive and should be used only if the mail
  will be transmitted as an RFC822 message later. All decode
  functions can cope with and all encode functions can optionally
  produce both line endings, which is much faster.

  @param s source string containing CRLF's

  @return the string with CRLF's substituted for LF's
  @see CRLFtoLF(const QByteArray&) LFtoCRLF
*/
[[nodiscard]] KMIME_EXPORT QByteArray LFtoCRLF(const QByteArray &s);

/**
 * Returns whether or not the given MIME node is an attachment part.
 * @param content the MIME node to parse
 * @see hasAttachment()
 */
[[nodiscard]] KMIME_EXPORT bool isAttachment(const Content *content);

/**
 * Returns whether or not the given MIME node contains an attachment part. This function will
 *  recursively parse the MIME tree looking for a suitable attachment and return true if one is found.
 * @param content the MIME node to parse
 * @see isAttachment()
 */
[[nodiscard]] KMIME_EXPORT bool hasAttachment(const Content *content);

/**
 * Returns whether or not the given MIME node contains an invitation part. This function will
 *  recursively parse the MIME tree looking for a suitable invitation and return true if one is found.
 * @param content the MIME node to parse
 * @since 4.14.6
 */
[[nodiscard]] KMIME_EXPORT bool hasInvitation(const Content *content);

/**
 * Returns whether or not the given @p message is partly or fully signed.
 *
 * @param message the message to check for being signed
 * @since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isSigned(const Message *message);

/**
 * Returns whether or not the given @p message is partly or fully encrypted.
 *
 * @param message the message to check for being encrypted
 * @since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isEncrypted(const Message *message);

/**
 * Determines if the MIME part @p content is a crypto part.
 * This is, is either an encrypted part or a signature part.
 */
[[nodiscard]] KMIME_EXPORT bool isCryptoPart(const Content *content);

/**
 * Returns whether or not the given MIME @p content is an invitation
 * message of the iTIP protocol.
 *
 * @since 4.6
 */
[[nodiscard]] KMIME_EXPORT bool isInvitation(const Content *content);

} // namespace KMime

