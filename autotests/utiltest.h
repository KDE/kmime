/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class UtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testUnfoldHeader();
    void testFoldHeader();
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


