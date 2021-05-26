#pragma once
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class SocketRequest : public oatpp::DTO
{

  DTO_INIT(SocketRequest, DTO /* extends */);
  DTO_FIELD(String, emitMessage);
};

class NewBoardRequest : public oatpp::DTO
{
  DTO_INIT(NewBoardRequest, SocketRequest);
};

class MoveRequest : public oatpp::DTO
{
  DTO_INIT(MoveRequest, SocketRequest);
  DTO_FIELD(String, move);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section