/*
    SPDX-FileCopyrightText: 2006-2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kmime_export.h"

#include <QSharedDataPointer>
#include <QString>
#include <QMetaType>

namespace KMime
{

/*!
  \class KMime::ContentIndex
  \inmodule KMime
  \inheaderfile KMime/ContentIndex

  \brief A class to uniquely identify message parts (Content) in a hierarchy.

  This class is implicitly shared.

  Based on RFC3501 section 6.4.5 and thus compatible with IMAP.
*/
class KMIME_EXPORT ContentIndex
{
public:
    /*!
      Creates an empty content index.
    */
    ContentIndex();

    /*!
      Creates a content index based on the specified string representation.

      \a index is a string representation of a message part index according
      to RFC3501 section 6.4.5.
    */
    explicit ContentIndex(QStringView index);

    ContentIndex(const ContentIndex &other);
    ContentIndex(ContentIndex &&) noexcept;

    ~ContentIndex();

    /*!
      Returns \c true if this index is non-empty (valid).
    */
    [[nodiscard]] bool isValid() const;

    /*!
      Removes and returns the top-most index. Used to recursively
      descend into the message part hierarchy.

      \sa push(), up().
    */
    [[nodiscard]] unsigned int pop();

    /*!
      Adds \a index to the content index. Used when ascending the message
      part hierarchy.

      \a index is the top-most content index part.

      \sa pop(), up().
    */
    void push(unsigned int index);

    /*!
      Removes and returns the bottom-most index.

      Used to navigate to the parent part.

      \sa push(), pop().
    */
    unsigned int up();

    /*!
      Returns a string representation of this content index according
      to RFC3501 section 6.4.5.
    */
    [[nodiscard]] QString toString() const;

    /*!
      Compares this with \a index for equality.

      \a index is the content index to compare.
    */
    [[nodiscard]] bool operator==(const ContentIndex &index) const;

    [[nodiscard]] bool operator!=(const ContentIndex &index) const;

    ContentIndex &operator=(const ContentIndex &other);
    ContentIndex &operator=(ContentIndex &&) noexcept;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

KMIME_EXPORT size_t qHash(const KMime::ContentIndex &, size_t seed = 0) noexcept;

}  //namespace KMime

Q_DECLARE_METATYPE(KMime::ContentIndex)
Q_DECLARE_TYPEINFO(KMime::ContentIndex, Q_RELOCATABLE_TYPE);
