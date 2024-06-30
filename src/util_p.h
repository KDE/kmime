/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <qglobal.h>

class QByteArray;
class QByteArrayView;
class QString;

#include <cstdlib>

// @cond PRIVATE

/* Internal helper functions. Not part of the public API. */

namespace KMime
{

/**
 *  Consult the charset cache. Only used for reducing mem usage by
 *  keeping strings in a common repository.
 *  @param name
 */
extern QByteArray cachedCharset(const QByteArray &name);

/**
  Finds the header end in @p src. Aligns the @p dataBegin if needed.
  @param dataBegin beginning of the data part of the header
  @param folded true if the headder is folded into multiple lines
  @returns the end index of the header, -1 if the @p dataBegin was -1.
*/
qsizetype findHeaderLineEnd(QByteArrayView src, qsizetype &dataBegin, bool *folded = nullptr);

/**
  Tries to extract the header with name @p name from the string
  @p src, unfolding it if necessary.

  @param src  the source string.
  @param name the name of the header to search for.

  @return the first instance of the header @p name in @p src
          or a null QByteArray if no such header was found.
*/
QByteArray extractHeader(const QByteArray &src, const QByteArray &name);

/**
  Finds the first header of type @p name in @p src.
  @param end The end index of the header.
  @param dataBegin begin of the data part of the header, -1 if not found.
  @param folded true if the headder is folded into multiple lines
  @returns the begin index of the header, -1 if not found.
*/
qsizetype indexOfHeader(const QByteArray &src, const QByteArray &name, qsizetype &end, qsizetype &dataBegin, bool *folded = nullptr);

/**
 *  Uses current time, pid and random numbers to construct a string
 *  that aims to be unique on a per-host basis (ie. for the local
 *  part of a message-id or for multipart boundaries.
 *
 *  @return the unique string.
 *  @see multiPartBoundary
 */
extern QByteArray uniqueString();

/**
  Unfolds the given header if necessary.
  @param header The header to unfold.
*/
QByteArray unfoldHeader(const QByteArray &header);
QByteArray unfoldHeader(const char *header, size_t headerSize);

/**
  Folds the given header if necessary.
  @param header The header to fold.
*/
QByteArray foldHeader(const QByteArray &header);

/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
void removeQuotes(QByteArray &str);

/**
  Removes quote (DQUOTE) characters and decodes "quoted-pairs"
  (ie. backslash-escaped characters)

  @param str the string to work on.
  @see addQuotes
*/
void removeQuotes(QString &str);

/**
  Converts the given string into a quoted-string if the string contains
  any special characters (ie. one of ()<>@,.;:[]=\").

  @param str us-ascii string to work on.
  @param forceQuotes if @c true, always add quote characters.
*/
void addQuotes(QByteArray &str, bool forceQuotes);

/**
 * Overloaded method, behaves same as the above.
 * @param str us-ascii string to work on.
 * @param forceQuotes if @c true, always add quote characters.
 * @since 4.5
 */
void addQuotes(QString &str, bool forceQuotes);

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
QString balanceBidiState(const QString &input);

/**
 * Similar to the above function. Instead of trying to balance the Bidi chars, it outright
 * removes them from the string.
 *
 * @param input the display name of a mailbox, which is checked for unbalanced Unicode
 * direction control characters
 * Reason: KHTML seems to ignore the PDF character, so adding them doesn't fix things :(
 */
QString removeBidiControlChars(const QString &input);

//@cond PRIVATE
extern const unsigned char aTextMap[16];
extern const unsigned char tTextMap[16];

inline bool isOfSet(const unsigned char map[16], unsigned char ch)
{
    return (ch < 128) && (map[ ch / 8 ] & 0x80 >> ch % 8);
}
inline bool isAText(char ch)
{
    return isOfSet(aTextMap, ch);
}
inline bool isTText(char ch)
{
    return isOfSet(tTextMap, ch);
}
//@endcond

}

// @endcond

