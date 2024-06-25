/*  -*- c++ -*-
    charfreq.h

    KMime, the KDE Internet mail/usenet news message library.
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the CharFreq class.

  @brief
  Defines the CharFreq class.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor Eight-Bit @anchor eight-bit @b 8-bit:
  Data that contains bytes with at least one value greater than 127, or at
  least one NUL byte.

  @glossary @anchor Eight-Bit-Binary @anchor eight-bit-binary @b 8-bit-binary:
  Eight-bit data that contains a high percentage of non-ascii values,
  or lines longer than 998 characters, or stray CRs, or NULs.

  @glossary @anchor Eight-Bit-Text @anchor eight-bit-text @b 8-bit-text:
  Eight-bit data that contains a high percentage of ascii values,
  no lines longer than 998 characters, no NULs, and either only LFs or
  only CRLFs.

  @glossary @anchor Seven-Bit @anchor seven-bit @b 7-Bit:
  Data that contains bytes with all values less than 128, and no NULs.

  @glossary @anchor Seven-Bit-Binary @anchor seven-bit-binary @b 7-bit-binary:
  Seven-bit data that contains a high percentage of non-ascii values,
  or lines longer than 998 characters, or stray CRs.

  @glossary @anchor Seven-Bit-Text @anchor seven-bit-text @b 7-bit-text:
  Seven-bit data that contains a high percentage of ascii values,
  no lines longer than 998 characters, and either only LFs, or only CRLFs.
*/

#pragma once

#include <QByteArray>
#undef None

#include <limits>

namespace KMime
{

/**
  @brief
  A class for performing basic data typing using frequency count heuristics.

  This class performs character frequency counts on the provided data which
  are used in heuristics to determine a basic data type.  The data types are:

  - @ref Eight-Bit-Binary
  - @ref Eight-Bit-Text
  - @ref Seven-Bit-Binary
  - @ref Seven-Bit-Text
*/
class CharFreq
{
public:
    /**
      Constructs a Character Frequency instance for a buffer @p buf of
      QByteArray data.

      @param buf is a QByteArray containing the data.
    */
    explicit CharFreq(QByteArrayView buf);

    /**
      The different types of data.
    */
    enum Type {
        None = 0,              /**< Unknown */
        EightBitData,          /**< 8bit binary */
        Binary = EightBitData, /**< 8bit binary */
        SevenBitData,          /**< 7bit binary */
        EightBitText,          /**< 8bit text */
        SevenBitText           /**< 7bit text */
    };

    /**
      Returns the data #Type as derived from the class heuristics.
    */
    [[nodiscard]] Type type() const;

    /**
      Returns true if the data #Type is EightBitData; false otherwise.
    */
    [[nodiscard]] bool isEightBitData() const;

    /**
      Returns true if the data #Type is EightBitText; false otherwise.
    */
    [[nodiscard]] bool isEightBitText() const;

    /**
      Returns true if the data #Type is SevenBitData; false otherwise.
    */
    [[nodiscard]] bool isSevenBitData() const;

    /**
      Returns true if the data #Type is SevenBitText; false otherwise.
    */
    [[nodiscard]] bool isSevenBitText() const;

    /**
      Returns true if the data contains trailing whitespace. i.e.,
      if any line ends with space (' ') or tab ('\\t').
    */
    [[nodiscard]] bool hasTrailingWhitespace() const;

    /**
      Returns true if the data contains a line that starts with "From ".
    */
    [[nodiscard]] bool hasLeadingFrom() const;

    /**
      Returns the percentage of printable characters in the data.
      The result is undefined if the number of data characters is zero.
    */
    [[nodiscard]] float printableRatio() const;

    /**
      Returns the percentage of control code characters (CTLs) in the data.
      The result is undefined if the number of data characters is zero.
    */
    [[nodiscard]] float controlCodesRatio() const;

  private:
    uint mNUL = 0;       // count of NUL chars
    uint mCTL = 0;       // count of CTLs (incl. DEL, excl. CR, LF, HT)
    uint mCR = 0;        // count of CR chars
    uint mLF = 0;        // count of LF chars
    uint mCRLF = 0;      // count of LFs, preceded by CRs
    uint mPrintable = 0; // count of printable US-ASCII chars (SPC..~)
    uint mEightBit = 0;  // count of other latin1 chars (those with 8th bit set)
    uint mTotal = 0;     // count of all chars
    uint mLineMin = std::numeric_limits<uint>::max(); // minimum line length
    uint mLineMax = 0;   // maximum line length
    bool mTrailingWS = false;  // does the buffer contain trailing whitespace?
    bool mLeadingFrom = false; // does the buffer contain lines starting with "From "?

    /**
      Performs the character frequency counts on the data.

      @param buf is a pointer to a character string containing the data.
      @param len is the length of @p buf, in characters.
    */
    void count(const char *buf, size_t len);
};

} // namespace KMime

