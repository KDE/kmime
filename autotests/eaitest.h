/*
    SPDX-FileCopyrightText: 2026 Arnt Gulbrandsen

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* This file is based on https://github.com/arnt/eai-test-messages,
   just mapped to C++. I really don't think there's enough creativity
   here to claim any rights. The SPDX comment is purely formal.
*/

#pragma once

#include "message.h"
#include <QObject>

class EaiTest : public QObject {
  Q_OBJECT
private Q_SLOTS:
  void testAddresses();
  void testFrom();
  void testMimefield();
  void testNotEmoji();
  void testPunycode();
  void testAttachment();
  void testUtf8Domain();

private:
  std::unique_ptr<KMime::Message>
  readAndParseMail(const QString &mailFile) const;
};
