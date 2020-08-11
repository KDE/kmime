/*  -*- c++ -*-
    kmime_util.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __KMIME_UTIL_H__
#define __KMIME_UTIL_H__

#include "kmime_export.h"
#include "kmime_headers.h"
#include "kmime_content.h"

#include <QString>
#include <QVector>

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
Q_REQUIRED_RESULT KMIME_EXPORT QVector<KMime::Headers::contentEncoding> encodingsForData(const QByteArray &data);

/**
  * Set whether or not to use outlook compatible attachment filename encoding. Outlook
  *  fails to properly adhere to the RFC2322 standard for parametrized header fields, and
  *  instead is only able to read and write attachment filenames encoded in RFC2047-style.
  *  This will create mails that are not standards-compliant!
  *
  * @param violateStandard      Whether or not to use outlook-compatible attachment
  *                              filename encodings.
  *
  * @since 4.5
  */
KMIME_EXPORT extern void setUseOutlookAttachmentEncoding(bool violateStandard);

/**
 * Retrieve whether or not to use outlook compatible encodings for attachments.
 */
KMIME_EXPORT extern bool useOutlookAttachmentEncoding();

/**
  Constructs a random string (sans leading/trailing "--") that can
  be used as a multipart delimiter (ie. as @p boundary parameter
  to a multipart/... content-type).

  @return the randomized string.
  @see uniqueString
*/
KMIME_EXPORT extern QByteArray multiPartBoundary();

/**
  Unfolds the given header if necessary.
  @param header The header to unfold.
*/

KMIME_EXPORT extern QByteArray unfoldHeader(const QByteArray &header);
KMIME_EXPORT extern QByteArray unfoldHeader(const char *header, size_t headerSize);

/**
  Tries to extract the header with name @p name from the string
  @p src, unfolding it if necessary.

  @param src  the source string.
  @param name the name of the header to search for.

  @return the first instance of the header @p name in @p src
          or a null QByteArray if no such header was found.
*/
KMIME_EXPORT extern QByteArray extractHeader(const QByteArray &src,
        const QByteArray &name);

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substitued for LF's
  @see CRLFtoLF(const char*) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray CRLFtoLF(const QByteArray &s);

/**
  Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CRLF's

  @return the string with CRLF's substitued for LF's
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

  @return the string with CRLF's substitued for LF's
  @see CRLFtoLF(const QByteArray&) LFtoCRLF
*/
KMIME_EXPORT extern QByteArray LFtoCRLF(const QByteArray &s);

/**
  Converts all occurrences of "\r" (CR) in @p s to "\n" (LF).

  This function is expensive and should be used only if the mail
  will be stored locally. All decode functions can cope with both
  line endings.

  @param s source string containing CR's

  @return the string with CR's substitued for LF's
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

  @return the string with CR's substitued for LF's
  @see CRtoLF(const QByteArray&) CRtoLF
*/
KMIME_EXPORT extern QByteArray CRtoLF(const QByteArray &s);


/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
KMIME_EXPORT extern void removeQuotes(QByteArray &str);

/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
KMIME_EXPORT extern void removeQuotes(QString &str);

/**
  Converts the given string into a quoted-string if the string contains
  any special characters (ie. one of ()<>@,.;:[]=\").

  @param str us-ascii string to work on.
  @param forceQuotes if @c true, always add quote characters.
*/
KMIME_EXPORT extern void addQuotes(QByteArray &str, bool forceQuotes);

/**
 * Overloaded method, behaves same as the above.
 * @param str us-ascii string to work on.
 * @param forceQuotes if @c true, always add quote characters.
 * @since 4.5
 */
KMIME_EXPORT extern void addQuotes(QString &str, bool forceQuotes);

/**
 * Makes sure that the bidirectional state at the end of the string is the
 * same as at the beginning of the string.
 *
 * This is useful so that Unicode control characters that can change the text
 * direction can not spill over to following strings.
 *
 * As an example, consider a mailbox in the form "display name" <local@domain.com>.
 * If the display name here contains unbalanced control characters that change the
 * text direction, it would also have an effect on the addrspec, which could lead to
 * spoofing.
 *
 * By passing the display name to this function, one can make sure that no change of
 * the bidi state can spill over to the next strings, in this case the addrspec.
 *
 * Example: The string "Hello <RLO>World" is unbalanced, as it contains a right-to-left
 *          override character, which is never followed by a <PDF>, the "pop directional
 *          formatting" character. This function adds the missing <PDF> at the end, and
 *          the output of this function would be "Hello <RLO>World<PDF>".
 *
 * Example of spoofing:
 *   Consider "Firstname Lastname<RLO>" <moc.mitciv@attacker.com>. Because of the RLO,
 *   it is displayed as "Firstname Lastname <moc.rekcatta@victim.com>", which spoofs the
 *   domain name.
 *   By passing "Firstname Lastname<RLO>" to this function, one can balance the <RLO>,
 *   leading to "Firstname Lastname<RLO><PDF>", so the whole mailbox is displayed
 *   correctly as "Firstname Lastname" <moc.mitciv@attacker.com> again.
 *
 * See https://unicode.org/reports/tr9 for more information on bidi control chars.
 *
 * @param input the display name of a mailbox, which is checked for unbalanced Unicode
 *              direction control characters
 * @return the display name which now contains a balanced state of direction control
 *         characters
 *
 * Note that this function does not do any parsing related to mailboxes, it only works
 * on plain strings. Therefore, passing the complete mailbox will not lead to any results,
 * only the display name should be passed.
 *
 * @since 4.5
 */
KMIME_EXPORT QString balanceBidiState(const QString &input);

/**
 * Similar to the above function. Instead of trying to balance the Bidi chars, it outright
 * removes them from the string.
 *
 * @param input the display name of a mailbox, which is checked for unbalanced Unicode
 * direction control characters
 * Reason: KHTML seems to ignore the PDF character, so adding them doesn't fix things :(
 */
KMIME_EXPORT QString removeBidiControlChars(const QString &input);

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

#endif /* __KMIME_UTIL_H__ */
