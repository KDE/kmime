/*
    kmime_header_factory.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the HeaderFactory class.

  @brief
  Defines the HeaderFactory class.

  @authors Constantin Berzan \<exit3219@gmail.com\>
*/

#pragma once

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

