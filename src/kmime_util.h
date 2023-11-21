/*  -*- c++ -*-
    kmime_util.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "kmime_export.h"
#include "kmime_headers.h"
#include "kmime_content.h"

#include <QList>
#include <QString>

namespace KMime
{

class Message;

/**
  Checks whether @p s contains any non-us-ascii characters.
  @param s
*/
KMIME_EXPORT extern bool isUsAscii(const QString &s);

/**
  Returns a user-visible string for a contentEncoding, for example
  "quoted-printable" for CEquPr.
  @param enc the contentEncoding to return string for
  @ since 4.4
  TODO should they be i18n'ed?
*/
KMIME_EXPORT extern QString nameForEncoding(KMime::Headers::contentEncoding enc);

/**
  Returns a list of encodings that can correctly encode the @p data.
  @param data the data to check encodings for
  @ since 4.4
*/
[[nodiscard]] KMIME_EXPORT QList<KMime::Headers::contentEncoding>
encodingsForData(const QByteArray &data);

/**
  Constructs a random string (sans leading/trailing "--") that can
  be used as a multipart delimiter (ie. as @p boundary parameter
  to a multipart/... content-type).

  @return the randomized string.
  @see uniqueString
*/
KMIME_EXPORT extern QByteArray multiPartBoundary();

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substituted for LF's
  @see CRLFtoLF(const char*) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray CRLFtoLF(const QByteArray &s);

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substituted for LF's
  @see CRLFtoLF(const QByteArray&) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray CRLFtoLF(const char *s);

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
KMIME_EXPORT extern QByteArray LFtoCRLF(const QByteArray &s);

/**
  Converts all occurrences of "\r" (CR) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CR's

  @return the string with CR's substituted for LF's
  @see CRtoLF(const QByteArray&) CRtoLF
*/
KMIME_EXPORT extern QByteArray CRtoLF(const char *s);

/**
  Converts all occurrences of "\r" (CR) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be transmitted as an RFC822 message later. All decode
  functions can cope with and all encode functions can optionally
  produce both line endings, which is much faster.

  @param s source string containing CR's

  @return the string with CR's substituted for LF's
  @see CRtoLF(const QByteArray&) CRtoLF
*/
KMIME_EXPORT extern QByteArray CRtoLF(const QByteArray &s);

/**
 * Returns whether or not the given MIME node is an attachment part.
 * @param content the MIME node to parse
 * @see hasAttachment()
 */
KMIME_EXPORT bool isAttachment(Content *content);

/**
 * Returns whether or not the given MIME node contains an attachment part. This function will
 *  recursively parse the MIME tree looking for a suitable attachment and return true if one is found.
 * @param content the MIME node to parse
 * @see isAttachment()
 */
KMIME_EXPORT bool hasAttachment(Content *content);

/**
 * Returns whether or not the given MIME node contains an invitation part. This function will
 *  recursively parse the MIME tree looking for a suitable invitation and return true if one is found.
 * @param content the MIME node to parse
 * @since 4.14.6
 */
KMIME_EXPORT bool hasInvitation(Content *content);

/**
 * Returns whether or not the given @p message is partly or fully signed.
 *
 * @param message the message to check for being signed
 * @since 4.6
 */
KMIME_EXPORT bool isSigned(Message *message);

/**
 * Returns whether or not the given @p message is partly or fully encrypted.
 *
 * @param message the message to check for being encrypted
 * @since 4.6
 */
KMIME_EXPORT bool isEncrypted(Message *message);

/**
 * Determines if the MIME part @p content is a crypto part.
 * This is, is either an encrypted part or a signature part.
 */
KMIME_EXPORT bool isCryptoPart(Content *content);

/**
 * Returns whether or not the given MIME @p content is an invitation
 * message of the iTIP protocol.
 *
 * @since 4.6
 */
KMIME_EXPORT bool isInvitation(Content *content);

} // namespace KMime

