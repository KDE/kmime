/*
    kmime_header_factory.h

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the HeaderFactory class.

  @brief
  Defines the HeaderFactory class.

  @authors Constantin Berzan \<exit3219@gmail.com\>
*/

#ifndef __KMIME_HEADERFACTORY_H__
#define __KMIME_HEADERFACTORY_H__

#include "kmime_export.h"

#include <QByteArray>

namespace KMime
{

namespace Headers
{
class Base;
}

namespace HeaderFactory
{
    Headers::Base *createHeader(const char *type, size_t typeLen);
    inline Headers::Base *createHeader(const QByteArray &type)
    {
        return createHeader(type.constData(), type.size());
    }

}

} // namespace KMime

#endif /* __KMIME_HEADERFACTORY_H__ */
