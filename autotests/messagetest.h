/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "message.h"
#include <QObject>

class MessageTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testMainBodyPart();
    void testBrunosMultiAssembleBug();
    void testWillsAndTillsCrash();
    void testDavidsParseCrash();
    void testHeaderFieldWithoutSpace();
    void testWronglyFoldedHeaders();
    void missingHeadersTest();
    void testBug219749();
    void testBidiSpoofing();
    void testUtf16();
    void testDecodedText();
    void testInlineImages();
    void testIssue3908();
    void testIssue3914();
    void testBug223509();
    void testEncapsulatedMessages();
    void testOutlookAttachmentNaming();
    void testEncryptedMails();
    void testReturnSameMail();
    void testEmptySubject();
    void testReplyHeader();

    void testBug392239();
    void testBugAttachment387423();
    void testCrashReplyInvalidEmail();
    void testHeadersWithNullBytes();
    void testMultipartParseAbort();
    void testUninitializedMemoryUse();
    void testParseDigitsOverflow();
    void testBigAllocation();
private:
    KMime::Message::Ptr readAndParseMail(const QString &mailFile) const;
};
