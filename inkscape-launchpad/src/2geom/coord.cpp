/** @file
 * @brief Conversion between Coord and strings
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 * 
 * Copyright 2014 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

// Most of the code in this file is derived from:
// https://code.google.com/p/double-conversion/
// The copyright notice for that code is attached below.
//
// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <2geom/coord.h>
#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <climits>
#include <cstdarg>
#include <cmath>

#ifndef ASSERT
#define ASSERT(condition)         \
    assert(condition);
#endif
#ifndef UNIMPLEMENTED
#define UNIMPLEMENTED() (abort())
#endif
#ifndef UNREACHABLE
#define UNREACHABLE()   (abort())
#endif

#define UINT64_2PART_C(a, b) (((static_cast<uint64_t>(a) << 32) + 0x##b##u))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                                   \
  ((sizeof(a) / sizeof(*(a))) /                         \
  static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)
#endif

#ifndef DISALLOW_IMPLICIT_CONSTRUCTORS
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_COPY_AND_ASSIGN(TypeName)
#endif

#if defined(__GNUC__)
#define DOUBLE_CONVERSION_UNUSED __attribute__((unused))
#else
#define DOUBLE_CONVERSION_UNUSED
#endif

namespace Geom {

namespace {

template <typename T>
class Vector {
 public:
  Vector() : start_(NULL), length_(0) {}
  Vector(T* data, int length) : start_(data), length_(length) {
    ASSERT(length == 0 || (length > 0 && data != NULL));
  }

  Vector<T> SubVector(int from, int to) {
    ASSERT(to <= length_);
    ASSERT(from < to);
    ASSERT(0 <= from);
    return Vector<T>(start() + from, to - from);
  }
  int length() const { return length_; }
  bool is_empty() const { return length_ == 0; }

  T* start() const { return start_; }

  T& operator[](int index) const {
    ASSERT(0 <= index && index < length_);
    return start_[index];
  }
  T& first() { return start_[0]; }
  T& last() { return start_[length_ - 1]; }

 private:
  T* start_;
  int length_;
};

template <class Dest, class Source>
inline Dest BitCast(const Source& source) {
  DOUBLE_CONVERSION_UNUSED
      typedef char VerifySizesAreEqual[sizeof(Dest) == sizeof(Source) ? 1 : -1];
  Dest dest;
  memmove(&dest, &source, sizeof(dest));
  return dest;
}

template <class Dest, class Source>
inline Dest BitCast(Source* source) {
  return BitCast<Dest>(reinterpret_cast<uintptr_t>(source));
}

// We assume that doubles and uint64_t have the same endianness.
static uint64_t double_to_uint64(double d) { return BitCast<uint64_t>(d); }
static double uint64_to_double(uint64_t d64) { return BitCast<double>(d64); }

// This "Do It Yourself Floating Point" class
class DiyFp {
 public:
  static const int kSignificandSize = 64;

  DiyFp() : f_(0), e_(0) {}
  DiyFp(uint64_t f, int e) : f_(f), e_(e) {}

  void Subtract(const DiyFp& other) {
    ASSERT(e_ == other.e_);
    ASSERT(f_ >= other.f_);
    f_ -= other.f_;
  }

  static DiyFp Minus(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Subtract(b);
    return result;
  }

  void Multiply(const DiyFp& other) {
      const uint64_t kM32 = 0xFFFFFFFFU;
      uint64_t a = f_ >> 32;
      uint64_t b = f_ & kM32;
      uint64_t c = other.f_ >> 32;
      uint64_t d = other.f_ & kM32;
      uint64_t ac = a * c;
      uint64_t bc = b * c;
      uint64_t ad = a * d;
      uint64_t bd = b * d;
      uint64_t tmp = (bd >> 32) + (ad & kM32) + (bc & kM32);
      // By adding 1U << 31 to tmp we round the final result.
      // Halfway cases will be round up.
      tmp += 1U << 31;
      uint64_t result_f = ac + (ad >> 32) + (bc >> 32) + (tmp >> 32);
      e_ += other.e_ + 64;
      f_ = result_f;
    }

  static DiyFp Times(const DiyFp& a, const DiyFp& b) {
    DiyFp result = a;
    result.Multiply(b);
    return result;
  }

  void Normalize() {
    ASSERT(f_ != 0);
    uint64_t f = f_;
    int e = e_;

    const uint64_t k10MSBits = UINT64_2PART_C(0xFFC00000, 00000000);
    while ((f & k10MSBits) == 0) {
      f <<= 10;
      e -= 10;
    }
    while ((f & kUint64MSB) == 0) {
      f <<= 1;
      e--;
    }
    f_ = f;
    e_ = e;
  }

  static DiyFp Normalize(const DiyFp& a) {
    DiyFp result = a;
    result.Normalize();
    return result;
  }

  uint64_t f() const { return f_; }
  int e() const { return e_; }

  void set_f(uint64_t new_value) { f_ = new_value; }
  void set_e(int new_value) { e_ = new_value; }

 private:
  static const uint64_t kUint64MSB = UINT64_2PART_C(0x80000000, 00000000);

  uint64_t f_;
  int e_;
};

class Double {
 public:
  static const uint64_t kSignMask = UINT64_2PART_C(0x80000000, 00000000);
  static const uint64_t kExponentMask = UINT64_2PART_C(0x7FF00000, 00000000);
  static const uint64_t kSignificandMask = UINT64_2PART_C(0x000FFFFF, FFFFFFFF);
  static const uint64_t kHiddenBit = UINT64_2PART_C(0x00100000, 00000000);
  static const int kPhysicalSignificandSize = 52;  // Excludes the hidden bit.
  static const int kSignificandSize = 53;

  Double() : d64_(0) {}
  explicit Double(double d) : d64_(double_to_uint64(d)) {}
  explicit Double(uint64_t d64) : d64_(d64) {}
  explicit Double(DiyFp diy_fp)
    : d64_(DiyFpToUint64(diy_fp)) {}

  DiyFp AsDiyFp() const {
    ASSERT(Sign() > 0);
    ASSERT(!IsSpecial());
    return DiyFp(Significand(), Exponent());
  }

  DiyFp AsNormalizedDiyFp() const {
    ASSERT(value() > 0.0);
    uint64_t f = Significand();
    int e = Exponent();

    // The current double could be a denormal.
    while ((f & kHiddenBit) == 0) {
      f <<= 1;
      e--;
    }
    // Do the final shifts in one go.
    f <<= DiyFp::kSignificandSize - kSignificandSize;
    e -= DiyFp::kSignificandSize - kSignificandSize;
    return DiyFp(f, e);
  }

  uint64_t AsUint64() const {
    return d64_;
  }

  double NextDouble() const {
    if (d64_ == kInfinity) return Double(kInfinity).value();
    if (Sign() < 0 && Significand() == 0) {
      // -0.0
      return 0.0;
    }
    if (Sign() < 0) {
      return Double(d64_ - 1).value();
    } else {
      return Double(d64_ + 1).value();
    }
  }

  double PreviousDouble() const {
    if (d64_ == (kInfinity | kSignMask)) return -Double::Infinity();
    if (Sign() < 0) {
      return Double(d64_ + 1).value();
    } else {
      if (Significand() == 0) return -0.0;
      return Double(d64_ - 1).value();
    }
  }

  int Exponent() const {
    if (IsDenormal()) return kDenormalExponent;

    uint64_t d64 = AsUint64();
    int biased_e =
        static_cast<int>((d64 & kExponentMask) >> kPhysicalSignificandSize);
    return biased_e - kExponentBias;
  }

  uint64_t Significand() const {
    uint64_t d64 = AsUint64();
    uint64_t significand = d64 & kSignificandMask;
    if (!IsDenormal()) {
      return significand + kHiddenBit;
    } else {
      return significand;
    }
  }

  bool IsDenormal() const {
    uint64_t d64 = AsUint64();
    return (d64 & kExponentMask) == 0;
  }

  // We consider denormals not to be special.
  // Hence only Infinity and NaN are special.
  bool IsSpecial() const {
    uint64_t d64 = AsUint64();
    return (d64 & kExponentMask) == kExponentMask;
  }

  bool IsNan() const {
    uint64_t d64 = AsUint64();
    return ((d64 & kExponentMask) == kExponentMask) &&
        ((d64 & kSignificandMask) != 0);
  }

  bool IsInfinite() const {
    uint64_t d64 = AsUint64();
    return ((d64 & kExponentMask) == kExponentMask) &&
        ((d64 & kSignificandMask) == 0);
  }

  int Sign() const {
    uint64_t d64 = AsUint64();
    return (d64 & kSignMask) == 0? 1: -1;
  }

  DiyFp UpperBoundary() const {
    ASSERT(Sign() > 0);
    return DiyFp(Significand() * 2 + 1, Exponent() - 1);
  }

  void NormalizedBoundaries(DiyFp* out_m_minus, DiyFp* out_m_plus) const {
    ASSERT(value() > 0.0);
    DiyFp v = this->AsDiyFp();
    DiyFp m_plus = DiyFp::Normalize(DiyFp((v.f() << 1) + 1, v.e() - 1));
    DiyFp m_minus;
    if (LowerBoundaryIsCloser()) {
      m_minus = DiyFp((v.f() << 2) - 1, v.e() - 2);
    } else {
      m_minus = DiyFp((v.f() << 1) - 1, v.e() - 1);
    }
    m_minus.set_f(m_minus.f() << (m_minus.e() - m_plus.e()));
    m_minus.set_e(m_plus.e());
    *out_m_plus = m_plus;
    *out_m_minus = m_minus;
  }

  bool LowerBoundaryIsCloser() const {
    bool physical_significand_is_zero = ((AsUint64() & kSignificandMask) == 0);
    return physical_significand_is_zero && (Exponent() != kDenormalExponent);
  }

  double value() const { return uint64_to_double(d64_); }

  static int SignificandSizeForOrderOfMagnitude(int order) {
    if (order >= (kDenormalExponent + kSignificandSize)) {
      return kSignificandSize;
    }
    if (order <= kDenormalExponent) return 0;
    return order - kDenormalExponent;
  }

  static double Infinity() {
    return Double(kInfinity).value();
  }

  static double NaN() {
    return Double(kNaN).value();
  }

 private:
  static const int kExponentBias = 0x3FF + kPhysicalSignificandSize;
  static const int kDenormalExponent = -kExponentBias + 1;
  static const int kMaxExponent = 0x7FF - kExponentBias;
  static const uint64_t kInfinity = UINT64_2PART_C(0x7FF00000, 00000000);
  static const uint64_t kNaN = UINT64_2PART_C(0x7FF80000, 00000000);

  const uint64_t d64_;

  static uint64_t DiyFpToUint64(DiyFp diy_fp) {
    uint64_t significand = diy_fp.f();
    int exponent = diy_fp.e();
    while (significand > kHiddenBit + kSignificandMask) {
      significand >>= 1;
      exponent++;
    }
    if (exponent >= kMaxExponent) {
      return kInfinity;
    }
    if (exponent < kDenormalExponent) {
      return 0;
    }
    while (exponent > kDenormalExponent && (significand & kHiddenBit) == 0) {
      significand <<= 1;
      exponent--;
    }
    uint64_t biased_exponent;
    if (exponent == kDenormalExponent && (significand & kHiddenBit) == 0) {
      biased_exponent = 0;
    } else {
      biased_exponent = static_cast<uint64_t>(exponent + kExponentBias);
    }
    return (significand & kSignificandMask) |
        (biased_exponent << kPhysicalSignificandSize);
  }

  DISALLOW_COPY_AND_ASSIGN(Double);
};

template<typename S>
static int BitSize(S value) {
  (void) value;  // Mark variable as used.
  return 8 * sizeof(value);
}

class Bignum {
 public:
  // 3584 = 128 * 28. We can represent 2^3584 > 10^1000 accurately.
  // This bignum can encode much bigger numbers, since it contains an
  // exponent.
  static const int kMaxSignificantBits = 3584;

  Bignum()
    : bigits_(bigits_buffer_, kBigitCapacity), used_digits_(0), exponent_(0)
  {
    for (int i = 0; i < kBigitCapacity; ++i) {
      bigits_[i] = 0;
    }
  }
  void AssignUInt16(uint16_t value) {
    ASSERT(kBigitSize >= BitSize(value));
    Zero();
    if (value == 0) return;

    EnsureCapacity(1);
    bigits_[0] = value;
    used_digits_ = 1;
  }
  void AssignUInt64(uint64_t value) {
    const int kUInt64Size = 64;

    Zero();
    if (value == 0) return;

    int needed_bigits = kUInt64Size / kBigitSize + 1;
    EnsureCapacity(needed_bigits);
    for (int i = 0; i < needed_bigits; ++i) {
      bigits_[i] = value & kBigitMask;
      value = value >> kBigitSize;
    }
    used_digits_ = needed_bigits;
    Clamp();
  }
  void AssignBignum(const Bignum& other) {
    exponent_ = other.exponent_;
    for (int i = 0; i < other.used_digits_; ++i) {
      bigits_[i] = other.bigits_[i];
    }
    // Clear the excess digits (if there were any).
    for (int i = other.used_digits_; i < used_digits_; ++i) {
      bigits_[i] = 0;
    }
    used_digits_ = other.used_digits_;
  }

  void AssignDecimalString(Vector<const char> value);
  void AssignHexString(Vector<const char> value);

  void AssignPowerUInt16(uint16_t base, int exponent);

  void AddUInt16(uint16_t operand);
  void AddUInt64(uint64_t operand);
  void AddBignum(const Bignum& other);
  // Precondition: this >= other.
  void SubtractBignum(const Bignum& other);

  void Square();
  void ShiftLeft(int shift_amount);
  void MultiplyByUInt32(uint32_t factor);
  void MultiplyByUInt64(uint64_t factor);
  void MultiplyByPowerOfTen(int exponent);
  void Times10() { return MultiplyByUInt32(10); }
  // Pseudocode:
  //  int result = this / other;
  //  this = this % other;
  // In the worst case this function is in O(this/other).
  uint16_t DivideModuloIntBignum(const Bignum& other);

  bool ToHexString(char* buffer, int buffer_size) const;

  // Returns
  //  -1 if a < b,
  //   0 if a == b, and
  //  +1 if a > b.
  static int Compare(const Bignum& a, const Bignum& b);
  static bool Equal(const Bignum& a, const Bignum& b) {
    return Compare(a, b) == 0;
  }
  static bool LessEqual(const Bignum& a, const Bignum& b) {
    return Compare(a, b) <= 0;
  }
  static bool Less(const Bignum& a, const Bignum& b) {
    return Compare(a, b) < 0;
  }
  // Returns Compare(a + b, c);
  static int PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c);
  // Returns a + b == c
  static bool PlusEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) == 0;
  }
  // Returns a + b <= c
  static bool PlusLessEqual(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) <= 0;
  }
  // Returns a + b < c
  static bool PlusLess(const Bignum& a, const Bignum& b, const Bignum& c) {
    return PlusCompare(a, b, c) < 0;
  }
 private:
  typedef uint32_t Chunk;
  typedef uint64_t DoubleChunk;

  static const int kChunkSize = sizeof(Chunk) * 8;
  static const int kDoubleChunkSize = sizeof(DoubleChunk) * 8;
  // With bigit size of 28 we loose some bits, but a double still fits easily
  // into two chunks, and more importantly we can use the Comba multiplication.
  static const int kBigitSize = 28;
  static const Chunk kBigitMask = (1 << kBigitSize) - 1;
  // Every instance allocates kBigitLength chunks on the stack. Bignums cannot
  // grow. There are no checks if the stack-allocated space is sufficient.
  static const int kBigitCapacity = kMaxSignificantBits / kBigitSize;

  void EnsureCapacity(int size) {
    if (size > kBigitCapacity) {
      UNREACHABLE();
    }
  }
  void Align(const Bignum& other);
  void Clamp();
  bool IsClamped() const;
  void Zero();
  // Requires this to have enough capacity (no tests done).
  // Updates used_digits_ if necessary.
  // shift_amount must be < kBigitSize.
  void BigitsShiftLeft(int shift_amount);
  // BigitLength includes the "hidden" digits encoded in the exponent.
  int BigitLength() const { return used_digits_ + exponent_; }
  Chunk BigitAt(int index) const;
  void SubtractTimes(const Bignum& other, int factor);

  Chunk bigits_buffer_[kBigitCapacity];
  // A vector backed by bigits_buffer_. This way accesses to the array are
  // checked for out-of-bounds errors.
  Vector<Chunk> bigits_;
  int used_digits_;
  // The Bignum's value equals value(bigits_) * 2^(exponent_ * kBigitSize).
  int exponent_;

  DISALLOW_COPY_AND_ASSIGN(Bignum);
};

static uint64_t ReadUInt64(Vector<const char> buffer,
                           int from,
                           int digits_to_read) {
  uint64_t result = 0;
  for (int i = from; i < from + digits_to_read; ++i) {
    int digit = buffer[i] - '0';
    ASSERT(0 <= digit && digit <= 9);
    result = result * 10 + digit;
  }
  return result;
}


void Bignum::AssignDecimalString(Vector<const char> value) {
  // 2^64 = 18446744073709551616 > 10^19
  const int kMaxUint64DecimalDigits = 19;
  Zero();
  int length = value.length();
  int pos = 0;
  // Let's just say that each digit needs 4 bits.
  while (length >= kMaxUint64DecimalDigits) {
    uint64_t digits = ReadUInt64(value, pos, kMaxUint64DecimalDigits);
    pos += kMaxUint64DecimalDigits;
    length -= kMaxUint64DecimalDigits;
    MultiplyByPowerOfTen(kMaxUint64DecimalDigits);
    AddUInt64(digits);
  }
  uint64_t digits = ReadUInt64(value, pos, length);
  MultiplyByPowerOfTen(length);
  AddUInt64(digits);
  Clamp();
}


static int HexCharValue(char c) {
  if ('0' <= c && c <= '9') return c - '0';
  if ('a' <= c && c <= 'f') return 10 + c - 'a';
  ASSERT('A' <= c && c <= 'F');
  return 10 + c - 'A';
}


void Bignum::AssignHexString(Vector<const char> value) {
  Zero();
  int length = value.length();

  int needed_bigits = length * 4 / kBigitSize + 1;
  EnsureCapacity(needed_bigits);
  int string_index = length - 1;
  for (int i = 0; i < needed_bigits - 1; ++i) {
    // These bigits are guaranteed to be "full".
    Chunk current_bigit = 0;
    for (int j = 0; j < kBigitSize / 4; j++) {
      current_bigit += HexCharValue(value[string_index--]) << (j * 4);
    }
    bigits_[i] = current_bigit;
  }
  used_digits_ = needed_bigits - 1;

  Chunk most_significant_bigit = 0;  // Could be = 0;
  for (int j = 0; j <= string_index; ++j) {
    most_significant_bigit <<= 4;
    most_significant_bigit += HexCharValue(value[j]);
  }
  if (most_significant_bigit != 0) {
    bigits_[used_digits_] = most_significant_bigit;
    used_digits_++;
  }
  Clamp();
}


void Bignum::AddUInt64(uint64_t operand) {
  if (operand == 0) return;
  Bignum other;
  other.AssignUInt64(operand);
  AddBignum(other);
}


void Bignum::AddBignum(const Bignum& other) {
  ASSERT(IsClamped());
  ASSERT(other.IsClamped());

  Align(other);

  EnsureCapacity(1 + std::max(BigitLength(), other.BigitLength()) - exponent_);
  Chunk carry = 0;
  int bigit_pos = other.exponent_ - exponent_;
  ASSERT(bigit_pos >= 0);
  for (int i = 0; i < other.used_digits_; ++i) {
    Chunk sum = bigits_[bigit_pos] + other.bigits_[i] + carry;
    bigits_[bigit_pos] = sum & kBigitMask;
    carry = sum >> kBigitSize;
    bigit_pos++;
  }

  while (carry != 0) {
    Chunk sum = bigits_[bigit_pos] + carry;
    bigits_[bigit_pos] = sum & kBigitMask;
    carry = sum >> kBigitSize;
    bigit_pos++;
  }
  used_digits_ = std::max(bigit_pos, used_digits_);
  ASSERT(IsClamped());
}


void Bignum::SubtractBignum(const Bignum& other) {
  ASSERT(IsClamped());
  ASSERT(other.IsClamped());
  // We require this to be bigger than other.
  ASSERT(LessEqual(other, *this));

  Align(other);

  int offset = other.exponent_ - exponent_;
  Chunk borrow = 0;
  int i;
  for (i = 0; i < other.used_digits_; ++i) {
    ASSERT((borrow == 0) || (borrow == 1));
    Chunk difference = bigits_[i + offset] - other.bigits_[i] - borrow;
    bigits_[i + offset] = difference & kBigitMask;
    borrow = difference >> (kChunkSize - 1);
  }
  while (borrow != 0) {
    Chunk difference = bigits_[i + offset] - borrow;
    bigits_[i + offset] = difference & kBigitMask;
    borrow = difference >> (kChunkSize - 1);
    ++i;
  }
  Clamp();
}


void Bignum::ShiftLeft(int shift_amount) {
  if (used_digits_ == 0) return;
  exponent_ += shift_amount / kBigitSize;
  int local_shift = shift_amount % kBigitSize;
  EnsureCapacity(used_digits_ + 1);
  BigitsShiftLeft(local_shift);
}


void Bignum::MultiplyByUInt32(uint32_t factor) {
  if (factor == 1) return;
  if (factor == 0) {
    Zero();
    return;
  }
  if (used_digits_ == 0) return;

  ASSERT(kDoubleChunkSize >= kBigitSize + 32 + 1);
  DoubleChunk carry = 0;
  for (int i = 0; i < used_digits_; ++i) {
    DoubleChunk product = static_cast<DoubleChunk>(factor) * bigits_[i] + carry;
    bigits_[i] = static_cast<Chunk>(product & kBigitMask);
    carry = (product >> kBigitSize);
  }
  while (carry != 0) {
    EnsureCapacity(used_digits_ + 1);
    bigits_[used_digits_] = carry & kBigitMask;
    used_digits_++;
    carry >>= kBigitSize;
  }
}


void Bignum::MultiplyByUInt64(uint64_t factor) {
  if (factor == 1) return;
  if (factor == 0) {
    Zero();
    return;
  }
  ASSERT(kBigitSize < 32);
  uint64_t carry = 0;
  uint64_t low = factor & 0xFFFFFFFF;
  uint64_t high = factor >> 32;
  for (int i = 0; i < used_digits_; ++i) {
    uint64_t product_low = low * bigits_[i];
    uint64_t product_high = high * bigits_[i];
    uint64_t tmp = (carry & kBigitMask) + product_low;
    bigits_[i] = tmp & kBigitMask;
    carry = (carry >> kBigitSize) + (tmp >> kBigitSize) +
        (product_high << (32 - kBigitSize));
  }
  while (carry != 0) {
    EnsureCapacity(used_digits_ + 1);
    bigits_[used_digits_] = carry & kBigitMask;
    used_digits_++;
    carry >>= kBigitSize;
  }
}


void Bignum::MultiplyByPowerOfTen(int exponent) {
  const uint64_t kFive27 = UINT64_2PART_C(0x6765c793, fa10079d);
  const uint16_t kFive1 = 5;
  const uint16_t kFive2 = kFive1 * 5;
  const uint16_t kFive3 = kFive2 * 5;
  const uint16_t kFive4 = kFive3 * 5;
  const uint16_t kFive5 = kFive4 * 5;
  const uint16_t kFive6 = kFive5 * 5;
  const uint32_t kFive7 = kFive6 * 5;
  const uint32_t kFive8 = kFive7 * 5;
  const uint32_t kFive9 = kFive8 * 5;
  const uint32_t kFive10 = kFive9 * 5;
  const uint32_t kFive11 = kFive10 * 5;
  const uint32_t kFive12 = kFive11 * 5;
  const uint32_t kFive13 = kFive12 * 5;
  const uint32_t kFive1_to_12[] =
      { kFive1, kFive2, kFive3, kFive4, kFive5, kFive6,
        kFive7, kFive8, kFive9, kFive10, kFive11, kFive12 };

  ASSERT(exponent >= 0);
  if (exponent == 0) return;
  if (used_digits_ == 0) return;

  int remaining_exponent = exponent;
  while (remaining_exponent >= 27) {
    MultiplyByUInt64(kFive27);
    remaining_exponent -= 27;
  }
  while (remaining_exponent >= 13) {
    MultiplyByUInt32(kFive13);
    remaining_exponent -= 13;
  }
  if (remaining_exponent > 0) {
    MultiplyByUInt32(kFive1_to_12[remaining_exponent - 1]);
  }
  ShiftLeft(exponent);
}


void Bignum::Square() {
  ASSERT(IsClamped());
  int product_length = 2 * used_digits_;
  EnsureCapacity(product_length);

  if ((1 << (2 * (kChunkSize - kBigitSize))) <= used_digits_) {
    UNIMPLEMENTED();
  }
  DoubleChunk accumulator = 0;
  // First shift the digits so we don't overwrite them.
  int copy_offset = used_digits_;
  for (int i = 0; i < used_digits_; ++i) {
    bigits_[copy_offset + i] = bigits_[i];
  }
  // We have two loops to avoid some 'if's in the loop.
  for (int i = 0; i < used_digits_; ++i) {
    // Process temporary digit i with power i.
    // The sum of the two indices must be equal to i.
    int bigit_index1 = i;
    int bigit_index2 = 0;
    // Sum all of the sub-products.
    while (bigit_index1 >= 0) {
      Chunk chunk1 = bigits_[copy_offset + bigit_index1];
      Chunk chunk2 = bigits_[copy_offset + bigit_index2];
      accumulator += static_cast<DoubleChunk>(chunk1) * chunk2;
      bigit_index1--;
      bigit_index2++;
    }
    bigits_[i] = static_cast<Chunk>(accumulator) & kBigitMask;
    accumulator >>= kBigitSize;
  }
  for (int i = used_digits_; i < product_length; ++i) {
    int bigit_index1 = used_digits_ - 1;
    int bigit_index2 = i - bigit_index1;

    while (bigit_index2 < used_digits_) {
      Chunk chunk1 = bigits_[copy_offset + bigit_index1];
      Chunk chunk2 = bigits_[copy_offset + bigit_index2];
      accumulator += static_cast<DoubleChunk>(chunk1) * chunk2;
      bigit_index1--;
      bigit_index2++;
    }
    bigits_[i] = static_cast<Chunk>(accumulator) & kBigitMask;
    accumulator >>= kBigitSize;
  }

  ASSERT(accumulator == 0);

  used_digits_ = product_length;
  exponent_ *= 2;
  Clamp();
}


void Bignum::AssignPowerUInt16(uint16_t base, int power_exponent) {
  ASSERT(base != 0);
  ASSERT(power_exponent >= 0);
  if (power_exponent == 0) {
    AssignUInt16(1);
    return;
  }
  Zero();
  int shifts = 0;

  while ((base & 1) == 0) {
    base >>= 1;
    shifts++;
  }
  int bit_size = 0;
  int tmp_base = base;
  while (tmp_base != 0) {
    tmp_base >>= 1;
    bit_size++;
  }
  int final_size = bit_size * power_exponent;

  EnsureCapacity(final_size / kBigitSize + 2);

  // Left to Right exponentiation.
  int mask = 1;
  while (power_exponent >= mask) mask <<= 1;

  mask >>= 2;
  uint64_t this_value = base;

  bool delayed_multipliciation = false;
  const uint64_t max_32bits = 0xFFFFFFFF;
  while (mask != 0 && this_value <= max_32bits) {
    this_value = this_value * this_value;
    // Verify that there is enough space in this_value to perform the
    // multiplication.  The first bit_size bits must be 0.
    if ((power_exponent & mask) != 0) {
      uint64_t base_bits_mask =
          ~((static_cast<uint64_t>(1) << (64 - bit_size)) - 1);
      bool high_bits_zero = (this_value & base_bits_mask) == 0;
      if (high_bits_zero) {
        this_value *= base;
      } else {
        delayed_multipliciation = true;
      }
    }
    mask >>= 1;
  }
  AssignUInt64(this_value);
  if (delayed_multipliciation) {
    MultiplyByUInt32(base);
  }

  // Now do the same thing as a bignum.
  while (mask != 0) {
    Square();
    if ((power_exponent & mask) != 0) {
      MultiplyByUInt32(base);
    }
    mask >>= 1;
  }

  // And finally add the saved shifts.
  ShiftLeft(shifts * power_exponent);
}


uint16_t Bignum::DivideModuloIntBignum(const Bignum& other) {
  ASSERT(IsClamped());
  ASSERT(other.IsClamped());
  ASSERT(other.used_digits_ > 0);

  if (BigitLength() < other.BigitLength()) {
    return 0;
  }

  Align(other);

  uint16_t result = 0;

  while (BigitLength() > other.BigitLength()) {
    ASSERT(other.bigits_[other.used_digits_ - 1] >= ((1 << kBigitSize) / 16));
    ASSERT(bigits_[used_digits_ - 1] < 0x10000);
    result += static_cast<uint16_t>(bigits_[used_digits_ - 1]);
    SubtractTimes(other, bigits_[used_digits_ - 1]);
  }

  ASSERT(BigitLength() == other.BigitLength());

  Chunk this_bigit = bigits_[used_digits_ - 1];
  Chunk other_bigit = other.bigits_[other.used_digits_ - 1];

  if (other.used_digits_ == 1) {
    int quotient = this_bigit / other_bigit;
    bigits_[used_digits_ - 1] = this_bigit - other_bigit * quotient;
    ASSERT(quotient < 0x10000);
    result += static_cast<uint16_t>(quotient);
    Clamp();
    return result;
  }

  int division_estimate = this_bigit / (other_bigit + 1);
  ASSERT(division_estimate < 0x10000);
  result += static_cast<uint16_t>(division_estimate);
  SubtractTimes(other, division_estimate);

  if (other_bigit * (division_estimate + 1) > this_bigit) {
    return result;
  }

  while (LessEqual(other, *this)) {
    SubtractBignum(other);
    result++;
  }
  return result;
}


template<typename S>
static int SizeInHexChars(S number) {
  ASSERT(number > 0);
  int result = 0;
  while (number != 0) {
    number >>= 4;
    result++;
  }
  return result;
}


static char HexCharOfValue(int value) {
  ASSERT(0 <= value && value <= 16);
  if (value < 10) return static_cast<char>(value + '0');
  return static_cast<char>(value - 10 + 'A');
}


bool Bignum::ToHexString(char* buffer, int buffer_size) const {
  ASSERT(IsClamped());
  // Each bigit must be printable as separate hex-character.
  ASSERT(kBigitSize % 4 == 0);
  const int kHexCharsPerBigit = kBigitSize / 4;

  if (used_digits_ == 0) {
    if (buffer_size < 2) return false;
    buffer[0] = '0';
    buffer[1] = '\0';
    return true;
  }
  // We add 1 for the terminating '\0' character.
  int needed_chars = (BigitLength() - 1) * kHexCharsPerBigit +
      SizeInHexChars(bigits_[used_digits_ - 1]) + 1;
  if (needed_chars > buffer_size) return false;
  int string_index = needed_chars - 1;
  buffer[string_index--] = '\0';
  for (int i = 0; i < exponent_; ++i) {
    for (int j = 0; j < kHexCharsPerBigit; ++j) {
      buffer[string_index--] = '0';
    }
  }
  for (int i = 0; i < used_digits_ - 1; ++i) {
    Chunk current_bigit = bigits_[i];
    for (int j = 0; j < kHexCharsPerBigit; ++j) {
      buffer[string_index--] = HexCharOfValue(current_bigit & 0xF);
      current_bigit >>= 4;
    }
  }
  // And finally the last bigit.
  Chunk most_significant_bigit = bigits_[used_digits_ - 1];
  while (most_significant_bigit != 0) {
    buffer[string_index--] = HexCharOfValue(most_significant_bigit & 0xF);
    most_significant_bigit >>= 4;
  }
  return true;
}


Bignum::Chunk Bignum::BigitAt(int index) const {
  if (index >= BigitLength()) return 0;
  if (index < exponent_) return 0;
  return bigits_[index - exponent_];
}


int Bignum::Compare(const Bignum& a, const Bignum& b) {
  ASSERT(a.IsClamped());
  ASSERT(b.IsClamped());
  int bigit_length_a = a.BigitLength();
  int bigit_length_b = b.BigitLength();
  if (bigit_length_a < bigit_length_b) return -1;
  if (bigit_length_a > bigit_length_b) return +1;
  for (int i = bigit_length_a - 1; i >= std::min(a.exponent_, b.exponent_); --i) {
    Chunk bigit_a = a.BigitAt(i);
    Chunk bigit_b = b.BigitAt(i);
    if (bigit_a < bigit_b) return -1;
    if (bigit_a > bigit_b) return +1;
  }
  return 0;
}


int Bignum::PlusCompare(const Bignum& a, const Bignum& b, const Bignum& c) {
  ASSERT(a.IsClamped());
  ASSERT(b.IsClamped());
  ASSERT(c.IsClamped());
  if (a.BigitLength() < b.BigitLength()) {
    return PlusCompare(b, a, c);
  }
  if (a.BigitLength() + 1 < c.BigitLength()) return -1;
  if (a.BigitLength() > c.BigitLength()) return +1;

  if (a.exponent_ >= b.BigitLength() && a.BigitLength() < c.BigitLength()) {
    return -1;
  }

  Chunk borrow = 0;
  // Starting at min_exponent all digits are == 0. So no need to compare them.
  int min_exponent = std::min(std::min(a.exponent_, b.exponent_), c.exponent_);
  for (int i = c.BigitLength() - 1; i >= min_exponent; --i) {
    Chunk chunk_a = a.BigitAt(i);
    Chunk chunk_b = b.BigitAt(i);
    Chunk chunk_c = c.BigitAt(i);
    Chunk sum = chunk_a + chunk_b;
    if (sum > chunk_c + borrow) {
      return +1;
    } else {
      borrow = chunk_c + borrow - sum;
      if (borrow > 1) return -1;
      borrow <<= kBigitSize;
    }
  }
  if (borrow == 0) return 0;
  return -1;
}


void Bignum::Clamp() {
  while (used_digits_ > 0 && bigits_[used_digits_ - 1] == 0) {
    used_digits_--;
  }
  if (used_digits_ == 0) {
    // Zero.
    exponent_ = 0;
  }
}


bool Bignum::IsClamped() const {
  return used_digits_ == 0 || bigits_[used_digits_ - 1] != 0;
}


void Bignum::Zero() {
  for (int i = 0; i < used_digits_; ++i) {
    bigits_[i] = 0;
  }
  used_digits_ = 0;
  exponent_ = 0;
}


void Bignum::Align(const Bignum& other) {
  if (exponent_ > other.exponent_) {
    int zero_digits = exponent_ - other.exponent_;
    EnsureCapacity(used_digits_ + zero_digits);
    for (int i = used_digits_ - 1; i >= 0; --i) {
      bigits_[i + zero_digits] = bigits_[i];
    }
    for (int i = 0; i < zero_digits; ++i) {
      bigits_[i] = 0;
    }
    used_digits_ += zero_digits;
    exponent_ -= zero_digits;
    ASSERT(used_digits_ >= 0);
    ASSERT(exponent_ >= 0);
  }
}


void Bignum::BigitsShiftLeft(int shift_amount) {
  ASSERT(shift_amount < kBigitSize);
  ASSERT(shift_amount >= 0);
  Chunk carry = 0;
  for (int i = 0; i < used_digits_; ++i) {
    Chunk new_carry = bigits_[i] >> (kBigitSize - shift_amount);
    bigits_[i] = ((bigits_[i] << shift_amount) + carry) & kBigitMask;
    carry = new_carry;
  }
  if (carry != 0) {
    bigits_[used_digits_] = carry;
    used_digits_++;
  }
}


void Bignum::SubtractTimes(const Bignum& other, int factor) {
  ASSERT(exponent_ <= other.exponent_);
  if (factor < 3) {
    for (int i = 0; i < factor; ++i) {
      SubtractBignum(other);
    }
    return;
  }
  Chunk borrow = 0;
  int exponent_diff = other.exponent_ - exponent_;
  for (int i = 0; i < other.used_digits_; ++i) {
    DoubleChunk product = static_cast<DoubleChunk>(factor) * other.bigits_[i];
    DoubleChunk remove = borrow + product;
    Chunk difference = bigits_[i + exponent_diff] - (remove & kBigitMask);
    bigits_[i + exponent_diff] = difference & kBigitMask;
    borrow = static_cast<Chunk>((difference >> (kChunkSize - 1)) +
                                (remove >> kBigitSize));
  }
  for (int i = other.used_digits_ + exponent_diff; i < used_digits_; ++i) {
    if (borrow == 0) return;
    Chunk difference = bigits_[i] - borrow;
    bigits_[i] = difference & kBigitMask;
    borrow = difference >> (kChunkSize - 1);
  }
  Clamp();
}

class PowersOfTenCache {
public:
  static const int kDecimalExponentDistance;

  static const int kMinDecimalExponent;
  static const int kMaxDecimalExponent;

  static void GetCachedPowerForBinaryExponentRange(int min_exponent,
                                                   int max_exponent,
                                                   DiyFp* power,
                                                   int* decimal_exponent);

  static void GetCachedPowerForDecimalExponent(int requested_exponent,
                                               DiyFp* power,
                                               int* found_exponent);
};

struct CachedPower {
  uint64_t significand;
  int16_t binary_exponent;
  int16_t decimal_exponent;
};

static const CachedPower kCachedPowers[] = {
  {UINT64_2PART_C(0xfa8fd5a0, 081c0288), -1220, -348},
  {UINT64_2PART_C(0xbaaee17f, a23ebf76), -1193, -340},
  {UINT64_2PART_C(0x8b16fb20, 3055ac76), -1166, -332},
  {UINT64_2PART_C(0xcf42894a, 5dce35ea), -1140, -324},
  {UINT64_2PART_C(0x9a6bb0aa, 55653b2d), -1113, -316},
  {UINT64_2PART_C(0xe61acf03, 3d1a45df), -1087, -308},
  {UINT64_2PART_C(0xab70fe17, c79ac6ca), -1060, -300},
  {UINT64_2PART_C(0xff77b1fc, bebcdc4f), -1034, -292},
  {UINT64_2PART_C(0xbe5691ef, 416bd60c), -1007, -284},
  {UINT64_2PART_C(0x8dd01fad, 907ffc3c), -980, -276},
  {UINT64_2PART_C(0xd3515c28, 31559a83), -954, -268},
  {UINT64_2PART_C(0x9d71ac8f, ada6c9b5), -927, -260},
  {UINT64_2PART_C(0xea9c2277, 23ee8bcb), -901, -252},
  {UINT64_2PART_C(0xaecc4991, 4078536d), -874, -244},
  {UINT64_2PART_C(0x823c1279, 5db6ce57), -847, -236},
  {UINT64_2PART_C(0xc2109436, 4dfb5637), -821, -228},
  {UINT64_2PART_C(0x9096ea6f, 3848984f), -794, -220},
  {UINT64_2PART_C(0xd77485cb, 25823ac7), -768, -212},
  {UINT64_2PART_C(0xa086cfcd, 97bf97f4), -741, -204},
  {UINT64_2PART_C(0xef340a98, 172aace5), -715, -196},
  {UINT64_2PART_C(0xb23867fb, 2a35b28e), -688, -188},
  {UINT64_2PART_C(0x84c8d4df, d2c63f3b), -661, -180},
  {UINT64_2PART_C(0xc5dd4427, 1ad3cdba), -635, -172},
  {UINT64_2PART_C(0x936b9fce, bb25c996), -608, -164},
  {UINT64_2PART_C(0xdbac6c24, 7d62a584), -582, -156},
  {UINT64_2PART_C(0xa3ab6658, 0d5fdaf6), -555, -148},
  {UINT64_2PART_C(0xf3e2f893, dec3f126), -529, -140},
  {UINT64_2PART_C(0xb5b5ada8, aaff80b8), -502, -132},
  {UINT64_2PART_C(0x87625f05, 6c7c4a8b), -475, -124},
  {UINT64_2PART_C(0xc9bcff60, 34c13053), -449, -116},
  {UINT64_2PART_C(0x964e858c, 91ba2655), -422, -108},
  {UINT64_2PART_C(0xdff97724, 70297ebd), -396, -100},
  {UINT64_2PART_C(0xa6dfbd9f, b8e5b88f), -369, -92},
  {UINT64_2PART_C(0xf8a95fcf, 88747d94), -343, -84},
  {UINT64_2PART_C(0xb9447093, 8fa89bcf), -316, -76},
  {UINT64_2PART_C(0x8a08f0f8, bf0f156b), -289, -68},
  {UINT64_2PART_C(0xcdb02555, 653131b6), -263, -60},
  {UINT64_2PART_C(0x993fe2c6, d07b7fac), -236, -52},
  {UINT64_2PART_C(0xe45c10c4, 2a2b3b06), -210, -44},
  {UINT64_2PART_C(0xaa242499, 697392d3), -183, -36},
  {UINT64_2PART_C(0xfd87b5f2, 8300ca0e), -157, -28},
  {UINT64_2PART_C(0xbce50864, 92111aeb), -130, -20},
  {UINT64_2PART_C(0x8cbccc09, 6f5088cc), -103, -12},
  {UINT64_2PART_C(0xd1b71758, e219652c), -77, -4},
  {UINT64_2PART_C(0x9c400000, 00000000), -50, 4},
  {UINT64_2PART_C(0xe8d4a510, 00000000), -24, 12},
  {UINT64_2PART_C(0xad78ebc5, ac620000), 3, 20},
  {UINT64_2PART_C(0x813f3978, f8940984), 30, 28},
  {UINT64_2PART_C(0xc097ce7b, c90715b3), 56, 36},
  {UINT64_2PART_C(0x8f7e32ce, 7bea5c70), 83, 44},
  {UINT64_2PART_C(0xd5d238a4, abe98068), 109, 52},
  {UINT64_2PART_C(0x9f4f2726, 179a2245), 136, 60},
  {UINT64_2PART_C(0xed63a231, d4c4fb27), 162, 68},
  {UINT64_2PART_C(0xb0de6538, 8cc8ada8), 189, 76},
  {UINT64_2PART_C(0x83c7088e, 1aab65db), 216, 84},
  {UINT64_2PART_C(0xc45d1df9, 42711d9a), 242, 92},
  {UINT64_2PART_C(0x924d692c, a61be758), 269, 100},
  {UINT64_2PART_C(0xda01ee64, 1a708dea), 295, 108},
  {UINT64_2PART_C(0xa26da399, 9aef774a), 322, 116},
  {UINT64_2PART_C(0xf209787b, b47d6b85), 348, 124},
  {UINT64_2PART_C(0xb454e4a1, 79dd1877), 375, 132},
  {UINT64_2PART_C(0x865b8692, 5b9bc5c2), 402, 140},
  {UINT64_2PART_C(0xc83553c5, c8965d3d), 428, 148},
  {UINT64_2PART_C(0x952ab45c, fa97a0b3), 455, 156},
  {UINT64_2PART_C(0xde469fbd, 99a05fe3), 481, 164},
  {UINT64_2PART_C(0xa59bc234, db398c25), 508, 172},
  {UINT64_2PART_C(0xf6c69a72, a3989f5c), 534, 180},
  {UINT64_2PART_C(0xb7dcbf53, 54e9bece), 561, 188},
  {UINT64_2PART_C(0x88fcf317, f22241e2), 588, 196},
  {UINT64_2PART_C(0xcc20ce9b, d35c78a5), 614, 204},
  {UINT64_2PART_C(0x98165af3, 7b2153df), 641, 212},
  {UINT64_2PART_C(0xe2a0b5dc, 971f303a), 667, 220},
  {UINT64_2PART_C(0xa8d9d153, 5ce3b396), 694, 228},
  {UINT64_2PART_C(0xfb9b7cd9, a4a7443c), 720, 236},
  {UINT64_2PART_C(0xbb764c4c, a7a44410), 747, 244},
  {UINT64_2PART_C(0x8bab8eef, b6409c1a), 774, 252},
  {UINT64_2PART_C(0xd01fef10, a657842c), 800, 260},
  {UINT64_2PART_C(0x9b10a4e5, e9913129), 827, 268},
  {UINT64_2PART_C(0xe7109bfb, a19c0c9d), 853, 276},
  {UINT64_2PART_C(0xac2820d9, 623bf429), 880, 284},
  {UINT64_2PART_C(0x80444b5e, 7aa7cf85), 907, 292},
  {UINT64_2PART_C(0xbf21e440, 03acdd2d), 933, 300},
  {UINT64_2PART_C(0x8e679c2f, 5e44ff8f), 960, 308},
  {UINT64_2PART_C(0xd433179d, 9c8cb841), 986, 316},
  {UINT64_2PART_C(0x9e19db92, b4e31ba9), 1013, 324},
  {UINT64_2PART_C(0xeb96bf6e, badf77d9), 1039, 332},
  {UINT64_2PART_C(0xaf87023b, 9bf0ee6b), 1066, 340},
};

static const int kCachedPowersLength = ARRAY_SIZE(kCachedPowers);
static const int kCachedPowersOffset = 348;  // -1 * the first decimal_exponent.
static const double kD_1_LOG2_10 = 0.30102999566398114;  //  1 / lg(10)
// Difference between the decimal exponents in the table above.
const int PowersOfTenCache::kDecimalExponentDistance = 8;
const int PowersOfTenCache::kMinDecimalExponent = -348;
const int PowersOfTenCache::kMaxDecimalExponent = 340;

void PowersOfTenCache::GetCachedPowerForBinaryExponentRange(
    int min_exponent,
    int max_exponent,
    DiyFp* power,
    int* decimal_exponent) {
  int kQ = DiyFp::kSignificandSize;
  double k = ceil((min_exponent + kQ - 1) * kD_1_LOG2_10);
  int foo = kCachedPowersOffset;
  int index =
      (foo + static_cast<int>(k) - 1) / kDecimalExponentDistance + 1;
  ASSERT(0 <= index && index < kCachedPowersLength);
  CachedPower cached_power = kCachedPowers[index];
  ASSERT(min_exponent <= cached_power.binary_exponent);
  (void) max_exponent;  // Mark variable as used.
  ASSERT(cached_power.binary_exponent <= max_exponent);
  *decimal_exponent = cached_power.decimal_exponent;
  *power = DiyFp(cached_power.significand, cached_power.binary_exponent);
}


void PowersOfTenCache::GetCachedPowerForDecimalExponent(int requested_exponent,
                                                        DiyFp* power,
                                                        int* found_exponent) {
  ASSERT(kMinDecimalExponent <= requested_exponent);
  ASSERT(requested_exponent < kMaxDecimalExponent + kDecimalExponentDistance);
  int index =
      (requested_exponent + kCachedPowersOffset) / kDecimalExponentDistance;
  CachedPower cached_power = kCachedPowers[index];
  *power = DiyFp(cached_power.significand, cached_power.binary_exponent);
  *found_exponent = cached_power.decimal_exponent;
  ASSERT(*found_exponent <= requested_exponent);
  ASSERT(requested_exponent < *found_exponent + kDecimalExponentDistance);
}

enum BignumDtoaMode {
  BIGNUM_DTOA_SHORTEST,
  BIGNUM_DTOA_FIXED,
  BIGNUM_DTOA_PRECISION
};

static int NormalizedExponent(uint64_t significand, int exponent) {
  ASSERT(significand != 0);
  while ((significand & Double::kHiddenBit) == 0) {
    significand = significand << 1;
    exponent = exponent - 1;
  }
  return exponent;
}

static int EstimatePower(int exponent);

static void InitialScaledStartValues(uint64_t significand,
                                     int exponent,
                                     bool lower_boundary_is_closer,
                                     int estimated_power,
                                     bool need_boundary_deltas,
                                     Bignum* numerator,
                                     Bignum* denominator,
                                     Bignum* delta_minus,
                                     Bignum* delta_plus);

static void FixupMultiply10(int estimated_power, bool is_even,
                            int* decimal_point,
                            Bignum* numerator, Bignum* denominator,
                            Bignum* delta_minus, Bignum* delta_plus);

static void GenerateShortestDigits(Bignum* numerator, Bignum* denominator,
                                   Bignum* delta_minus, Bignum* delta_plus,
                                   bool is_even,
                                   Vector<char> buffer, int* length);

static void BignumToFixed(int requested_digits, int* decimal_point,
                          Bignum* numerator, Bignum* denominator,
                          Vector<char>(buffer), int* length);

static void GenerateCountedDigits(int count, int* decimal_point,
                                  Bignum* numerator, Bignum* denominator,
                                  Vector<char>(buffer), int* length);


void BignumDtoa(double v, BignumDtoaMode mode, int requested_digits,
                Vector<char> buffer, int* length, int* decimal_point) {
  ASSERT(v > 0);
  ASSERT(!Double(v).IsSpecial());
  uint64_t significand;
  int exponent;
  bool lower_boundary_is_closer;

  significand = Double(v).Significand();
  exponent = Double(v).Exponent();
  lower_boundary_is_closer = Double(v).LowerBoundaryIsCloser();

  bool need_boundary_deltas =
      (mode == BIGNUM_DTOA_SHORTEST);

  bool is_even = (significand & 1) == 0;
  int normalized_exponent = NormalizedExponent(significand, exponent);
  // estimated_power might be too low by 1.
  int estimated_power = EstimatePower(normalized_exponent);

  if (mode == BIGNUM_DTOA_FIXED && -estimated_power - 1 > requested_digits) {
    buffer[0] = '\0';
    *length = 0;
    *decimal_point = -requested_digits;
    return;
  }

  Bignum numerator;
  Bignum denominator;
  Bignum delta_minus;
  Bignum delta_plus;

  ASSERT(Bignum::kMaxSignificantBits >= 324*4);
  InitialScaledStartValues(significand, exponent, lower_boundary_is_closer,
                           estimated_power, need_boundary_deltas,
                           &numerator, &denominator,
                           &delta_minus, &delta_plus);

  FixupMultiply10(estimated_power, is_even, decimal_point,
                  &numerator, &denominator,
                  &delta_minus, &delta_plus);

  switch (mode) {
    case BIGNUM_DTOA_SHORTEST:
      GenerateShortestDigits(&numerator, &denominator,
                             &delta_minus, &delta_plus,
                             is_even, buffer, length);
      break;
    case BIGNUM_DTOA_FIXED:
      BignumToFixed(requested_digits, decimal_point,
                    &numerator, &denominator,
                    buffer, length);
      break;
    case BIGNUM_DTOA_PRECISION:
      GenerateCountedDigits(requested_digits, decimal_point,
                            &numerator, &denominator,
                            buffer, length);
      break;
    default:
      UNREACHABLE();
  }
  buffer[*length] = '\0';
}

static void GenerateShortestDigits(Bignum* numerator, Bignum* denominator,
                                   Bignum* delta_minus, Bignum* delta_plus,
                                   bool is_even,
                                   Vector<char> buffer, int* length) {
  if (Bignum::Equal(*delta_minus, *delta_plus)) {
    delta_plus = delta_minus;
  }
  *length = 0;
  for (;;) {
    uint16_t digit;
    digit = numerator->DivideModuloIntBignum(*denominator);
    ASSERT(digit <= 9);

    buffer[(*length)++] = static_cast<char>(digit + '0');

    bool in_delta_room_minus;
    bool in_delta_room_plus;
    if (is_even) {
      in_delta_room_minus = Bignum::LessEqual(*numerator, *delta_minus);
    } else {
      in_delta_room_minus = Bignum::Less(*numerator, *delta_minus);
    }
    if (is_even) {
      in_delta_room_plus =
          Bignum::PlusCompare(*numerator, *delta_plus, *denominator) >= 0;
    } else {
      in_delta_room_plus =
          Bignum::PlusCompare(*numerator, *delta_plus, *denominator) > 0;
    }
    if (!in_delta_room_minus && !in_delta_room_plus) {
      numerator->Times10();
      delta_minus->Times10();

      if (delta_minus != delta_plus) {
        delta_plus->Times10();
      }
    } else if (in_delta_room_minus && in_delta_room_plus) {

      int compare = Bignum::PlusCompare(*numerator, *numerator, *denominator);
      if (compare < 0) {
        // Remaining digits are less than .5. -> Round down (== do nothing).
      } else if (compare > 0) {
        // Remaining digits are more than .5 of denominator. -> Round up.
        ASSERT(buffer[(*length) - 1] != '9');
        buffer[(*length) - 1]++;
      } else {
        if ((buffer[(*length) - 1] - '0') % 2 == 0) {
          // Round down => Do nothing.
        } else {
          ASSERT(buffer[(*length) - 1] != '9');
          buffer[(*length) - 1]++;
        }
      }
      return;
    } else if (in_delta_room_minus) {
      return;
    } else {  // in_delta_room_plus
      // Round up
      ASSERT(buffer[(*length) -1] != '9');
      buffer[(*length) - 1]++;
      return;
    }
  }
}

static void GenerateCountedDigits(int count, int* decimal_point,
                                  Bignum* numerator, Bignum* denominator,
                                  Vector<char> buffer, int* length) {
  ASSERT(count >= 0);
  for (int i = 0; i < count - 1; ++i) {
    uint16_t digit;
    digit = numerator->DivideModuloIntBignum(*denominator);
    ASSERT(digit <= 9);

    buffer[i] = static_cast<char>(digit + '0');
    numerator->Times10();
  }

  uint16_t digit;
  digit = numerator->DivideModuloIntBignum(*denominator);
  if (Bignum::PlusCompare(*numerator, *numerator, *denominator) >= 0) {
    digit++;
  }
  ASSERT(digit <= 10);
  buffer[count - 1] = static_cast<char>(digit + '0');

  for (int i = count - 1; i > 0; --i) {
    if (buffer[i] != '0' + 10) break;
    buffer[i] = '0';
    buffer[i - 1]++;
  }
  if (buffer[0] == '0' + 10) {
    buffer[0] = '1';
    (*decimal_point)++;
  }
  *length = count;
}

static void BignumToFixed(int requested_digits, int* decimal_point,
                          Bignum* numerator, Bignum* denominator,
                          Vector<char>(buffer), int* length)
{
  if (-(*decimal_point) > requested_digits) {
    *decimal_point = -requested_digits;
    *length = 0;
    return;
  } else if (-(*decimal_point) == requested_digits) {
    ASSERT(*decimal_point == -requested_digits);

    denominator->Times10();
    if (Bignum::PlusCompare(*numerator, *numerator, *denominator) >= 0) {
      buffer[0] = '1';
      *length = 1;
      (*decimal_point)++;
    } else {
      *length = 0;
    }
    return;
  } else {
    int needed_digits = (*decimal_point) + requested_digits;
    GenerateCountedDigits(needed_digits, decimal_point,
                          numerator, denominator,
                          buffer, length);
  }
}

static int EstimatePower(int exponent) {
  const double k1Log10 = 0.30102999566398114;  // 1/lg(10)

  // For doubles len(f) == 53 (don't forget the hidden bit).
  const int kSignificandSize = Double::kSignificandSize;
  double estimate = ceil((exponent + kSignificandSize - 1) * k1Log10 - 1e-10);
  return static_cast<int>(estimate);
}

static void InitialScaledStartValuesPositiveExponent(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus)
{
  ASSERT(estimated_power >= 0);

  numerator->AssignUInt64(significand);
  numerator->ShiftLeft(exponent);
  denominator->AssignPowerUInt16(10, estimated_power);

  if (need_boundary_deltas) {
    denominator->ShiftLeft(1);
    numerator->ShiftLeft(1);
    delta_plus->AssignUInt16(1);
    delta_plus->ShiftLeft(exponent);
    delta_minus->AssignUInt16(1);
    delta_minus->ShiftLeft(exponent);
  }
}

static void InitialScaledStartValuesNegativeExponentPositivePower(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus)
{
  numerator->AssignUInt64(significand);
  denominator->AssignPowerUInt16(10, estimated_power);
  denominator->ShiftLeft(-exponent);

  if (need_boundary_deltas) {
    denominator->ShiftLeft(1);
    numerator->ShiftLeft(1);
    delta_plus->AssignUInt16(1);
    delta_minus->AssignUInt16(1);
  }
}

static void InitialScaledStartValuesNegativeExponentNegativePower(
    uint64_t significand, int exponent,
    int estimated_power, bool need_boundary_deltas,
    Bignum* numerator, Bignum* denominator,
    Bignum* delta_minus, Bignum* delta_plus)
{
  Bignum* power_ten = numerator;
  power_ten->AssignPowerUInt16(10, -estimated_power);

  if (need_boundary_deltas) {
    delta_plus->AssignBignum(*power_ten);
    delta_minus->AssignBignum(*power_ten);
  }

  ASSERT(numerator == power_ten);
  numerator->MultiplyByUInt64(significand);

  denominator->AssignUInt16(1);
  denominator->ShiftLeft(-exponent);

  if (need_boundary_deltas) {
    numerator->ShiftLeft(1);
    denominator->ShiftLeft(1);
  }
}

static void InitialScaledStartValues(uint64_t significand,
                                     int exponent,
                                     bool lower_boundary_is_closer,
                                     int estimated_power,
                                     bool need_boundary_deltas,
                                     Bignum* numerator,
                                     Bignum* denominator,
                                     Bignum* delta_minus,
                                     Bignum* delta_plus)
{
  if (exponent >= 0) {
    InitialScaledStartValuesPositiveExponent(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  } else if (estimated_power >= 0) {
    InitialScaledStartValuesNegativeExponentPositivePower(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  } else {
    InitialScaledStartValuesNegativeExponentNegativePower(
        significand, exponent, estimated_power, need_boundary_deltas,
        numerator, denominator, delta_minus, delta_plus);
  }

  if (need_boundary_deltas && lower_boundary_is_closer) {
    denominator->ShiftLeft(1);  // *2
    numerator->ShiftLeft(1);    // *2
    delta_plus->ShiftLeft(1);   // *2
  }
}

static void FixupMultiply10(int estimated_power, bool is_even,
                            int* decimal_point,
                            Bignum* numerator, Bignum* denominator,
                            Bignum* delta_minus, Bignum* delta_plus) {
  bool in_range;
  if (is_even) {
    in_range = Bignum::PlusCompare(*numerator, *delta_plus, *denominator) >= 0;
  } else {
    in_range = Bignum::PlusCompare(*numerator, *delta_plus, *denominator) > 0;
  }
  if (in_range) {
    *decimal_point = estimated_power + 1;
  } else {
    *decimal_point = estimated_power;
    numerator->Times10();
    if (Bignum::Equal(*delta_minus, *delta_plus)) {
      delta_minus->Times10();
      delta_plus->AssignBignum(*delta_minus);
    } else {
      delta_minus->Times10();
      delta_plus->Times10();
    }
  }
}

enum FastDtoaMode {
  FAST_DTOA_SHORTEST,
  FAST_DTOA_PRECISION
};

bool FastDtoa(double d,
              FastDtoaMode mode,
              int requested_digits,
              Vector<char> buffer,
              int* length,
              int* decimal_point);

static const int kMinimalTargetExponent = -60;
static const int kMaximalTargetExponent = -32;

static bool RoundWeed(Vector<char> buffer, int length,
                      uint64_t distance_too_high_w, uint64_t unsafe_interval,
                      uint64_t rest, uint64_t ten_kappa, uint64_t unit)
{
  uint64_t small_distance = distance_too_high_w - unit;
  uint64_t big_distance = distance_too_high_w + unit;

  ASSERT(rest <= unsafe_interval);
  while (rest < small_distance &&  // Negated condition 1
         unsafe_interval - rest >= ten_kappa &&  // Negated condition 2
         (rest + ten_kappa < small_distance ||  // buffer{-1} > w_high
          small_distance - rest >= rest + ten_kappa - small_distance)) {
    buffer[length - 1]--;
    rest += ten_kappa;
  }

  if (rest < big_distance &&
      unsafe_interval - rest >= ten_kappa &&
      (rest + ten_kappa < big_distance ||
       big_distance - rest > rest + ten_kappa - big_distance)) {
    return false;
  }

  return (2 * unit <= rest) && (rest <= unsafe_interval - 4 * unit);
}

static bool RoundWeedCounted(Vector<char> buffer, int length,
                             uint64_t rest, uint64_t ten_kappa, uint64_t unit,
                             int* kappa)
{
  ASSERT(rest < ten_kappa);

  if (unit >= ten_kappa) return false;
  if (ten_kappa - unit <= unit) return false;
  if ((ten_kappa - rest > rest) && (ten_kappa - 2 * rest >= 2 * unit)) {
    return true;
  }

  if ((rest > unit) && (ten_kappa - (rest - unit) <= (rest - unit))) {
    buffer[length - 1]++;
    for (int i = length - 1; i > 0; --i) {
      if (buffer[i] != '0' + 10) break;
      buffer[i] = '0';
      buffer[i - 1]++;
    }
    if (buffer[0] == '0' + 10) {
      buffer[0] = '1';
      (*kappa) += 1;
    }
    return true;
  }
  return false;
}

static unsigned int const kSmallPowersOfTen[] =
    {0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
     1000000000};

static void BiggestPowerTen(uint32_t number,
                            int number_bits,
                            uint32_t* power,
                            int* exponent_plus_one) {
  ASSERT(number < (1u << (number_bits + 1)));

  int exponent_plus_one_guess = ((number_bits + 1) * 1233 >> 12);
  exponent_plus_one_guess++;

  if (number < kSmallPowersOfTen[exponent_plus_one_guess]) {
    exponent_plus_one_guess--;
  }
  *power = kSmallPowersOfTen[exponent_plus_one_guess];
  *exponent_plus_one = exponent_plus_one_guess;
}

static bool DigitGen(DiyFp low, DiyFp w, DiyFp high, Vector<char> buffer,
                     int* length, int* kappa)
{
  ASSERT(low.e() == w.e() && w.e() == high.e());
  ASSERT(low.f() + 1 <= high.f() - 1);
  ASSERT(kMinimalTargetExponent <= w.e() && w.e() <= kMaximalTargetExponent);

  uint64_t unit = 1;
  DiyFp too_low = DiyFp(low.f() - unit, low.e());
  DiyFp too_high = DiyFp(high.f() + unit, high.e());
  DiyFp unsafe_interval = DiyFp::Minus(too_high, too_low);
  DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.e(), w.e());

  uint32_t integrals = static_cast<uint32_t>(too_high.f() >> -one.e());
  uint64_t fractionals = too_high.f() & (one.f() - 1);
  uint32_t divisor;
  int divisor_exponent_plus_one;
  BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.e()),
                  &divisor, &divisor_exponent_plus_one);
  *kappa = divisor_exponent_plus_one;
  *length = 0;

  while (*kappa > 0) {
    int digit = integrals / divisor;
    ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    integrals %= divisor;
    (*kappa)--;

    uint64_t rest =
        (static_cast<uint64_t>(integrals) << -one.e()) + fractionals;

    if (rest < unsafe_interval.f()) {
      return RoundWeed(buffer, *length, DiyFp::Minus(too_high, w).f(),
                       unsafe_interval.f(), rest,
                       static_cast<uint64_t>(divisor) << -one.e(), unit);
    }
    divisor /= 10;
  }

  ASSERT(one.e() >= -60);
  ASSERT(fractionals < one.f());
  ASSERT(UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF) / 10 >= one.f());

  for (;;) {
    fractionals *= 10;
    unit *= 10;
    unsafe_interval.set_f(unsafe_interval.f() * 10);
    // Integer division by one.
    int digit = static_cast<int>(fractionals >> -one.e());
    ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    fractionals &= one.f() - 1;  // Modulo by one.
    (*kappa)--;
    if (fractionals < unsafe_interval.f()) {
      return RoundWeed(buffer, *length, DiyFp::Minus(too_high, w).f() * unit,
                       unsafe_interval.f(), fractionals, one.f(), unit);
    }
  }
}

static bool DigitGenCounted(DiyFp w, int requested_digits, Vector<char> buffer,
                            int* length, int* kappa)
{
  ASSERT(kMinimalTargetExponent <= w.e() && w.e() <= kMaximalTargetExponent);
  ASSERT(kMinimalTargetExponent >= -60);
  ASSERT(kMaximalTargetExponent <= -32);

  uint64_t w_error = 1;
  DiyFp one = DiyFp(static_cast<uint64_t>(1) << -w.e(), w.e());
  uint32_t integrals = static_cast<uint32_t>(w.f() >> -one.e());
  uint64_t fractionals = w.f() & (one.f() - 1);
  uint32_t divisor;
  int divisor_exponent_plus_one;
  BiggestPowerTen(integrals, DiyFp::kSignificandSize - (-one.e()),
                  &divisor, &divisor_exponent_plus_one);
  *kappa = divisor_exponent_plus_one;
  *length = 0;

  while (*kappa > 0) {
    int digit = integrals / divisor;
    ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    requested_digits--;
    integrals %= divisor;
    (*kappa)--;
    if (requested_digits == 0) break;
    divisor /= 10;
  }

  if (requested_digits == 0) {
    uint64_t rest =
        (static_cast<uint64_t>(integrals) << -one.e()) + fractionals;
    return RoundWeedCounted(buffer, *length, rest,
                            static_cast<uint64_t>(divisor) << -one.e(), w_error,
                            kappa);
  }

  ASSERT(one.e() >= -60);
  ASSERT(fractionals < one.f());
  ASSERT(UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF) / 10 >= one.f());

  while (requested_digits > 0 && fractionals > w_error) {
    fractionals *= 10;
    w_error *= 10;
    // Integer division by one.
    int digit = static_cast<int>(fractionals >> -one.e());
    ASSERT(digit <= 9);
    buffer[*length] = static_cast<char>('0' + digit);
    (*length)++;
    requested_digits--;
    fractionals &= one.f() - 1;  // Modulo by one.
    (*kappa)--;
  }
  if (requested_digits != 0) return false;
  return RoundWeedCounted(buffer, *length, fractionals, one.f(), w_error,
                          kappa);
}

static bool Grisu3(double v, FastDtoaMode mode, Vector<char> buffer,
                   int* length, int* decimal_exponent)
{
  DiyFp w = Double(v).AsNormalizedDiyFp();
  DiyFp boundary_minus, boundary_plus;

  ASSERT(mode == FAST_DTOA_SHORTEST);
  Double(v).NormalizedBoundaries(&boundary_minus, &boundary_plus);

  ASSERT(boundary_plus.e() == w.e());
  DiyFp ten_mk;  // Cached power of ten: 10^-k
  int mk;        // -k
  int ten_mk_minimal_binary_exponent =
     kMinimalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  int ten_mk_maximal_binary_exponent =
     kMaximalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  PowersOfTenCache::GetCachedPowerForBinaryExponentRange(
      ten_mk_minimal_binary_exponent,
      ten_mk_maximal_binary_exponent,
      &ten_mk, &mk);

  ASSERT((kMinimalTargetExponent <= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize) &&
         (kMaximalTargetExponent >= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize));

  DiyFp scaled_w = DiyFp::Times(w, ten_mk);
  ASSERT(scaled_w.e() ==
         boundary_plus.e() + ten_mk.e() + DiyFp::kSignificandSize);

  DiyFp scaled_boundary_minus = DiyFp::Times(boundary_minus, ten_mk);
  DiyFp scaled_boundary_plus  = DiyFp::Times(boundary_plus,  ten_mk);

  int kappa;
  bool result = DigitGen(scaled_boundary_minus, scaled_w, scaled_boundary_plus,
                         buffer, length, &kappa);
  *decimal_exponent = -mk + kappa;
  return result;
}

static bool Grisu3Counted(double v, int requested_digits, Vector<char> buffer,
                          int* length, int* decimal_exponent)
{
  DiyFp w = Double(v).AsNormalizedDiyFp();
  DiyFp ten_mk;  // Cached power of ten: 10^-k
  int mk;        // -k
  int ten_mk_minimal_binary_exponent =
     kMinimalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  int ten_mk_maximal_binary_exponent =
     kMaximalTargetExponent - (w.e() + DiyFp::kSignificandSize);
  PowersOfTenCache::GetCachedPowerForBinaryExponentRange(
      ten_mk_minimal_binary_exponent,
      ten_mk_maximal_binary_exponent,
      &ten_mk, &mk);
  ASSERT((kMinimalTargetExponent <= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize) &&
         (kMaximalTargetExponent >= w.e() + ten_mk.e() +
          DiyFp::kSignificandSize));

  DiyFp scaled_w = DiyFp::Times(w, ten_mk);

  int kappa;
  bool result = DigitGenCounted(scaled_w, requested_digits,
                                buffer, length, &kappa);
  *decimal_exponent = -mk + kappa;
  return result;
}


bool FastDtoa(double v,
              FastDtoaMode mode,
              int requested_digits,
              Vector<char> buffer,
              int* length,
              int* decimal_point) {
  ASSERT(v > 0);
  ASSERT(!Double(v).IsSpecial());

  bool result = false;
  int decimal_exponent = 0;
  switch (mode) {
    case FAST_DTOA_SHORTEST:
      result = Grisu3(v, mode, buffer, length, &decimal_exponent);
      break;
    case FAST_DTOA_PRECISION:
      result = Grisu3Counted(v, requested_digits,
                             buffer, length, &decimal_exponent);
      break;
    default:
      UNREACHABLE();
  }
  if (result) {
    *decimal_point = *length + decimal_exponent;
    buffer[*length] = '\0';
  }
  return result;
}

// Represents a 128bit type. This class should be replaced by a native type on
// platforms that support 128bit integers.
class UInt128 {
 public:
  UInt128() : high_bits_(0), low_bits_(0) { }
  UInt128(uint64_t high, uint64_t low) : high_bits_(high), low_bits_(low) { }

  void Multiply(uint32_t multiplicand) {
    uint64_t accumulator;

    accumulator = (low_bits_ & kMask32) * multiplicand;
    uint32_t part = static_cast<uint32_t>(accumulator & kMask32);
    accumulator >>= 32;
    accumulator = accumulator + (low_bits_ >> 32) * multiplicand;
    low_bits_ = (accumulator << 32) + part;
    accumulator >>= 32;
    accumulator = accumulator + (high_bits_ & kMask32) * multiplicand;
    part = static_cast<uint32_t>(accumulator & kMask32);
    accumulator >>= 32;
    accumulator = accumulator + (high_bits_ >> 32) * multiplicand;
    high_bits_ = (accumulator << 32) + part;
    ASSERT((accumulator >> 32) == 0);
  }

  void Shift(int shift_amount) {
    ASSERT(-64 <= shift_amount && shift_amount <= 64);
    if (shift_amount == 0) {
      return;
    } else if (shift_amount == -64) {
      high_bits_ = low_bits_;
      low_bits_ = 0;
    } else if (shift_amount == 64) {
      low_bits_ = high_bits_;
      high_bits_ = 0;
    } else if (shift_amount <= 0) {
      high_bits_ <<= -shift_amount;
      high_bits_ += low_bits_ >> (64 + shift_amount);
      low_bits_ <<= -shift_amount;
    } else {
      low_bits_ >>= shift_amount;
      low_bits_ += high_bits_ << (64 - shift_amount);
      high_bits_ >>= shift_amount;
    }
  }

  // Modifies *this to *this MOD (2^power).
  // Returns *this DIV (2^power).
  int DivModPowerOf2(int power) {
    if (power >= 64) {
      int result = static_cast<int>(high_bits_ >> (power - 64));
      high_bits_ -= static_cast<uint64_t>(result) << (power - 64);
      return result;
    } else {
      uint64_t part_low = low_bits_ >> power;
      uint64_t part_high = high_bits_ << (64 - power);
      int result = static_cast<int>(part_low + part_high);
      high_bits_ = 0;
      low_bits_ -= part_low << power;
      return result;
    }
  }

  bool IsZero() const {
    return high_bits_ == 0 && low_bits_ == 0;
  }

  int BitAt(int position) {
    if (position >= 64) {
      return static_cast<int>(high_bits_ >> (position - 64)) & 1;
    } else {
      return static_cast<int>(low_bits_ >> position) & 1;
    }
  }

 private:
  static const uint64_t kMask32 = 0xFFFFFFFF;
  // Value == (high_bits_ << 64) + low_bits_
  uint64_t high_bits_;
  uint64_t low_bits_;
};


static const int kDoubleSignificandSize = 53;  // Includes the hidden bit.


static void FillDigits32FixedLength(uint32_t number, int requested_length,
                                    Vector<char> buffer, int* length) {
  for (int i = requested_length - 1; i >= 0; --i) {
    buffer[(*length) + i] = '0' + number % 10;
    number /= 10;
  }
  *length += requested_length;
}


static void FillDigits32(uint32_t number, Vector<char> buffer, int* length) {
  int number_length = 0;
  // We fill the digits in reverse order and exchange them afterwards.
  while (number != 0) {
    int digit = number % 10;
    number /= 10;
    buffer[(*length) + number_length] = static_cast<char>('0' + digit);
    number_length++;
  }
  // Exchange the digits.
  int i = *length;
  int j = *length + number_length - 1;
  while (i < j) {
    char tmp = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = tmp;
    i++;
    j--;
  }
  *length += number_length;
}


static void FillDigits64FixedLength(uint64_t number,
                                    Vector<char> buffer, int* length) {
  const uint32_t kTen7 = 10000000;
  // For efficiency cut the number into 3 uint32_t parts, and print those.
  uint32_t part2 = static_cast<uint32_t>(number % kTen7);
  number /= kTen7;
  uint32_t part1 = static_cast<uint32_t>(number % kTen7);
  uint32_t part0 = static_cast<uint32_t>(number / kTen7);

  FillDigits32FixedLength(part0, 3, buffer, length);
  FillDigits32FixedLength(part1, 7, buffer, length);
  FillDigits32FixedLength(part2, 7, buffer, length);
}


static void FillDigits64(uint64_t number, Vector<char> buffer, int* length) {
  const uint32_t kTen7 = 10000000;
  // For efficiency cut the number into 3 uint32_t parts, and print those.
  uint32_t part2 = static_cast<uint32_t>(number % kTen7);
  number /= kTen7;
  uint32_t part1 = static_cast<uint32_t>(number % kTen7);
  uint32_t part0 = static_cast<uint32_t>(number / kTen7);

  if (part0 != 0) {
    FillDigits32(part0, buffer, length);
    FillDigits32FixedLength(part1, 7, buffer, length);
    FillDigits32FixedLength(part2, 7, buffer, length);
  } else if (part1 != 0) {
    FillDigits32(part1, buffer, length);
    FillDigits32FixedLength(part2, 7, buffer, length);
  } else {
    FillDigits32(part2, buffer, length);
  }
}


static void RoundUp(Vector<char> buffer, int* length, int* decimal_point) {
  // An empty buffer represents 0.
  if (*length == 0) {
    buffer[0] = '1';
    *decimal_point = 1;
    *length = 1;
    return;
  }

  buffer[(*length) - 1]++;
  for (int i = (*length) - 1; i > 0; --i) {
    if (buffer[i] != '0' + 10) {
      return;
    }
    buffer[i] = '0';
    buffer[i - 1]++;
  }

  if (buffer[0] == '0' + 10) {
    buffer[0] = '1';
    (*decimal_point)++;
  }
}

static void FillFractionals(uint64_t fractionals, int exponent,
                            int fractional_count, Vector<char> buffer,
                            int* length, int* decimal_point)
{
  ASSERT(-128 <= exponent && exponent <= 0);

  if (-exponent <= 64) {
    ASSERT(fractionals >> 56 == 0);
    int point = -exponent;
    for (int i = 0; i < fractional_count; ++i) {
      if (fractionals == 0) break;
      fractionals *= 5;
      point--;
      int digit = static_cast<int>(fractionals >> point);
      ASSERT(digit <= 9);
      buffer[*length] = static_cast<char>('0' + digit);
      (*length)++;
      fractionals -= static_cast<uint64_t>(digit) << point;
    }

    if (((fractionals >> (point - 1)) & 1) == 1) {
      RoundUp(buffer, length, decimal_point);
    }
  } else {  // We need 128 bits.
    ASSERT(64 < -exponent && -exponent <= 128);
    UInt128 fractionals128 = UInt128(fractionals, 0);
    fractionals128.Shift(-exponent - 64);
    int point = 128;
    for (int i = 0; i < fractional_count; ++i) {
      if (fractionals128.IsZero()) break;
      fractionals128.Multiply(5);
      point--;
      int digit = fractionals128.DivModPowerOf2(point);
      ASSERT(digit <= 9);
      buffer[*length] = static_cast<char>('0' + digit);
      (*length)++;
    }
    if (fractionals128.BitAt(point - 1) == 1) {
      RoundUp(buffer, length, decimal_point);
    }
  }
}


// Removes leading and trailing zeros.
// If leading zeros are removed then the decimal point position is adjusted.
static void TrimZeros(Vector<char> buffer, int* length, int* decimal_point) {
  while (*length > 0 && buffer[(*length) - 1] == '0') {
    (*length)--;
  }
  int first_non_zero = 0;
  while (first_non_zero < *length && buffer[first_non_zero] == '0') {
    first_non_zero++;
  }
  if (first_non_zero != 0) {
    for (int i = first_non_zero; i < *length; ++i) {
      buffer[i - first_non_zero] = buffer[i];
    }
    *length -= first_non_zero;
    *decimal_point -= first_non_zero;
  }
}


bool FastFixedDtoa(double v,
                   int fractional_count,
                   Vector<char> buffer,
                   int* length,
                   int* decimal_point) {
  const uint32_t kMaxUInt32 = 0xFFFFFFFF;
  uint64_t significand = Double(v).Significand();
  int exponent = Double(v).Exponent();

  if (exponent > 20) return false;
  if (fractional_count > 20) return false;
  *length = 0;

  if (exponent + kDoubleSignificandSize > 64) {
    const uint64_t kFive17 = UINT64_2PART_C(0xB1, A2BC2EC5);  // 5^17
    uint64_t divisor = kFive17;
    int divisor_power = 17;
    uint64_t dividend = significand;
    uint32_t quotient;
    uint64_t remainder;

    if (exponent > divisor_power) {
      dividend <<= exponent - divisor_power;
      quotient = static_cast<uint32_t>(dividend / divisor);
      remainder = (dividend % divisor) << divisor_power;
    } else {
      divisor <<= divisor_power - exponent;
      quotient = static_cast<uint32_t>(dividend / divisor);
      remainder = (dividend % divisor) << exponent;
    }
    FillDigits32(quotient, buffer, length);
    FillDigits64FixedLength(remainder, buffer, length);
    *decimal_point = *length;
  } else if (exponent >= 0) {
    // 0 <= exponent <= 11
    significand <<= exponent;
    FillDigits64(significand, buffer, length);
    *decimal_point = *length;
  } else if (exponent > -kDoubleSignificandSize) {
    uint64_t integrals = significand >> -exponent;
    uint64_t fractionals = significand - (integrals << -exponent);
    if (integrals > kMaxUInt32) {
      FillDigits64(integrals, buffer, length);
    } else {
      FillDigits32(static_cast<uint32_t>(integrals), buffer, length);
    }
    *decimal_point = *length;
    FillFractionals(fractionals, exponent, fractional_count,
                    buffer, length, decimal_point);
  } else if (exponent < -128) {
    // This configuration (with at most 20 digits) means that all digits must be
    // 0.
    ASSERT(fractional_count <= 20);
    buffer[0] = '\0';
    *length = 0;
    *decimal_point = -fractional_count;
  } else {
    *decimal_point = 0;
    FillFractionals(significand, exponent, fractional_count,
                    buffer, length, decimal_point);
  }
  TrimZeros(buffer, length, decimal_point);
  buffer[*length] = '\0';
  if ((*length) == 0) {
    *decimal_point = -fractional_count;
  }
  return true;
}

static const int kMaxUint64DecimalDigits = 19;

static const int kMaxDecimalPower = 309;
static const int kMinDecimalPower = -324;

// 2^64 = 18446744073709551616
static const uint64_t kMaxUint64 = UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF);
static const int kMaxSignificantDecimalDigits = 780;

static Vector<const char> TrimLeadingZeros(Vector<const char> buffer) {
  for (int i = 0; i < buffer.length(); i++) {
    if (buffer[i] != '0') {
      return buffer.SubVector(i, buffer.length());
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static Vector<const char> TrimTrailingZeros(Vector<const char> buffer) {
  for (int i = buffer.length() - 1; i >= 0; --i) {
    if (buffer[i] != '0') {
      return buffer.SubVector(0, i + 1);
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static void CutToMaxSignificantDigits(Vector<const char> buffer,
                                       int exponent,
                                       char* significant_buffer,
                                       int* significant_exponent) {
  for (int i = 0; i < kMaxSignificantDecimalDigits - 1; ++i) {
    significant_buffer[i] = buffer[i];
  }

  ASSERT(buffer[buffer.length() - 1] != '0');

  significant_buffer[kMaxSignificantDecimalDigits - 1] = '1';
  *significant_exponent =
      exponent + (buffer.length() - kMaxSignificantDecimalDigits);
}

static void TrimAndCut(Vector<const char> buffer, int exponent,
                       char* buffer_copy_space, int space_size,
                       Vector<const char>* trimmed, int* updated_exponent) {
  Vector<const char> left_trimmed = TrimLeadingZeros(buffer);
  Vector<const char> right_trimmed = TrimTrailingZeros(left_trimmed);
  exponent += left_trimmed.length() - right_trimmed.length();
  if (right_trimmed.length() > kMaxSignificantDecimalDigits) {
    (void) space_size;  // Mark variable as used.
    ASSERT(space_size >= kMaxSignificantDecimalDigits);
    CutToMaxSignificantDigits(right_trimmed, exponent,
                              buffer_copy_space, updated_exponent);
    *trimmed = Vector<const char>(buffer_copy_space,
                                 kMaxSignificantDecimalDigits);
  } else {
    *trimmed = right_trimmed;
    *updated_exponent = exponent;
  }
}

static uint64_t ReadUint64(Vector<const char> buffer,
                           int* number_of_read_digits) {
  uint64_t result = 0;
  int i = 0;
  while (i < buffer.length() && result <= (kMaxUint64 / 10 - 1)) {
    int digit = buffer[i++] - '0';
    ASSERT(0 <= digit && digit <= 9);
    result = 10 * result + digit;
  }
  *number_of_read_digits = i;
  return result;
}

static void ReadDiyFp(Vector<const char> buffer,
                      DiyFp* result,
                      int* remaining_decimals) {
  int read_digits;
  uint64_t significand = ReadUint64(buffer, &read_digits);
  if (buffer.length() == read_digits) {
    *result = DiyFp(significand, 0);
    *remaining_decimals = 0;
  } else {
    // Round the significand.
    if (buffer[read_digits] >= '5') {
      significand++;
    }
    // Compute the binary exponent.
    int exponent = 0;
    *result = DiyFp(significand, exponent);
    *remaining_decimals = buffer.length() - read_digits;
  }
}

static DiyFp AdjustmentPowerOfTen(int exponent) {
  ASSERT(0 < exponent);
  ASSERT(exponent < PowersOfTenCache::kDecimalExponentDistance);
  // Simply hardcode the remaining powers for the given decimal exponent
  // distance.
  ASSERT(PowersOfTenCache::kDecimalExponentDistance == 8);
  switch (exponent) {
    case 1: return DiyFp(UINT64_2PART_C(0xa0000000, 00000000), -60);
    case 2: return DiyFp(UINT64_2PART_C(0xc8000000, 00000000), -57);
    case 3: return DiyFp(UINT64_2PART_C(0xfa000000, 00000000), -54);
    case 4: return DiyFp(UINT64_2PART_C(0x9c400000, 00000000), -50);
    case 5: return DiyFp(UINT64_2PART_C(0xc3500000, 00000000), -47);
    case 6: return DiyFp(UINT64_2PART_C(0xf4240000, 00000000), -44);
    case 7: return DiyFp(UINT64_2PART_C(0x98968000, 00000000), -40);
    default:
      UNREACHABLE();
  }
}

static bool DiyFpStrtod(Vector<const char> buffer,
                        int exponent,
                        double* result) {
  DiyFp input;
  int remaining_decimals;
  ReadDiyFp(buffer, &input, &remaining_decimals);

  const int kDenominatorLog = 3;
  const int kDenominator = 1 << kDenominatorLog;
  // Move the remaining decimals into the exponent.
  exponent += remaining_decimals;
  int error = (remaining_decimals == 0 ? 0 : kDenominator / 2);

  int old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  ASSERT(exponent <= PowersOfTenCache::kMaxDecimalExponent);
  if (exponent < PowersOfTenCache::kMinDecimalExponent) {
    *result = 0.0;
    return true;
  }
  DiyFp cached_power;
  int cached_decimal_exponent;
  PowersOfTenCache::GetCachedPowerForDecimalExponent(exponent,
                                                     &cached_power,
                                                     &cached_decimal_exponent);

  if (cached_decimal_exponent != exponent) {
    int adjustment_exponent = exponent - cached_decimal_exponent;
    DiyFp adjustment_power = AdjustmentPowerOfTen(adjustment_exponent);
    input.Multiply(adjustment_power);
    if (kMaxUint64DecimalDigits - buffer.length() >= adjustment_exponent) {
      // The product of input with the adjustment power fits into a 64 bit
      // integer.
      ASSERT(DiyFp::kSignificandSize == 64);
    } else {
      // The adjustment power is exact. There is hence only an error of 0.5.
      error += kDenominator / 2;
    }
  }

  input.Multiply(cached_power);

  int error_b = kDenominator / 2;
  int error_ab = (error == 0 ? 0 : 1);  // We round up to 1.
  int fixed_error = kDenominator / 2;
  error += error_b + error_ab + fixed_error;

  old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  int order_of_magnitude = DiyFp::kSignificandSize + input.e();
  int effective_significand_size =
      Double::SignificandSizeForOrderOfMagnitude(order_of_magnitude);
  int precision_digits_count =
      DiyFp::kSignificandSize - effective_significand_size;
  if (precision_digits_count + kDenominatorLog >= DiyFp::kSignificandSize) {
    int shift_amount = (precision_digits_count + kDenominatorLog) -
        DiyFp::kSignificandSize + 1;
    input.set_f(input.f() >> shift_amount);
    input.set_e(input.e() + shift_amount);
    error = (error >> shift_amount) + 1 + kDenominator;
    precision_digits_count -= shift_amount;
  }

  ASSERT(DiyFp::kSignificandSize == 64);
  ASSERT(precision_digits_count < 64);
  uint64_t one64 = 1;
  uint64_t precision_bits_mask = (one64 << precision_digits_count) - 1;
  uint64_t precision_bits = input.f() & precision_bits_mask;
  uint64_t half_way = one64 << (precision_digits_count - 1);
  precision_bits *= kDenominator;
  half_way *= kDenominator;
  DiyFp rounded_input(input.f() >> precision_digits_count,
                      input.e() + precision_digits_count);
  if (precision_bits >= half_way + error) {
    rounded_input.set_f(rounded_input.f() + 1);
  }

  *result = Double(rounded_input).value();
  if (half_way - error < precision_bits && precision_bits < half_way + error) {
    return false;
  } else {
    return true;
  }
}

static int CompareBufferWithDiyFp(Vector<const char> buffer,
                                  int exponent,
                                  DiyFp diy_fp) {
  ASSERT(buffer.length() + exponent <= kMaxDecimalPower + 1);
  ASSERT(buffer.length() + exponent > kMinDecimalPower);
  ASSERT(buffer.length() <= kMaxSignificantDecimalDigits);
  ASSERT(((kMaxDecimalPower + 1) * 333 / 100) < Bignum::kMaxSignificantBits);

  Bignum buffer_bignum;
  Bignum diy_fp_bignum;
  buffer_bignum.AssignDecimalString(buffer);
  diy_fp_bignum.AssignUInt64(diy_fp.f());
  if (exponent >= 0) {
    buffer_bignum.MultiplyByPowerOfTen(exponent);
  } else {
    diy_fp_bignum.MultiplyByPowerOfTen(-exponent);
  }
  if (diy_fp.e() > 0) {
    diy_fp_bignum.ShiftLeft(diy_fp.e());
  } else {
    buffer_bignum.ShiftLeft(-diy_fp.e());
  }
  return Bignum::Compare(buffer_bignum, diy_fp_bignum);
}

static bool ComputeGuess(Vector<const char> trimmed, int exponent,
                         double* guess)
{
  if (trimmed.length() == 0) {
    *guess = 0.0;
    return true;
  }
  if (exponent + trimmed.length() - 1 >= kMaxDecimalPower) {
    *guess = Double::Infinity();
    return true;
  }
  if (exponent + trimmed.length() <= kMinDecimalPower) {
    *guess = 0.0;
    return true;
  }

  if (DiyFpStrtod(trimmed, exponent, guess)) {
    return true;
  }
  if (*guess == Double::Infinity()) {
    return true;
  }
  return false;
}

double Strtod(Vector<const char> buffer, int exponent)
{
  char copy_buffer[kMaxSignificantDecimalDigits];
  Vector<const char> trimmed;
  int updated_exponent;
  TrimAndCut(buffer, exponent, copy_buffer, kMaxSignificantDecimalDigits,
             &trimmed, &updated_exponent);
  exponent = updated_exponent;

  double guess;
  bool is_correct = ComputeGuess(trimmed, exponent, &guess);
  if (is_correct) return guess;

  DiyFp upper_boundary = Double(guess).UpperBoundary();
  int comparison = CompareBufferWithDiyFp(trimmed, exponent, upper_boundary);
  if (comparison < 0) {
    return guess;
  } else if (comparison > 0) {
    return Double(guess).NextDouble();
  } else if ((Double(guess).Significand() & 1) == 0) {
    // Round towards even.
    return guess;
  } else {
    return Double(guess).NextDouble();
  }
}

class DoubleToStringConverter {
public:
  static const int kMaxFixedDigitsBeforePoint = 60;
  static const int kMaxFixedDigitsAfterPoint = 60;
  static const int kMaxExponentialDigits = 120;
  static const int kMinPrecisionDigits = 1;
  static const int kMaxPrecisionDigits = 120;

  enum Flags {
    NO_FLAGS = 0,
    EMIT_POSITIVE_EXPONENT_SIGN = 1,
    EMIT_TRAILING_DECIMAL_POINT = 2,
    EMIT_TRAILING_ZERO_AFTER_POINT = 4,
    UNIQUE_ZERO = 8
  };

  DoubleToStringConverter(int flags,
                          const char* infinity_symbol,
                          const char* nan_symbol,
                          char exponent_character,
                          int decimal_in_shortest_low,
                          int decimal_in_shortest_high,
                          int max_leading_padding_zeroes_in_precision_mode,
                          int max_trailing_padding_zeroes_in_precision_mode)
      : flags_(flags),
        infinity_symbol_(infinity_symbol),
        nan_symbol_(nan_symbol),
        exponent_character_(exponent_character),
        decimal_in_shortest_low_(decimal_in_shortest_low),
        decimal_in_shortest_high_(decimal_in_shortest_high),
        max_leading_padding_zeroes_in_precision_mode_(
            max_leading_padding_zeroes_in_precision_mode),
        max_trailing_padding_zeroes_in_precision_mode_(
            max_trailing_padding_zeroes_in_precision_mode) {
    // When 'trailing zero after the point' is set, then 'trailing point'
    // must be set too.
    ASSERT(((flags & EMIT_TRAILING_DECIMAL_POINT) != 0) ||
        !((flags & EMIT_TRAILING_ZERO_AFTER_POINT) != 0));
  }

  bool ToShortest(double value, std::string &s) const {
    return ToShortestIeeeNumber(value, s, SHORTEST);
  }

  bool ToFixed(double value,
               int requested_digits,
               std::string &s) const;

  bool ToExponential(double value,
                     int requested_digits,
                     std::string &s) const;

  bool ToPrecision(double value,
                   int precision,
                   std::string &s) const;

  enum DtoaMode {
    SHORTEST,
    FIXED, // Produce a fixed number of digits after the decimal point
    PRECISION // Fixed number of digits (independent of the decimal point)
  };

  static const int kBase10MaximalLength = 17;

  // The result should be interpreted as buffer * 10^(point-length).
  static void DoubleToAscii(double v,
                            DtoaMode mode,
                            int requested_digits,
                            char* buffer,
                            int buffer_length,
                            bool* sign,
                            int* length,
                            int* point);

 private:
  // Implementation for ToShortest.
  bool ToShortestIeeeNumber(double value,
                            std::string &s,
                            DtoaMode mode) const;

  bool HandleSpecialValues(double value, std::string &s) const;

  void CreateExponentialRepresentation(const char* decimal_digits,
                                       int length,
                                       int exponent,
                                       std::string &s) const;

  void CreateDecimalRepresentation(const char* decimal_digits,
                                   int length,
                                   int decimal_point,
                                   int digits_after_point,
                                   std::string &s) const;

  const int flags_;
  const char* const infinity_symbol_;
  const char* const nan_symbol_;
  const char exponent_character_;
  const int decimal_in_shortest_low_;
  const int decimal_in_shortest_high_;
  const int max_leading_padding_zeroes_in_precision_mode_;
  const int max_trailing_padding_zeroes_in_precision_mode_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(DoubleToStringConverter);
};


class StringToDoubleConverter {
 public:
  enum Flags {
    NO_FLAGS = 0,
    ALLOW_HEX = 1,
    ALLOW_OCTALS = 2,
    ALLOW_TRAILING_JUNK = 4,
    ALLOW_LEADING_SPACES = 8,
    ALLOW_TRAILING_SPACES = 16,
    ALLOW_SPACES_AFTER_SIGN = 32
  };

  StringToDoubleConverter(int flags,
                          double empty_string_value,
                          double junk_string_value,
                          const char* infinity_symbol,
                          const char* nan_symbol)
      : flags_(flags),
        empty_string_value_(empty_string_value),
        junk_string_value_(junk_string_value),
        infinity_symbol_(infinity_symbol),
        nan_symbol_(nan_symbol) {
  }

  double StringToDouble(const char* buffer,
                        int length,
                        int* processed_characters_count) const;

 private:
  const int flags_;
  const double empty_string_value_;
  const double junk_string_value_;
  const char* const infinity_symbol_;
  const char* const nan_symbol_;

  double StringToIeee(const char *start_pointer,
                      int length,
                      int* processed_characters_count) const;

  DISALLOW_IMPLICIT_CONSTRUCTORS(StringToDoubleConverter);
};

bool DoubleToStringConverter::HandleSpecialValues(
    double value,
    std::string &result) const {
  Double double_inspect(value);
  if (double_inspect.IsInfinite()) {
    if (infinity_symbol_ == NULL) return false;
    if (value < 0) {
      result += '-';
    }
    result += infinity_symbol_;
    return true;
  }
  if (double_inspect.IsNan()) {
    if (nan_symbol_ == NULL) return false;
    result = nan_symbol_;
    return true;
  }
  return false;
}


void DoubleToStringConverter::CreateExponentialRepresentation(
    const char* decimal_digits,
    int length,
    int exponent,
    std::string &result) const {
  ASSERT(length != 0);
  result += decimal_digits[0];
  if (length != 1) {
    result += '.';
    result.append(&decimal_digits[1], length-1);
  }
  result += exponent_character_;
  if (exponent < 0) {
    result += '-';
    exponent = -exponent;
  } else {
    if ((flags_ & EMIT_POSITIVE_EXPONENT_SIGN) != 0) {
      result += '+';
    }
  }
  if (exponent == 0) {
    result += '0';
    return;
  }
  ASSERT(exponent < 1e4);
  const int kMaxExponentLength = 5;
  char buffer[kMaxExponentLength + 1];
  buffer[kMaxExponentLength] = '\0';
  int first_char_pos = kMaxExponentLength;
  while (exponent > 0) {
    buffer[--first_char_pos] = '0' + (exponent % 10);
    exponent /= 10;
  }
  result.append(&buffer[first_char_pos],
                kMaxExponentLength - first_char_pos);
}


void DoubleToStringConverter::CreateDecimalRepresentation(
    const char* decimal_digits,
    int length,
    int decimal_point,
    int digits_after_point,
    std::string &result) const {
  // Create a representation that is padded with zeros if needed.
  if (decimal_point <= 0) {
      // "0.00000decimal_rep".
    result += '0';
    if (digits_after_point > 0) {
      result += '.';
      result.append(-decimal_point, '0');
      ASSERT(length <= digits_after_point - (-decimal_point));
      result.append(decimal_digits, length);
      int remaining_digits = digits_after_point - (-decimal_point) - length;
      result.append(remaining_digits, '0');
    }
  } else if (decimal_point >= length) {
    // "decimal_rep0000.00000" or "decimal_rep.0000"
    result.append(decimal_digits, length);
    result.append(decimal_point - length, '0');
    if (digits_after_point > 0) {
      result += '.';
      result.append(digits_after_point, '0');
    }
  } else {
    // "decima.l_rep000"
    ASSERT(digits_after_point > 0);
    result.append(decimal_digits, decimal_point);
    result += '.';
    ASSERT(length - decimal_point <= digits_after_point);
    result.append(&decimal_digits[decimal_point], length - decimal_point);
    int remaining_digits = digits_after_point - (length - decimal_point);
    result.append(remaining_digits, '0');
  }
  if (digits_after_point == 0) {
    if ((flags_ & EMIT_TRAILING_DECIMAL_POINT) != 0) {
      result += '.';
    }
    if ((flags_ & EMIT_TRAILING_ZERO_AFTER_POINT) != 0) {
      result += '0';
    }
  }
}


bool DoubleToStringConverter::ToShortestIeeeNumber(
    double value,
    std::string &result,
    DoubleToStringConverter::DtoaMode mode) const {
  ASSERT(mode == SHORTEST);
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result);
  }

  int decimal_point;
  bool sign;
  const int kDecimalRepCapacity = kBase10MaximalLength + 1;
  char decimal_rep[kDecimalRepCapacity];
  int decimal_rep_length;

  DoubleToAscii(value, mode, 0, decimal_rep, kDecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);

  bool unique_zero = (flags_ & UNIQUE_ZERO) != 0;
  if (sign && (value != 0.0 || !unique_zero)) {
    result += '-';
  }

  int exponent = decimal_point - 1;
  if ((decimal_in_shortest_low_ <= exponent) &&
      (exponent < decimal_in_shortest_high_)) {
    CreateDecimalRepresentation(decimal_rep, decimal_rep_length, decimal_point,
                                std::max(0, decimal_rep_length - decimal_point),
                                result);
  } else {
    CreateExponentialRepresentation(decimal_rep, decimal_rep_length, exponent, result);
  }
  return true;
}


bool DoubleToStringConverter::ToFixed(double value,
                                      int requested_digits,
                                      std::string &result) const
{
  ASSERT(kMaxFixedDigitsBeforePoint == 60);
  const double kFirstNonFixed = 1e60;

  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result);
  }

  if (requested_digits > kMaxFixedDigitsAfterPoint) return false;
  if (value >= kFirstNonFixed || value <= -kFirstNonFixed) return false;

  // Find a sufficiently precise decimal representation of n.
  int decimal_point;
  bool sign;
  // Add space for the '\0' byte.
  const int kDecimalRepCapacity =
      kMaxFixedDigitsBeforePoint + kMaxFixedDigitsAfterPoint + 1;
  char decimal_rep[kDecimalRepCapacity];
  int decimal_rep_length;
  DoubleToAscii(value, FIXED, requested_digits,
                decimal_rep, kDecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);

  bool unique_zero = ((flags_ & UNIQUE_ZERO) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result += '-';
  }

  CreateDecimalRepresentation(decimal_rep, decimal_rep_length, decimal_point,
                              requested_digits, result);
  return true;
}


bool DoubleToStringConverter::ToExponential(
    double value,
    int requested_digits,
    std::string &result) const {
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result);
  }

  if (requested_digits < -1) return false;
  if (requested_digits > kMaxExponentialDigits) return false;

  int decimal_point;
  bool sign;
  // Add space for digit before the decimal point and the '\0' character.
  const int kDecimalRepCapacity = kMaxExponentialDigits + 2;
  ASSERT(kDecimalRepCapacity > kBase10MaximalLength);
  char decimal_rep[kDecimalRepCapacity];
  int decimal_rep_length;

  if (requested_digits == -1) {
    DoubleToAscii(value, SHORTEST, 0,
                  decimal_rep, kDecimalRepCapacity,
                  &sign, &decimal_rep_length, &decimal_point);
  } else {
    DoubleToAscii(value, PRECISION, requested_digits + 1,
                  decimal_rep, kDecimalRepCapacity,
                  &sign, &decimal_rep_length, &decimal_point);
    ASSERT(decimal_rep_length <= requested_digits + 1);

    for (int i = decimal_rep_length; i < requested_digits + 1; ++i) {
      decimal_rep[i] = '0';
    }
    decimal_rep_length = requested_digits + 1;
  }

  bool unique_zero = ((flags_ & UNIQUE_ZERO) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result += '-';
  }

  int exponent = decimal_point - 1;
  CreateExponentialRepresentation(decimal_rep,
                                  decimal_rep_length,
                                  exponent, result);
  return true;
}


bool DoubleToStringConverter::ToPrecision(double value,
                                          int precision,
                                          std::string &result) const {
  if (Double(value).IsSpecial()) {
    return HandleSpecialValues(value, result);
  }

  if (precision < kMinPrecisionDigits || precision > kMaxPrecisionDigits) {
    return false;
  }

  // Find a sufficiently precise decimal representation of n.
  int decimal_point;
  bool sign;
  // Add one for the terminating null character.
  const int kDecimalRepCapacity = kMaxPrecisionDigits + 1;
  char decimal_rep[kDecimalRepCapacity];
  int decimal_rep_length;

  DoubleToAscii(value, PRECISION, precision,
                decimal_rep, kDecimalRepCapacity,
                &sign, &decimal_rep_length, &decimal_point);
  ASSERT(decimal_rep_length <= precision);

  bool unique_zero = ((flags_ & UNIQUE_ZERO) != 0);
  if (sign && (value != 0.0 || !unique_zero)) {
    result += '-';
  }

  // The exponent if we print the number as x.xxeyyy. That is with the
  // decimal point after the first digit.
  int exponent = decimal_point - 1;

  int extra_zero = ((flags_ & EMIT_TRAILING_ZERO_AFTER_POINT) != 0) ? 1 : 0;
  if ((-decimal_point + 1 > max_leading_padding_zeroes_in_precision_mode_) ||
      (decimal_point - precision + extra_zero >
       max_trailing_padding_zeroes_in_precision_mode_)) {
    for (int i = decimal_rep_length; i < precision; ++i) {
      decimal_rep[i] = '0';
    }

    CreateExponentialRepresentation(decimal_rep,
                                    precision,
                                    exponent,
                                    result);
  } else {
    CreateDecimalRepresentation(decimal_rep, decimal_rep_length, decimal_point,
                                std::max(0, precision - decimal_point),
                                result);
  }
  return true;
}


static BignumDtoaMode DtoaToBignumDtoaMode(
    DoubleToStringConverter::DtoaMode dtoa_mode) {
  switch (dtoa_mode) {
    case DoubleToStringConverter::SHORTEST:  return BIGNUM_DTOA_SHORTEST;
    case DoubleToStringConverter::FIXED:     return BIGNUM_DTOA_FIXED;
    case DoubleToStringConverter::PRECISION: return BIGNUM_DTOA_PRECISION;
    default:
      UNREACHABLE();
  }
}


void DoubleToStringConverter::DoubleToAscii(double v, DtoaMode mode, int requested_digits,
                                            char* buffer, int buffer_length,
                                            bool* sign, int* length, int* point)
{
  Vector<char> vector(buffer, buffer_length);
  ASSERT(!Double(v).IsSpecial());
  ASSERT(mode == SHORTEST || requested_digits >= 0);

  if (Double(v).Sign() < 0) {
    *sign = true;
    v = -v;
  } else {
    *sign = false;
  }

  if (mode == PRECISION && requested_digits == 0) {
    vector[0] = '\0';
    *length = 0;
    return;
  }

  if (v == 0) {
    vector[0] = '0';
    vector[1] = '\0';
    *length = 1;
    *point = 1;
    return;
  }

  bool fast_worked;
  switch (mode) {
    case SHORTEST:
      fast_worked = FastDtoa(v, FAST_DTOA_SHORTEST, 0, vector, length, point);
      break;
    case FIXED:
      fast_worked = FastFixedDtoa(v, requested_digits, vector, length, point);
      break;
    case PRECISION:
      fast_worked = FastDtoa(v, FAST_DTOA_PRECISION, requested_digits,
                             vector, length, point);
      break;
    default:
      fast_worked = false;
      UNREACHABLE();
  }
  if (fast_worked) return;

  // If the fast dtoa didn't succeed use the slower bignum version.
  BignumDtoaMode bignum_mode = DtoaToBignumDtoaMode(mode);
  BignumDtoa(v, bignum_mode, requested_digits, vector, length, point);
  vector[*length] = '\0';
}

template <class Iterator>
static bool ConsumeSubString(Iterator* current,
                             Iterator end,
                             const char* substring) {
  ASSERT(**current == *substring);
  for (substring++; *substring != '\0'; substring++) {
    ++*current;
    if (*current == end || **current != *substring) return false;
  }
  ++*current;
  return true;
}

const int kMaxSignificantDigits = 772;

static const char kWhitespaceTable7[] = { 32, 13, 10, 9, 11, 12 };
static const int kWhitespaceTable7Length = ARRAY_SIZE(kWhitespaceTable7);

static bool isWhitespace(int x) {
  if (x < 128) {
    for (int i = 0; i < kWhitespaceTable7Length; i++) {
      if (kWhitespaceTable7[i] == x) return true;
    }
  }
  return false;
}

// Returns true if a nonspace found and false if the end has reached.
template <class Iterator>
static inline bool AdvanceToNonspace(Iterator* current, Iterator end) {
  while (*current != end) {
    if (!isWhitespace(**current)) return true;
    ++*current;
  }
  return false;
}

static bool isDigit(int x, int radix) {
  return (x >= '0' && x <= '9' && x < '0' + radix)
      || (radix > 10 && x >= 'a' && x < 'a' + radix - 10)
      || (radix > 10 && x >= 'A' && x < 'A' + radix - 10);
}

static double SignedZero(bool sign) {
  return sign ? -0.0 : 0.0;
}

static bool IsDecimalDigitForRadix(int c, int radix) {
  return '0' <= c && c <= '9' && (c - '0') < radix;
}

static bool IsCharacterDigitForRadix(int c, int radix, char a_character) {
  return radix > 10 && c >= a_character && c < a_character + radix - 10;
}

template <int radix_log_2, class Iterator>
static double RadixStringToIeee(Iterator* current, Iterator end,
                                bool sign, bool allow_trailing_junk, double junk_string_value,
                                bool* result_is_junk)
{
  ASSERT(*current != end);

  const int kSignificandSize = Double::kSignificandSize;

  *result_is_junk = true;

  // Skip leading 0s.
  while (**current == '0') {
    ++(*current);
    if (*current == end) {
      *result_is_junk = false;
      return SignedZero(sign);
    }
  }

  int64_t number = 0;
  int exponent = 0;
  const int radix = (1 << radix_log_2);

  do {
    int digit;
    if (IsDecimalDigitForRadix(**current, radix)) {
      digit = static_cast<char>(**current) - '0';
    } else if (IsCharacterDigitForRadix(**current, radix, 'a')) {
      digit = static_cast<char>(**current) - 'a' + 10;
    } else if (IsCharacterDigitForRadix(**current, radix, 'A')) {
      digit = static_cast<char>(**current) - 'A' + 10;
    } else {
      if (allow_trailing_junk || !AdvanceToNonspace(current, end)) {
        break;
      } else {
        return junk_string_value;
      }
    }

    number = number * radix + digit;
    int overflow = static_cast<int>(number >> kSignificandSize);
    if (overflow != 0) {
      // Overflow occurred. Need to determine which direction to round the
      // result.
      int overflow_bits_count = 1;
      while (overflow > 1) {
        overflow_bits_count++;
        overflow >>= 1;
      }

      int dropped_bits_mask = ((1 << overflow_bits_count) - 1);
      int dropped_bits = static_cast<int>(number) & dropped_bits_mask;
      number >>= overflow_bits_count;
      exponent = overflow_bits_count;

      bool zero_tail = true;
      for (;;) {
        ++(*current);
        if (*current == end || !isDigit(**current, radix)) break;
        zero_tail = zero_tail && **current == '0';
        exponent += radix_log_2;
      }

      if (!allow_trailing_junk && AdvanceToNonspace(current, end)) {
        return junk_string_value;
      }

      int middle_value = (1 << (overflow_bits_count - 1));
      if (dropped_bits > middle_value) {
        number++;  // Rounding up.
      } else if (dropped_bits == middle_value) {
        // Rounding to even to consistency with decimals: half-way case rounds
        // up if significant part is odd and down otherwise.
        if ((number & 1) != 0 || !zero_tail) {
          number++;  // Rounding up.
        }
      }

      // Rounding up may cause overflow.
      if ((number & ((int64_t)1 << kSignificandSize)) != 0) {
        exponent++;
        number >>= 1;
      }
      break;
    }
    ++(*current);
  } while (*current != end);

  ASSERT(number < ((int64_t)1 << kSignificandSize));
  ASSERT(static_cast<int64_t>(static_cast<double>(number)) == number);

  *result_is_junk = false;

  if (exponent == 0) {
    if (sign) {
      if (number == 0) return -0.0;
      number = -number;
    }
    return static_cast<double>(number);
  }

  ASSERT(number != 0);
  return Double(DiyFp(number, exponent)).value();
}

double StringToDoubleConverter::StringToIeee(
    const char *input,
    int length,
    int* processed_characters_count) const {
  const char *current = input;
  const char *end = input + length;

  *processed_characters_count = 0;

  const bool allow_trailing_junk = (flags_ & ALLOW_TRAILING_JUNK) != 0;
  const bool allow_leading_spaces = (flags_ & ALLOW_LEADING_SPACES) != 0;
  const bool allow_trailing_spaces = (flags_ & ALLOW_TRAILING_SPACES) != 0;
  const bool allow_spaces_after_sign = (flags_ & ALLOW_SPACES_AFTER_SIGN) != 0;

  if (current == end) return empty_string_value_;

  if (allow_leading_spaces || allow_trailing_spaces) {
    if (!AdvanceToNonspace(&current, end)) {
      *processed_characters_count = static_cast<int>(current - input);
      return empty_string_value_;
    }
    if (!allow_leading_spaces && (input != current)) {
      return junk_string_value_;
    }
  }

  const int kBufferSize = kMaxSignificantDigits + 10;
  char buffer[kBufferSize];  // NOLINT: size is known at compile time.
  int buffer_pos = 0;

  int exponent = 0;
  int significant_digits = 0;
  int insignificant_digits = 0;
  bool nonzero_digit_dropped = false;

  bool sign = false;

  if (*current == '+' || *current == '-') {
    sign = (*current == '-');
    ++current;
    const char *next_non_space = current;

    if (!AdvanceToNonspace(&next_non_space, end)) return junk_string_value_;
    if (!allow_spaces_after_sign && (current != next_non_space)) {
      return junk_string_value_;
    }
    current = next_non_space;
  }

  if (infinity_symbol_ != NULL) {
    if (*current == infinity_symbol_[0]) {
      if (!ConsumeSubString(&current, end, infinity_symbol_)) {
        return junk_string_value_;
      }

      if (!(allow_trailing_spaces || allow_trailing_junk) && (current != end)) {
        return junk_string_value_;
      }
      if (!allow_trailing_junk && AdvanceToNonspace(&current, end)) {
        return junk_string_value_;
      }

      ASSERT(buffer_pos == 0);
      *processed_characters_count = static_cast<int>(current - input);
      return sign ? -Double::Infinity() : Double::Infinity();
    }
  }

  if (nan_symbol_ != NULL) {
    if (*current == nan_symbol_[0]) {
      if (!ConsumeSubString(&current, end, nan_symbol_)) {
        return junk_string_value_;
      }

      if (!(allow_trailing_spaces || allow_trailing_junk) && (current != end)) {
        return junk_string_value_;
      }
      if (!allow_trailing_junk && AdvanceToNonspace(&current, end)) {
        return junk_string_value_;
      }

      ASSERT(buffer_pos == 0);
      *processed_characters_count = static_cast<int>(current - input);
      return sign ? -Double::NaN() : Double::NaN();
    }
  }

  bool leading_zero = false;
  if (*current == '0') {
    ++current;
    if (current == end) {
      *processed_characters_count = static_cast<int>(current - input);
      return SignedZero(sign);
    }

    leading_zero = true;

    // It could be hexadecimal value.
    if ((flags_ & ALLOW_HEX) && (*current == 'x' || *current == 'X')) {
      ++current;
      if (current == end || !isDigit(*current, 16)) {
        return junk_string_value_;  // "0x".
      }

      bool result_is_junk;
      double result = RadixStringToIeee<4>(&current,
                                           end,
                                           sign,
                                           allow_trailing_junk,
                                           junk_string_value_,
                                           &result_is_junk);
      if (!result_is_junk) {
        if (allow_trailing_spaces) AdvanceToNonspace(&current, end);
        *processed_characters_count = static_cast<int>(current - input);
      }
      return result;
    }

    // Ignore leading zeros in the integer part.
    while (*current == '0') {
      ++current;
      if (current == end) {
        *processed_characters_count = static_cast<int>(current - input);
        return SignedZero(sign);
      }
    }
  }

  bool octal = leading_zero && (flags_ & ALLOW_OCTALS) != 0;

  // Copy significant digits of the integer part (if any) to the buffer.
  while (*current >= '0' && *current <= '9') {
    if (significant_digits < kMaxSignificantDigits) {
      ASSERT(buffer_pos < kBufferSize);
      buffer[buffer_pos++] = static_cast<char>(*current);
      significant_digits++;
      // Will later check if it's an octal in the buffer.
    } else {
      insignificant_digits++;  // Move the digit into the exponential part.
      nonzero_digit_dropped = nonzero_digit_dropped || *current != '0';
    }
    octal = octal && *current < '8';
    ++current;
    if (current == end) goto parsing_done;
  }

  if (significant_digits == 0) {
    octal = false;
  }

  if (*current == '.') {
    if (octal && !allow_trailing_junk) return junk_string_value_;
    if (octal) goto parsing_done;

    ++current;
    if (current == end) {
      if (significant_digits == 0 && !leading_zero) {
        return junk_string_value_;
      } else {
        goto parsing_done;
      }
    }

    if (significant_digits == 0) {
      // octal = false;
      // Integer part consists of 0 or is absent. Significant digits start after
      // leading zeros (if any).
      while (*current == '0') {
        ++current;
        if (current == end) {
          *processed_characters_count = static_cast<int>(current - input);
          return SignedZero(sign);
        }
        exponent--;  // Move this 0 into the exponent.
      }
    }

    // There is a fractional part.
    // We don't emit a '.', but adjust the exponent instead.
    while (*current >= '0' && *current <= '9') {
      if (significant_digits < kMaxSignificantDigits) {
        ASSERT(buffer_pos < kBufferSize);
        buffer[buffer_pos++] = static_cast<char>(*current);
        significant_digits++;
        exponent--;
      } else {
        // Ignore insignificant digits in the fractional part.
        nonzero_digit_dropped = nonzero_digit_dropped || *current != '0';
      }
      ++current;
      if (current == end) goto parsing_done;
    }
  }

  if (!leading_zero && exponent == 0 && significant_digits == 0) {
    // If leading_zeros is true then the string contains zeros.
    // If exponent < 0 then string was [+-]\.0*...
    // If significant_digits != 0 the string is not equal to 0.
    // Otherwise there are no digits in the string.
    return junk_string_value_;
  }

  // Parse exponential part.
  if (*current == 'e' || *current == 'E') {
    if (octal && !allow_trailing_junk) return junk_string_value_;
    if (octal) goto parsing_done;
    ++current;
    if (current == end) {
      if (allow_trailing_junk) {
        goto parsing_done;
      } else {
        return junk_string_value_;
      }
    }
    char sign = '+';
    if (*current == '+' || *current == '-') {
      sign = static_cast<char>(*current);
      ++current;
      if (current == end) {
        if (allow_trailing_junk) {
          goto parsing_done;
        } else {
          return junk_string_value_;
        }
      }
    }

    if (current == end || *current < '0' || *current > '9') {
      if (allow_trailing_junk) {
        goto parsing_done;
      } else {
        return junk_string_value_;
      }
    }

    const int max_exponent = INT_MAX / 2;
    ASSERT(-max_exponent / 2 <= exponent && exponent <= max_exponent / 2);
    int num = 0;
    do {
      // Check overflow.
      int digit = *current - '0';
      if (num >= max_exponent / 10
          && !(num == max_exponent / 10 && digit <= max_exponent % 10)) {
        num = max_exponent;
      } else {
        num = num * 10 + digit;
      }
      ++current;
    } while (current != end && *current >= '0' && *current <= '9');

    exponent += (sign == '-' ? -num : num);
  }

  if (!(allow_trailing_spaces || allow_trailing_junk) && (current != end)) {
    return junk_string_value_;
  }
  if (!allow_trailing_junk && AdvanceToNonspace(&current, end)) {
    return junk_string_value_;
  }
  if (allow_trailing_spaces) {
    AdvanceToNonspace(&current, end);
  }

  parsing_done:
  exponent += insignificant_digits;

  if (octal) {
    double result;
    bool result_is_junk;
    char* start = buffer;
    result = RadixStringToIeee<3>(&start,
                                  buffer + buffer_pos,
                                  sign,
                                  allow_trailing_junk,
                                  junk_string_value_,
                                  &result_is_junk);
    ASSERT(!result_is_junk);
    *processed_characters_count = static_cast<int>(current - input);
    return result;
  }

  if (nonzero_digit_dropped) {
    buffer[buffer_pos++] = '1';
    exponent--;
  }

  ASSERT(buffer_pos < kBufferSize);
  buffer[buffer_pos] = '\0';

  double converted = Strtod(Vector<const char>(buffer, buffer_pos), exponent);
  *processed_characters_count = static_cast<int>(current - input);
  return sign? -converted: converted;
}


double StringToDoubleConverter::StringToDouble(
    const char* buffer,
    int length,
    int* processed_characters_count) const {
  return StringToIeee(buffer, length, processed_characters_count);
}

} // end anonymous namespace

std::string format_coord_shortest(Coord x)
{
    char buf[20];
    bool sign;
    int length, point;

    DoubleToStringConverter::DoubleToAscii(x, DoubleToStringConverter::SHORTEST,
        0, buf, 20, &sign, &length, &point);

    int exponent = point - length;

    std::string ret;
    ret.reserve(32);

    if (sign) {
        ret += '-';
    }

    if (exponent == 0) {
        // return digits without any changes
        ret += buf;
    } else if (point >= 0 && point <= length) {
        // insert decimal point
        ret.append(buf, point);
        ret += '.';
        ret.append(&buf[point], length - point);
    } else if (exponent > 0 && exponent <= 2) {
        // add trailing zeroes
        ret += buf;
        ret.append(exponent, '0');
    } else if (point >= -3 && point <= -1) {
        // add leading zeroes
        ret += '.';
        ret.append(-point, '0');
        ret += buf;
    } else {
        // exponential form
        ret += buf;
        ret += 'e';
        if (exponent < 0) {
            ret += '-';
            exponent = -exponent;
        }

        /* Convert exponent by hand.
         * Using ostringstream is ~3x slower */
        int const buflen = 6;
        int i = 0;
        char expdigits[buflen+1];
        expdigits[buflen] = 0;

        for (; exponent && i < buflen; ++i) {
            expdigits[buflen - 1 - i] = '0' + (exponent % 10);
            exponent /= 10;
        }
        ret.append(&expdigits[buflen - i]);
    }

    return ret;
}

std::string format_coord_nice(Coord x)
{
    static DoubleToStringConverter conv(
        DoubleToStringConverter::UNIQUE_ZERO,
        "inf", "NaN", 'e', -6, 21, 0, 0);
    std::string ret;
    ret.reserve(32);
    conv.ToShortest(x, ret);
    return ret;
}

Coord parse_coord(std::string const &s)
{
    static StringToDoubleConverter conv(
        StringToDoubleConverter::ALLOW_LEADING_SPACES |
        StringToDoubleConverter::ALLOW_TRAILING_SPACES |
        StringToDoubleConverter::ALLOW_SPACES_AFTER_SIGN,
        0.0, nan(""), "inf", "NaN");
    int dummy;
    return conv.StringToDouble(s.c_str(), s.length(), &dummy);
}

} // namespace Geom

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
