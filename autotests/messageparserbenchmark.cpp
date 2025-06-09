/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KMime/Message>

#include <QFile>
#include <QTest>
using namespace Qt::Literals;

class MessageParserBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPlainTextParse()
    {
        QFile file(QLatin1StringView(TEST_DATA_DIR "/plain-text-body.mbox"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray data = KMime::CRLFtoLF(file.readAll());

        QBENCHMARK {
            auto msg = std::make_unique<KMime::Message>();
            msg->setContent(data);
            msg->parse();
            QVERIFY(msg->subject(false));
            QCOMPARE(msg->subject()->asUnicodeString(), "New Project: Kool Desktop Environment (KDE)"_L1);
            QVERIFY(msg->decodedText().contains("Kool Desktop Environment"_L1));
        }
    }
};

QTEST_GUILESS_MAIN(MessageParserBenchmark)

#include "messageparserbenchmark.moc"
