/*  -*- c++ -*-
    kmime_util.h

    KMime, the KDE internet mail/usenet news message library.
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
#ifndef __KMIME_UTIL_H__
#define __KMIME_UTIL_H__

#include <time.h>
#include <qdatetime.h>
#include <qstring.h>
#include "kmime.h"

namespace KMime {

/**
 * Consult the charset cache. Only used for reducing mem usage by
 * keeping strings in a common repository.
 * @param name
 */
extern QByteArray cachedCharset( const QByteArray &name ) KMIME_EXPORT;

/**
 * Consult the language cache. Only used for reducing mem usage by
 * keeping strings in a common repository.
 * @param name
 */
extern QByteArray cachedLanguage( const QByteArray &name ) KMIME_EXPORT;

/**
 * Checks whether @p s contains any non-us-ascii characters.
 * @param s
 */
extern bool isUsAscii( const QString &s ) KMIME_EXPORT;

inline bool isOfSet( const uchar map[16], unsigned char ch )
{
  Q_ASSERT( ch < 128 );
  return ( map[ ch/8 ] & 0x80 >> ch%8 );
}

extern const uchar specialsMap[16];
extern const uchar tSpecialsMap[16];
extern const uchar aTextMap[16];
extern const uchar tTextMap[16];
extern const uchar eTextMap[16];

inline bool isSpecial( char ch )
{
  return isOfSet( specialsMap, ch );
}
inline bool isTSpecial( char ch )
{
  return isOfSet( tSpecialsMap, ch );
}
inline bool isAText( char ch )
{
  return isOfSet( aTextMap, ch );
}
inline bool isTText( char ch )
{
  return isOfSet( tTextMap, ch );
}
inline bool isEText( char ch )
{
  return isOfSet( eTextMap, ch );
}

/**
 * Decode string @p src according to RFC2047 (ie. the
 * =?charset?[qb]?encoded?= construct).
 *
 * @param src       source string.
 * @param usedCS    the detected charset is returned here
 * @param defaultCS the charset to use in case the detected
 *                  one isn't known to us.
 * @param forceCS   force the use of the default charset.
 *
 * @return the decoded string.
 */
extern QString decodeRFC2047String(
  const QByteArray &src, QByteArray &usedCS, const QByteArray &defaultCS,
  bool forceCS ) KMIME_EXPORT;

/**
 * Encode string @p src according to RFC2047 using charset @p charset.
 *
 * @param src           source string.
 * @param charset       charset to use.
 * @param addressHeader if this flag is true, all special chars
 *                      like <,>,[,],... will be encoded, too.
 * @param allow8bitHeaders if this flag is true, 8Bit headers are allowed.
 *
 * @return the encoded string.
 */
extern QByteArray encodeRFC2047String(
  const QString &src, const QByteArray &charset, bool addressHeader=false,
  bool allow8bitHeaders=false ) KMIME_EXPORT;

/**
 * Uses current time, pid and random numbers to construct a string
 * that aims to be unique on a per-host basis (ie. for the local
 * part of a message-id or for multipart boundaries.
 *
 * @return the unique string.
 * @see multiPartBoundary
 */
extern QByteArray uniqueString() KMIME_EXPORT;

/**
 * Constructs a random string (sans leading/trailing "--") that can
 * be used as a multipart delimiter (ie. as @p boundary parameter
 * to a multipart/... content-type).
 *
 * @return the randomized string.
 * @see uniqueString
 */
extern QByteArray multiPartBoundary() KMIME_EXPORT;

/**
 * Unfolds the given header if necessary.
 * @param header The header to unfold.
 */
extern QByteArray unfoldHeader( const QByteArray &header ) KMIME_EXPORT;

/**
 * Tries to extract the header with name @p name from the string
 * @p src, unfolding it if necessary.
 *
 * @param src  the source string.
 * @param name the name of the header to search for.
 *
 * @return the first instance of the header @p name in @p src
 *         or a null QCString if no such header was found.
 */
extern QByteArray extractHeader( const QByteArray &src,
                                 const QByteArray &name ) KMIME_EXPORT;

/**
 * Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).
 *
 * This function is expensive and should be used only if the mail
 * will be stored locally. All decode functions can cope with both
 * line endings.
 *
 * @param s source string containing CRLF's
 *
 * @return the string with CRLF's substitued for LF's
 * @see CRLFtoLF(const char*) LFtoCRLF
 */
extern QByteArray CRLFtoLF( const QByteArray &s ) KMIME_EXPORT;

/**
 * Converts all occurrences of "\r\n" (CRLF) in @p s to "\n" (LF).
 *
 * This function is expensive and should be used only if the mail
 * will be stored locally. All decode functions can cope with both
 * line endings.
 *
 * @param s source string containing CRLF's
 *
 * @return the string with CRLF's substitued for LF's
 * @see CRLFtoLF(const QCString&) LFtoCRLF
 */
extern QByteArray CRLFtoLF( const char *s ) KMIME_EXPORT;

/**
 * Converts all occurrences of "\n" (LF) in @p s to "\r\n" (CRLF).
 *
 * This function is expensive and should be used only if the mail
 * will be transmitted as an RFC822 message later. All decode
 * functions can cope with and all encode functions can optionally
 * produce both line endings, which is much faster.
 *
 * @param s source string containing CRLF's
 *
 * @return the string with CRLF's substitued for LF's
 * @see CRLFtoLF(const QCString&) LFtoCRLF
 */
extern QByteArray LFtoCRLF( const QByteArray &s ) KMIME_EXPORT;

/**
 * Removes quote (DQUOTE) characters and decodes "quoted-pairs"
 * (ie. backslash-escaped characters)
 *
 * @param str the string to work on.
 * @see addQuotes
 */
KMIME_EXPORT extern void removeQuots( QByteArray &str );

/**
 * Removes quote (DQUOTE) characters and decodes "quoted-pairs"
 * (ie. backslash-escaped characters)
 *
 * @param str the string to work on.
 * @see addQuotes
 */
KMIME_EXPORT extern void removeQuots( QString &str );

/**
 * Converts the given string into a quoted-string if the string contains
 * any special characters (ie. one of ()<>@,.;:[]=\").
 *
 * @param str us-ascii string to work on.
 * @param forceQuotes if @p true, always add quote characters.
 */
KMIME_EXPORT extern void addQuotes( QByteArray &str, bool forceQuotes );

/**
 * @short class abstracting date formatting
 *
 * DateFormatter deals with different kinds of date
 * display formats. The formats supported by the class include:
 * <ul>
 *     <li> fancy "Today 02:08:35"
 *     <li> ctime "Sun Mar 31 02:08:35 2002"
 *     <li> localized "2002-03-31 02:08"
 *     <li> iso  "2002-03-31 02:08:35"
 *     <li> rfc2822 "Sun, 31 Mar 2002 02:08:35 -0500"
 *     <li> custom "whatever you like"
 * </ul>
 *
 */
class KMIME_EXPORT DateFormatter
{
  public:
    enum FormatType {
      CTime,      //< ctime "Sun Mar 31 02:08:35 2002"
      Localized,  //< localized "2002-03-31 02:08"
      Fancy,      //< fancy "Today 02:08:35"
      Iso,        //< iso  "2002-03-31 02:08:35"
      Custom      //< custom "whatever you like"
    };

    /**
     * Constructor
     *
     * @param fType default format used by the class
     */
    DateFormatter( FormatType fType=DateFormatter::Fancy );

    ~DateFormatter();

    /**
     * Returns the currently set format
     */
    FormatType getFormat() const;

    /**
     * Sets the currently used format
     *
     * @param t is the FormatType to set.
    */
    void setFormat( FormatType t );

    /**
     * Returns formatted date string in a currently set format.
     *
     * @param otime time to format
     * @param lang used <em>only</em> by the Localized format,
     *             sets the used language
     * @param shortFormat used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     * @param includeSecs used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     */
    QString dateString( time_t otime, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
     * Overloaded, does exactly what #dateString does (it's slower)
     */
    QString dateString( const QDateTime &dtime, const QString &lang=QString(),
                        bool shortFormat=true, bool includeSecs=false ) const;

    /**
     * Makes the class use the custom format for date to string conversions.
     * Method accepts the same arguments as QDateTime::toString method and adds
     * "Z" expression which is substituted with the RFC-822 style numeric
     * timezone (-0500).
     *
     * @param format the custom format
     */
    void setCustomFormat( const QString &format );
    QString getCustomFormat() const;

    /**
     * Returns rfc2822 formatted string.
     *
     * @param otime time to use for formatting
     */
    QByteArray rfc2822( time_t otime ) const;

    /**
     * Resets the internal clock.
     */
    void reset();

    //statics
    /**
     * Convenience function dateString
     *
     * @param t specifies the FormatType to use
     * @param time time to format
     * @param data is either the format when FormatType is Custom,
     *             or language when FormatType is Localized
     * @param shortFormat used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     * @param includeSecs used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     */
    static QString  formatDate( DateFormatter::FormatType t, time_t time,
                                const QString &data=QString(),
                                bool shortFormat=true,
                                bool includeSecs=false );

    /**
     * Convenience function, same as #formatDate but returns the current time
     * formatted.
     *
     * @param t specifies the FormatType to use
     * @param data is either the format when FormatType is Custom,
     *             or language when FormatType is Localized
     * @param shortFormat used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     * @param includeSecs used <em>only</em> by the Localized format,
     *             is passed to KLocale::formatDateTime
     */
    static QString  formatCurrentDate( DateFormatter::FormatType t,
                                       const QString &data=QString(),
                                       bool shortFormat=true,
                                       bool includeSecs=false );

    /**
      Convenience function, same as #rfc2822
    */
    static QByteArray rfc2822FormatDate( time_t time );
    static bool isDaylight();

  protected:
    /**
     * Returns fancy formatted date string.
     *
     * @param otime time to format
     * @internal
     */
    QString fancy( time_t otime ) const ;

    /**
     * Returns localized formatted date string.
     *
     * @param otime time to format
     * @param shortFormat
     * @param includeSecs
     * @param localeLanguage language used for formatting
     * @internal
     */
    QString localized( time_t otime, bool shortFormat=true,
                       bool includeSecs=false,
                       const QString &localeLanguage=QString() ) const;

    /**
     * Returns string as formatted with ctime function.
     *
     * @param otime time to format
     * @internal
     */
    QString cTime( time_t otime ) const;

    /**
     * Returns a string in the "%Y-%m-%d %H:%M:%S" format.
     *
     * @param otime time to format
     * @internal
     */
    QString isoDate( time_t otime ) const;

    /**
     * Returns date formatted with the earlier given custom format.
     *
     * @param t time used for formatting
     * @internal
     */
    QString custom( time_t t ) const;

    /**
     * Returns a string identifying the timezone (eg."-0500")
     *
     * @param otime time to compute timezone from.
     * @internal
     */
    QByteArray zone( time_t otime ) const;

    time_t qdateToTimeT( const QDateTime &dt ) const;

  private:
    FormatType          mFormat;
    mutable time_t      mCurrentTime;
    mutable QDateTime   mDate;
    QString             mCustomFormat;
    static int          mDaylight;
};

} // namespace KMime

#endif /* __KMIME_UTIL_H__ */
