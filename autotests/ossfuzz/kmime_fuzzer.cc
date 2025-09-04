/*
 * SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "message.h"

#include <QCoreApplication>
#include <QtEnvironmentVariables>

void traverseContent(KMime::Content *content) {
  for (KMime::Content *c : content->contents()) {
    auto decodedBody = c->decodedContent();
    auto decodedText = c->decodedText();

    for (const auto &header : c->headers()) {
      auto headerAs7BitString = header->as7BitString();
      auto headerAsUnicodeString = header->asUnicodeString();
    }

    traverseContent(c);
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  qputenv("QT_ENABLE_REGEXP_JIT", "1");

  int argc = 0;
  QCoreApplication a(argc, nullptr);

  QByteArray input((const char *)data, size);

  KMime::Message message;
  message.setContent(input);
  message.parse();

  traverseContent(&message);

  return 0;
}
