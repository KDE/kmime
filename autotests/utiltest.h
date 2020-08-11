/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef UTIL_TEST_H
#define UTIL_TEST_H

#include <QObject>

class UtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUnfoldHeader();
    void testExtractHeader();
    void testBalanceBidiState();
    void testBalanceBidiState_data();
    void testAddQuotes();
    void testAddQuotes_data();
    void testIsSigned_data();
    void testIsSigned();
    void testIsCryptoPart_data();
    void testIsCryptoPart();
    void testLFCRLF_data();
    void testLFCRLF();
    void testLFCRLF_performance();
};

#endif

