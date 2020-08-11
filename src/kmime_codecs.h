/*  -*- c++ -*-
    kmime_codecs.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001 the KMime authors.
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __KMIME_CODECS_H__
#define __KMIME_CODECS_H__

#include <QByteArray>
#include <QString>

namespace KMime
{

/**
  Encodes string @p src according to RFC2047 using charset @p charset.

  This function also makes commas, quotes and other characters part of the encoded name, for example
  the string "Jöhn Döe" <john@example.com"> would be encoded as <encoded word for "Jöhn Döe"> <john@example.com>,
  i.e. the opening and closing quote mark would be part of the encoded word.
  Therefore don't use this function for input strings that contain semantically meaningful characters,
  like the quoting marks in this example.

  @param src           source string.
  @param charset       charset to use. If it can't encode the string, UTF-8 will be used instead.
  @param addressHeader if this flag is true, all special chars
                       like <,>,[,],... will be encoded, too.
  @param allow8bitHeaders if this flag is true, 8Bit headers are allowed.

  @return the encoded string.
*/
Q_REQUIRED_RESULT QByteArray encodeRFC2047String(const QString &src, const QByteArray &charset, bool addressHeader = false, bool allow8bitHeaders = false);

/**
 * Same as encodeRFC2047String(), but with a crucial difference: Instead of encoding the complete
 * string as a single encoded word, the string will be split up at control characters, and only parts of
 * the sentence that really need to be encoded will be encoded.
 */
Q_REQUIRED_RESULT QByteArray encodeRFC2047Sentence(const QString &src, const QByteArray &charset);

/**
  Decodes string @p src according to RFC2231

  @param src       source string.
  @param usedCs    the detected charset is returned here
  @param defaultCS the charset to use in case the detected
                   one isn't known to us.
  @param forceCS   force the use of the default charset.

  @return the decoded string.
*/
Q_REQUIRED_RESULT QString decodeRFC2231String(const QByteArray &src, QByteArray &usedCS, const QByteArray &defaultCS = QByteArray(), bool forceCS = false);

/**
  Encodes string @p src according to RFC2231 using charset @p charset.

  @param src           source string.
  @param charset       charset to use.
  @return the encoded string.
*/
Q_REQUIRED_RESULT QByteArray encodeRFC2231String(const QString &src, const QByteArray &charset);

} // namespace KMime

#endif /* __KMIME_CODEC_H__ */
