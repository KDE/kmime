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

#ifndef KMIME_MESSAGE_P_H
#define KMIME_MESSAGE_P_H

#include "kmime_content_p.h"

// @cond PRIVATE

namespace KMime {

class MessagePrivate : public ContentPrivate
{
  public:
    MessagePrivate( Message *q ) : ContentPrivate( q )
    {
      subject.setParent( q );
      date.setParent( q );
    }

    KMime::Headers::Subject subject;
    KMime::Headers::Date date;

    Q_DECLARE_PUBLIC(Message)
};

}

// @endcond

#endif