/*
    SPDX-FileCopyrightText: 2006 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class ContentIndexTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testToString();
    void testFromString();
    void testContent();
    void testNavigation();
};

