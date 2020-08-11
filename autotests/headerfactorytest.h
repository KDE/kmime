/*
    SPDX-FileCopyrightText: 2009 Constantin Berzan <exit3219@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HEADERFACTORYTEST_H
#define HEADERFACTORYTEST_H

#include <QObject>

class HeaderFactoryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testBuiltInHeaders();
};

#endif
