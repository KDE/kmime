/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KMIME_UTIL_P_H
#define KMIME_UTIL_P_H

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

//@cond PRIVATE
extern const uchar aTextMap[16];
extern const uchar tTextMap[16];

inline bool isOfSet(const uchar map[16], unsigned char ch)
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

#endif
