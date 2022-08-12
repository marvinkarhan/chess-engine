#pragma once

#include <iostream>
#include <cstring>
#include <memory>

#if defined(USE_AVX2)
#include <immintrin.h>

#elif defined(USE_SSE41)
#include <smmintrin.h>

#elif defined(USE_SSSE3)
#include <tmmintrin.h>

#elif defined(USE_SSE2)
#include <emmintrin.h>

#elif defined(USE_MMX)
#include <mmintrin.h>

#elif defined(USE_NEON)
#include <arm_neon.h>
#endif

// HACK: Use _mm256_loadu_si256() instead of _mm256_load_si256. Otherwise a binary
//       compiled with older g++ crashes because the output memory is not aligned
//       even though alignas is specified.
#if defined(USE_AVX2)
#if defined(__GNUC__ ) && (__GNUC__ < 9) && defined(_WIN32) && !defined(__clang__)
#define _mm256_loadA_si256  _mm256_loadu_si256
#define _mm256_storeA_si256 _mm256_storeu_si256
#else
#define _mm256_loadA_si256  _mm256_load_si256
#define _mm256_storeA_si256 _mm256_store_si256
#endif
#endif

#if defined(USE_AVX512)
#if defined(__GNUC__ ) && (__GNUC__ < 9) && defined(_WIN32) && !defined(__clang__)
#define _mm512_loadA_si512   _mm512_loadu_si512
#define _mm512_storeA_si512  _mm512_storeu_si512
#else
#define _mm512_loadA_si512   _mm512_load_si512
#define _mm512_storeA_si512  _mm512_store_si512
#endif
#endif

// forward declaration of Board
class Board;

void *stdAlignedAlloc(size_t alignment, size_t size);
void stdAlignedFree(void* ptr);
constexpr std::size_t kMaxSimdWidth = 32;
constexpr std::size_t kCacheLineSize = 64;
constexpr int FV_SCALE = 16;
constexpr int kWeightScaleBits = 6;
constexpr int NNUE_PAWN_VALUE = 208;

// Deleter for automating release of memory area
template <typename T>
struct AlignedDeleter {
  void operator()(T* ptr) const {
    ptr->~T();
    stdAlignedFree(ptr);
  }
};

template <typename T>
using AlignedPtr = std::unique_ptr<T, AlignedDeleter<T>>;

// Round n up to be a multiple of base
template <typename IntType>
constexpr IntType CeilToMultiple(IntType n, IntType base)
{
  return (n + base - 1) / base * base;
}

template <typename IntType>
inline IntType readLittleEndian(std::istream &stream)
{

  IntType result;
  std::uint8_t u[sizeof(IntType)];
  typename std::make_unsigned<IntType>::type v = 0;

  stream.read(reinterpret_cast<char *>(u), sizeof(IntType));
  for (std::size_t i = 0; i < sizeof(IntType); ++i)
    v = (v << 8) | u[sizeof(IntType) - i - 1];

  std::memcpy(&result, &v, sizeof(IntType));
  return result;
}

// unique number for each piece type on each square
enum
{
  PS_NONE = 0,
  PS_W_PAWN = 1,
  PS_B_PAWN = 1 * 64 + 1,
  PS_W_KNIGHT = 2 * 64 + 1,
  PS_B_KNIGHT = 3 * 64 + 1,
  PS_W_BISHOP = 4 * 64 + 1,
  PS_B_BISHOP = 5 * 64 + 1,
  PS_W_ROOK = 6 * 64 + 1,
  PS_B_ROOK = 7 * 64 + 1,
  PS_W_QUEEN = 8 * 64 + 1,
  PS_B_QUEEN = 9 * 64 + 1,
  PS_W_KING = 10 * 64 + 1,
  PS_END = PS_W_KING, // pieces without kings (pawns included)
  PS_B_KING = 11 * 64 + 1,
  PS_END2 = 12 * 64 + 1
};

extern uint32_t kpp_board_index[16][2];

// SIMD width (in bytes)
#if defined(USE_AVX2)
constexpr std::size_t kSimdWidth = 32;

#elif defined(USE_SSE2)
constexpr std::size_t kSimdWidth = 16;

#elif defined(USE_MMX)
constexpr std::size_t kSimdWidth = 8;

#elif defined(USE_NEON)
constexpr std::size_t kSimdWidth = 16;
#endif
