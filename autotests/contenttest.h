/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QObject>

class ContentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testGetHeaderInstance();
    void testHeaderAddRemove();
    void testHeaderAppend();
    void testImplicitMultipartGeneration();
    void testExplicitMultipartGeneration();
    void testSetContent();
    void testEncodedContent();
    void testDecodedContent();
    void testMultipartMixed();
    void testMultipleHeaderExtraction();
    /**
      Tests that a message with uuencoded content
      is parsed correctly and if a corresponding
      MIME structure is created.
    */
    void testParsingUuencoded();
    // TODO: grab samples from http://www.yenc.org/develop.htm and make a Yenc test
    void testParent();
    void testFreezing();
    void testContentTypeMimetype_data();
    void testContentTypeMimetype();
};

