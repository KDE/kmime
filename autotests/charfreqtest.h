/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CHARSET_TEST_H
#define CHARSET_TEST_H

#include <QObject>

class CharFreqTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test8bitData();
    void test8bitText();
    void test7bitData();
    void test7bitText();
    void testTrailingWhitespace();
    void testLeadingFrom();
};

#endif
