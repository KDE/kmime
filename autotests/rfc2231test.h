/*
    SPDX-FileCopyrightText: 2010 Leo Franchi <lfranchi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef RFC2231TEST_H
#define RFC2231TEST_H

#include <QObject>

class RFC2231Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRFC2231decode();
    void testInvalidDecode();
    void testRFC2231encode();
};

#endif

