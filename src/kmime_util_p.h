/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

class QByteArray;
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
extern int findHeaderLineEnd(const QByteArray &src, int &dataBegin, bool *folded = nullptr);

/**
  Finds the first header of type @p name in @p src.
  @param end The end index of the header.
  @param dataBegin begin of the data part of the header, -1 if not found.
  @param folded true if the headder is folded into multiple lines
  @returns the begin index of the header, -1 if not found.
*/
extern int indexOfHeader(const QByteArray &src, const QByteArray &name, int &end, int &dataBegin, bool *folded = nullptr);

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

