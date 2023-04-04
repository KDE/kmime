/*
    SPDX-FileCopyrightText: 2006-2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the ContentIndex class.

  @brief
  Defines the ContentIndex class.

  @authors Volker Krause \<vkrause@kde.org\>
*/

#include "kmime_contentindex.h"

#include <QHash>
#include <QList>
#include <QSharedData>
#include <QStringList>

using namespace KMime;

class ContentIndex::Private : public QSharedData
{
public:
  Private() = default;
  Private(const Private &other) : QSharedData(other) { index = other.index; }

  QList<unsigned int> index;
};

KMime::ContentIndex::ContentIndex() : d(new Private)
{
}

KMime::ContentIndex::ContentIndex(const QString &index) : d(new Private)
{
    const QStringList l = index.split(QLatin1Char('.'));
    for (const QString &s : l) {
        bool ok;
        unsigned int i = s.toUInt(&ok);
        if (!ok) {
            d->index.clear();
            break;
        }
        d->index.append(i);
    }
}

ContentIndex::ContentIndex(const ContentIndex &other) = default;

ContentIndex::~ContentIndex() = default;

bool KMime::ContentIndex::isValid() const
{
    return !d->index.isEmpty();
}

unsigned int KMime::ContentIndex::pop()
{
    return d->index.takeFirst();
}

void KMime::ContentIndex::push(unsigned int index)
{
    d->index.prepend(index);
}

unsigned int ContentIndex::up()
{
    return d->index.takeLast();
}

QString KMime::ContentIndex::toString() const
{
    QStringList l;
    l.reserve(d->index.count());
    for (unsigned int i : std::as_const(d->index)) {
        l.append(QString::number(i));
    }
    return l.join(QLatin1Char('.'));
}

bool KMime::ContentIndex::operator ==(const ContentIndex &index) const
{
    return d->index == index.d->index;
}

bool KMime::ContentIndex::operator !=(const ContentIndex &index) const
{
    return d->index != index.d->index;
}

ContentIndex &ContentIndex::operator =(const ContentIndex &other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

uint KMime::qHash(const KMime::ContentIndex &index)
{
    return qHash(index.toString());
}
