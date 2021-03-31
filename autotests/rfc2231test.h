/*
    SPDX-FileCopyrightText: 2010 Leo Franchi <lfranchi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QObject>

class RFC2231Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRFC2231decode();
    void testInvalidDecode();
    void testRFC2231encode();
};


