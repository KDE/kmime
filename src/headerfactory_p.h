/*
    kmime_header_factory.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QByteArrayView>

#include <memory>

namespace KMime
{

namespace Headers
{
class Base;
}

namespace HeaderFactory
{
    [[nodiscard]] std::unique_ptr<Headers::Base> createHeader(QByteArrayView type);
    [[nodiscard]] std::unique_ptr<Headers::Base> clone(const Headers::Base *header);
}

} // namespace KMime

