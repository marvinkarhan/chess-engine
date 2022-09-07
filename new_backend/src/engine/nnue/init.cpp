#include <string>
#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <fstream>

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
INCBIN(BinNNUE, NNUEFileName);
namespace NNUE
{

  AlignedPtr<FeatureTransformer> feature_transformer;
  AlignedPtr<Network> network;

  bool init()
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
    if (!loadWeights(stream))
    {
      std::cout << "Could not load nnue file!" << std::endl;
      return false;
    }
    return true;
  }

  bool loadFile(std::string fileName) {
    ifstream stream(fileName, ios::binary);
    if (!loadWeights(stream))
    {
      std::cout << "Could not load nnue file!" << std::endl;
      return false;
    } else {
      std::cout << "Now using NNUE file: " << fileName << std::endl;
      return true;
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

  bool loadWeights(std::istream &stream)
  {
    initNet();
    return readParameters(stream);
  }

  // Evaluation function. Perform differential calculation.
  int evaluate(Board &board)
  {
    alignas(kCacheLineSize) std::uint8_t transformed_features[FeatureTransformer::kBufferSize];
    feature_transformer->Transform(board, transformed_features);
    alignas(kCacheLineSize) char buffer[Network::kBufferSize];
    const auto output = network->Propagate(transformed_features, buffer);
    // std::cout << "fen: " << board.toFenString() << " output: " << output[0] << " scaled: " << output[0] / FV_SCALE / NNUE_PAWN_VALUE << " new scaling:" << output[0] / FV_SCALE << std::endl;
    return output[0] / FV_SCALE;
  }
} // namespace NNUE