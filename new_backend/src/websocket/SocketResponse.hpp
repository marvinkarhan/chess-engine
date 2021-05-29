#ifndef SOCKET_RESPONSE_HPP
#define SOCKET_RESPONSE_HPP
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class SocketResponse : public oatpp::DTO
{

  DTO_INIT(SocketResponse, DTO /* extends */);
  DTO_FIELD(String, fen);
  DTO_FIELD(List<String>, moves);
  DTO_FIELD(Float32, evaluation);
  DTO_FIELD(List<String>, aiMoves);
};
#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section
#endif