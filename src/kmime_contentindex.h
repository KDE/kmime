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

  @glossary @anchor RFC3501 @anchor rfc3501 @b RFC @b 3501:
  RFC that defines the <a href="https://tools.ietf.org/html/rfc3501">
  Internet Message Access Protocol (IMAP)</a>.
*/

#pragma once

#include "kmime_export.h"

#include <QSharedDataPointer>
#include <QString>
#include <QMetaType>

namespace KMime
{

/**
  @brief
  A class to uniquely identify message parts (Content) in a hierarchy.

  This class is implicitly shared.

  Based on @ref RFC3501 section 6.4.5 and thus compatible with @acronym IMAP.
*/
class KMIME_EXPORT ContentIndex
{
public:
    /**
      Creates an empty content index.
    */
    ContentIndex();

    /**
      Creates a content index based on the specified string representation.

      @param index is a string representation of a message part index according
      to @ref RFC3501 section 6.4.5.
    */
    explicit ContentIndex(QStringView index);

    /**
      Copy constructor.
    */
    ContentIndex(const ContentIndex &other);
    ContentIndex(ContentIndex &&) noexcept;

    /**
      Destructor.
    */
    ~ContentIndex();

    /**
      Returns true if this index is non-empty (valid).
    */
    [[nodiscard]] bool isValid() const;

    /**
      Removes and returns the top-most index. Used to recursively
      descend into the message part hierarchy.

      @see push(), up().
    */
    [[nodiscard]] unsigned int pop();

    /**
      Adds @p index to the content index. Used when ascending the message
      part hierarchy.

      @param index is the top-most content index part.

      @see pop(), up().
    */
    void push(unsigned int index);

    /**
      Removes and returns the bottom-most index. Used to navigate to
      the parent part.

      @see push(), pop().
    */
    unsigned int up();

    /**
      Returns a string representation of this content index according
      to @ref RFC3501 section 6.4.5.
    */
    [[nodiscard]] QString toString() const;

    /**
      Compares this with @p index for equality.

      @param index is the content index to compare.
    */
    [[nodiscard]] bool operator==(const ContentIndex &index) const;

    /**
      Compares this with @p index for inequality.

      @param index is the content index to compare.
    */
    [[nodiscard]] bool operator!=(const ContentIndex &index) const;

    /**
      Assignment operator.
    */
    ContentIndex &operator=(const ContentIndex &other);
    ContentIndex &operator=(ContentIndex &&) noexcept;

private:
    //@cond PRIVATE
    class Private;
    QSharedDataPointer<Private> d;
    //@endcond
};

KMIME_EXPORT uint qHash(const KMime::ContentIndex &);

}  //namespace KMime

Q_DECLARE_METATYPE(KMime::ContentIndex)

