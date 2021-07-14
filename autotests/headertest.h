/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class HeaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIdentHeader();
    void testAddressListHeader();
    void testMailboxListHeader();
    void testSingleMailboxHeader();
    void testMailCopiesToHeader();
    void testParametrizedHeader();
    void testContentDispositionHeader();
    void testContentTypeHeader();
    void testTokenHeader();
    void testContentTransferEncoding();
    void testPhraseListHeader();
    void testDotAtomHeader();
    void testDateHeader();
    void testLinesHeader();
    void testNewsgroupsHeader();
    void testControlHeader();
    void testReturnPath();
    void testInvalidButOkQEncoding();
    void testInvalidQEncoding();
    void testInvalidQEncoding_data();
    void testBug271192();
    void testBug271192_data();
    void testMissingQuotes();

    // makes sure we don't accidentally have an abstract header class that's not
    // meant to be abstract
    void noAbstractHeaders();
};

