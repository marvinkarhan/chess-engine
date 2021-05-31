#include "WSListener.hpp"
#include <map>
#include "BoardEvents.hpp"
#include "SocketResponse.hpp"
#include "SocketRequest.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "../engine/board.h"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include <iostream>
#include "../engine/board.h"
#include "../engine/constants.h"
#include "../engine/moveHelper.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSListener

std::map<long, Board> SessionMap;
void WSListener::onPing(const WebSocket &socket, const oatpp::String &message)
{
  OATPP_LOGD(TAG, "onPing");
  socket.sendPong(message);
}

void WSListener::onPong(const WebSocket &socket, const oatpp::String &message)
{
  OATPP_LOGD(TAG, "onPong");
}

void WSListener::onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message)
{
  auto shared = socket.getListener();
  long long pointerToSession = (long long)shared.get();
  SessionMap.erase(pointerToSession);
  OATPP_LOGD(TAG, "onClose code=%d", code);
}

void WSListener::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{

  if (size == 0)
  { // message transfer finished

    const oatpp::String wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.clear();
    auto shared = socket.getListener();
    long long pointerToSession = (long long)shared.get();

    const char *jsonData = wholeMessage->c_str();
    auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    oatpp::Object<SocketRequest> request = jsonObjectMapper->readFromString<oatpp::Object<SocketRequest>>(wholeMessage);
    const char *emitMessage = request->emitMessage->c_str();
    cout << "Request: " << request->emitMessage->c_str() << endl;
    if (strcmp(emitMessage, BOARD_EVENTS_NAMES[BoardEvents::NEW_BOARD]) == 0)
    {
      cout << "Requested new board!" << endl;
      Board board;
      SessionMap[pointerToSession] = board;
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = board.toFenString().c_str();
      socketResponse->moves = {};
      for (Move move : MoveList<LEGAL_MOVES>(board, board.activeSide))
      {
        const string value = move.to_uci_string();
        socketResponse->moves->push_front(value.c_str());
      }
      socketResponse->evaluation = 0.0;
      socketResponse->aiMoves = {""};
      auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      oatpp::String json = jsonObjectMapper->writeToString(socketResponse);
      socket.sendOneFrameText(json);
    }
    else if (strcmp(emitMessage, BOARD_EVENTS_NAMES[BoardEvents::MAKE_MOVE]) == 0)
    {
      cout << "Requested new Move " << endl;
      //Map to NewBoardRequest
      oatpp::Object<MoveRequest> request = jsonObjectMapper->readFromString<oatpp::Object<MoveRequest>>(wholeMessage);
      cout << "Requested move is: " << request->move->c_str() << endl;

      Board *userBoard = &SessionMap[pointerToSession];
      userBoard->makeMove(uciToMove(request->move->c_str()));
      int depth = 6;
      PVariation pVariation;
      cout << "depth: " << depth << endl;
      int eval = userBoard->negaMax(depth, -2000000, 2000000, &pVariation);
      cout << "variation: " << pVariation.moves[0].to_uci_string() << endl;
      userBoard->makeMove(pVariation.moves[0]);
      cout << "Made move" << endl;
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = userBoard->toFenString().c_str();
      socketResponse->moves = {};
      for (Move move : MoveList<LEGAL_MOVES>(*userBoard, userBoard->activeSide))
      {
        const string value = move.to_uci_string();
        socketResponse->moves->push_front(value.c_str());
      }
      socketResponse->evaluation = eval;
      socketResponse->aiMoves = {};
      for (int i = 0; i < pVariation.len; i++) {
        socketResponse->aiMoves->push_back(pVariation.moves[i].to_uci_string().c_str());
      }
      auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      oatpp::String json = jsonObjectMapper->writeToString(socketResponse);

      socket.sendOneFrameText(json);
    }
  }
  else if (size > 0)
  { // message frame received
    m_messageBuffer.writeSimple(data, size);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSInstanceListener

std::atomic<v_int32> WSInstanceListener::SOCKETS(0);

void WSInstanceListener::onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params)
{

  SOCKETS++;
  OATPP_LOGD(TAG, "New Incoming Connection. Connection count=%d", SOCKETS.load());
  /* In this particular case we create one WSListener per each connection */
  /* Which may be redundant in many cases */
  socket.setListener(std::make_shared<WSListener>());
}

void WSInstanceListener::onBeforeDestroy(const oatpp::websocket::WebSocket &socket)
{

  SOCKETS--;
  OATPP_LOGD(TAG, "Connection closed. Connection count=%d", SOCKETS.load());
}