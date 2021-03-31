/*
    SPDX-FileCopyrightText: 2006-2015 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class AttachmentTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIsAttachment_data();
    void testIsAttachment();
    void testIsAttachmentSpecial();
    void testHasAttachment();
    void testNestedMultipart();
    void testEncrypted();
    void testAttachment1();
    void testAttachment2();
};


