#include <string>
#include <stdexcept>
#include <stdio.h>
#include <iostream>

#include "../incbin/incbin.h"
#include "init.h"
#include "feature-transformer.h"
#include "halfkp.h"
#include "nnue-helper.h"

// embed NNUE net into the engine binary (using incbin by Dale Weiler)
// declares:
//   const unsigned char        gBinNNUEData[];  // pointer to start
//   const unsigned char *const gBinNNUEEnd;     // pointer to end
//   const unsigned int         gBinNNUESize;    // file size
#if !defined(_WIN32) && !defined(WIN32) && !defined(_MSC_VER)
INCBIN(BinNNUE, NNUEFileName);
#else
const unsigned char gBinNNUEData[1] = {0x0};
const unsigned char *const gBinNNUEEnd = &gBinNNUEData[1];
const unsigned int gBinNNUESize = 1;
#endif

namespace NNUE
{

  AlignedPtr<FeatureTransformer> feature_transformer;
  AlignedPtr<Network> network;
  std::string fileName;

  void init()
  {
    // C++ way to prepare a buffer for a memory stream
    class MemoryBuffer : public basic_streambuf<char>
    {
    public:
      MemoryBuffer(char *data, size_t size)
      {
        setg(data, data, data + size);
      }
    };

    MemoryBuffer buffer(const_cast<char *>(reinterpret_cast<const char *>(gBinNNUEData)),
                        size_t(gBinNNUESize));

    istream stream(&buffer);
    if (!loadWeights(NNUEFileName, stream))
    {
      throw std::invalid_argument("could not load nnue file");
    }
  }

  template <typename T>
  void initPointer(AlignedPtr<T> &pointer)
  {
    // init pointer with alignment, because of simd
    pointer.reset(reinterpret_cast<T *>(stdAlignedAlloc(alignof(T), sizeof(T))));
    // pointer.reset(reinterpret_cast<T *>(std::malloc(sizeof(T))));
    std::memset(pointer.get(), 0, sizeof(T));
  }

  void initNet()
  {
    initPointer(feature_transformer);
    initPointer(network);
  }

  template <typename T>
  bool readLayer(std::istream &stream, const AlignedPtr<T> &pointer)
  {
    return pointer->readParameters(stream);
  }

  bool readParameters(std::istream &stream)
  {
    if (!readLayer(stream, feature_transformer))
      return false;
    if (!readLayer(stream, network))
      return false;
    return stream && stream.peek() == std::ios::traits_type::eof();
  }

  bool loadWeights(std::string name, std::istream &stream)
  {
    initNet();
    fileName = name;
    return readParameters(stream);
  }

  // Evaluation function. Perform differential calculation.
  int evaluate(Board &board)
  {
    alignas(kCacheLineSize) std::uint8_t transformed_features[FeatureTransformer::kBufferSize];
    feature_transformer->Transform(board, transformed_features);
    alignas(kCacheLineSize) char buffer[Network::kBufferSize];
    const auto output = network->Propagate(transformed_features, buffer);

    return output[0] / FV_SCALE / NNUE_PAWN_VALUE;
  }
} // namespace NNUE