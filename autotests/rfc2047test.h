/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class RFC2047Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRFC2047encode();
};


