#include "nnue-helper.h"

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__OpenBSD__) || (defined(__GLIBCXX__) && !defined(_GLIBCXX_HAVE_ALIGNED_ALLOC) && !defined(_WIN32))
#define POSIXALIGNEDALLOC
#include <stdlib.h>
#endif

void *stdAlignedAlloc(size_t alignment, size_t size)
{
#if defined(POSIXALIGNEDALLOC)
  void *mem;
  return posix_memalign(&mem, alignment, size) ? nullptr : mem;
#elif defined(_WIN32)
  return _mm_malloc(size, alignment);
#else
  return std::aligned_alloc(alignment, size);
#endif
}

void stdAlignedFree(void* ptr) {
#if defined(POSIXALIGNEDALLOC)
  free(ptr);
#elif defined(_WIN32)
  _mm_free(ptr);
#else
  free(ptr);
#endif
}

uint32_t kpp_board_index[16][2] = {
    // convention: W - us, B - them
    // viewed from other side, W and B are reversed
    {PS_NONE, PS_NONE},
    {PS_W_PAWN, PS_B_PAWN},
    {PS_W_KNIGHT, PS_B_KNIGHT},
    {PS_W_BISHOP, PS_B_BISHOP},
    {PS_W_ROOK, PS_B_ROOK},
    {PS_W_QUEEN, PS_B_QUEEN},
    {PS_W_KING, PS_B_KING},
    {PS_NONE, PS_NONE},
    {PS_NONE, PS_NONE},
    {PS_B_PAWN, PS_W_PAWN},
    {PS_B_KNIGHT, PS_W_KNIGHT},
    {PS_B_BISHOP, PS_W_BISHOP},
    {PS_B_ROOK, PS_W_ROOK},
    {PS_B_QUEEN, PS_W_QUEEN},
    {PS_B_KING, PS_W_KING},
    {PS_NONE, PS_NONE}};
