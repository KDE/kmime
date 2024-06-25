/*
    warning.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>
    See file AUTHORS for details

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMIME_WARNING_H
#define KMIME_WARNING_H

#ifndef KMIME_NO_WARNING
#  include "kmime_debug.h"
#  define KMIME_WARN qCDebug(KMIME_LOG) << "Tokenizer Warning:"
#  define KMIME_WARN_UNKNOWN(x,y) KMIME_WARN << "unknown " #x ": \"" \
        << y << "\"";
#  define KMIME_WARN_UNKNOWN_ENCODING KMIME_WARN << "unknown encoding in " \
        "RFC 2047 encoded-word (only know 'q' and 'b')";
#  define KMIME_WARN_UNKNOWN_CHARSET(c) KMIME_WARN << "unknown charset \"" \
        << c << "\" in RFC 2047 encoded-word";
#  define KMIME_WARN_8BIT(ch) KMIME_WARN \
            << "8Bit character '" << ch << "'"
#  define KMIME_WARN_IF_8BIT(ch) if ( (unsigned char)(ch) > 127 ) \
    { KMIME_WARN_8BIT(ch); }
#  define KMIME_WARN_PREMATURE_END_OF(x) KMIME_WARN \
            << "Premature end of " #x
#  define KMIME_WARN_LONE(x) KMIME_WARN << "Lonely " #x " character"
#  define KMIME_WARN_NON_FOLDING(x) KMIME_WARN << "Non-folding " #x
#  define KMIME_WARN_CTL_OUTSIDE_QS(x) KMIME_WARN << "Control character " \
        #x " outside quoted-string"
#  define KMIME_WARN_INVALID_X_IN_Y(X,Y) KMIME_WARN << "Invalid character '" \
        QString(QChar(X)) << "' in " #Y;
#  define KMIME_WARN_TOO_LONG(x) KMIME_WARN << #x \
        " too long or missing delimiter";
#else
#  define KMIME_NOP do {} while (0)
#  define KMIME_WARN_8BIT(ch) KMIME_NOP
#  define KMIME_WARN_IF_8BIT(ch) KMIME_NOP
#  define KMIME_WARN_PREMATURE_END_OF(x) KMIME_NOP
#  define KMIME_WARN_LONE(x) KMIME_NOP
#  define KMIME_WARN_NON_FOLDING(x) KMIME_NOP
#  define KMIME_WARN_CTL_OUTSIDE_QS(x) KMIME_NOP
#endif

#endif
