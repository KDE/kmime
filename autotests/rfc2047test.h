/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RFC2047TEST_H
#define RFC2047TEST_H

#include <QObject>

class RFC2047Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRFC2047encode();
};

#endif

