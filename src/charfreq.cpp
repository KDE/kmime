/*
  kmime_charfreq.cpp

  KMime, the KDE Internet mail/usenet news message library.
  SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

/**
  @file
  This file is part of the API for handling MIME data and
  defines the CharFreq class.

  @brief
  Defines the CharFreq class.

  @authors Marc Mutz \<mutz@kde.org\>
*/

#include "charfreq_p.h"

using namespace KMime;

CharFreq::CharFreq(QByteArrayView buf)
    : mNUL(0),
      mCTL(0),
      mCR(0), mLF(0),
      mCRLF(0),
      mPrintable(0),
      mEightBit(0),
      mTotal(0),
      mLineMin(0xffffffff),
      mLineMax(0)
{
    if (!buf.isEmpty()) {
        count(buf.data(), buf.size());
    }
}

static inline bool isWS(char ch)
{
    return (ch == '\t' || ch == ' ');
}

void CharFreq::count(const char *it, size_t len)
{
    const char *end = it + len;
    uint currentLineLength = 0;
    // initialize the prevChar with LF so that From_ detection works w/o
    // special-casing:
    char prevChar = '\n';
    char prevPrevChar = 0;

    for (; it != end ; ++it) {
        ++currentLineLength;
        switch (*it) {
        case '\0': ++mNUL; break;
        case '\r': ++mCR;  break;
        case '\n': ++mLF;
            if (prevChar == '\r') {
                --currentLineLength; ++mCRLF;
            }
            if (currentLineLength >= mLineMax) {
                mLineMax = currentLineLength - 1;
            }
            if (currentLineLength <= mLineMin) {
                mLineMin = currentLineLength - 1;
            }
            if (!mTrailingWS) {
                if (isWS(prevChar) ||
                        (prevChar == '\r' && isWS(prevPrevChar))) {
                    mTrailingWS = true;
                }
            }
            currentLineLength = 0;
            break;
        case 'F': // check for lines starting with From_ if not found already:
            if (!mLeadingFrom) {
                if (prevChar == '\n' && end - it >= 5 &&
                        !qstrncmp("From ", it, 5)) {
                    mLeadingFrom = true;
                }
            }
            ++mPrintable;
            break;
        default: {
            uchar c = *it;
            if (c == '\t' || (c >= ' ' && c <= '~')) {
                ++mPrintable;
            } else if (c == 127 || c < ' ') {
                ++mCTL;
            } else {
                ++mEightBit;
            }
        }
        }
        prevPrevChar = prevChar;
        prevChar = *it;
    }

    // consider the length of the last line
    if (currentLineLength >= mLineMax) {
        mLineMax = currentLineLength;
    }
    if (currentLineLength <= mLineMin) {
        mLineMin = currentLineLength;
    }

    // check whether the last character is tab or space
    if (isWS(prevChar)) {
        mTrailingWS = true;
    }

    mTotal = len;
}

bool CharFreq::isEightBitData() const
{
    return type() == EightBitData;
}

bool CharFreq::isEightBitText() const
{
    return type() == EightBitText;
}

bool CharFreq::isSevenBitData() const
{
    return type() == SevenBitData;
}

bool CharFreq::isSevenBitText() const
{
    return type() == SevenBitText;
}

bool CharFreq::hasTrailingWhitespace() const
{
    return mTrailingWS;
}

bool CharFreq::hasLeadingFrom() const
{
    return mLeadingFrom;
}

CharFreq::Type CharFreq::type() const
{
#if 0
    qCDebug(KMIME_LOG)("Total: %d; NUL: %d; CTL: %d;\n"
           "CR: %d; LF: %d; CRLF: %d;\n"
           "lineMin: %d; lineMax: %d;\n"
           "printable: %d; eightBit: %d;\n"
           "trailing whitespace: %s;\n"
           "leading 'From ': %s;\n",
           total, NUL, CTL, CR, LF, CRLF, lineMin, lineMax,
           printable, eightBit,
           mTrailingWS ? "yes" : "no" , mLeadingFrom ? "yes" : "no");
#endif
    if (mNUL) {   // must be binary
        return Binary;
    }

    // doesn't contain NUL's:
    if (mEightBit) {
        if (mLineMax > 988) {
            return EightBitData; // not allowed in 8bit
        }
        if ((mLF != mCRLF && mCRLF > 0) || mCR != mCRLF || controlCodesRatio() > 0.2) {
            return EightBitData;
        }
        return EightBitText;
    }

    // doesn't contain NUL's, nor 8bit chars:
    if (mLineMax > 988) {
        return SevenBitData;
    }
    if ((mLF != mCRLF && mCRLF > 0) || mCR != mCRLF || controlCodesRatio() > 0.2) {
        return SevenBitData;
    }

    // no NUL, no 8bit chars, no excessive CTLs and no lines > 998 chars:
    return SevenBitText;
}

float CharFreq::printableRatio() const
{
    if (mTotal) {
        return float(mPrintable) / float(mTotal);
    } else {
        return 0;
    }
}

float CharFreq::controlCodesRatio() const
{
    if (mTotal) {
        return float(mCTL) / float(mTotal);
    } else {
        return 0;
    }
}

