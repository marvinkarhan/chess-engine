#include "WSListener.hpp"
#include <map>
#include "BoardEvents.hpp"
#include "SocketResponse.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "../engine/board.h"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include <iostream>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WSListener

std::map<long, Board*> SessionMap;
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
  delete SessionMap[pointerToSession];
  SessionMap.erase(pointerToSession);
  OATPP_LOGD(TAG, "onClose code=%d", code);
}

void WSListener::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{

  if (size == 0)
  { // message transfer finished

    auto wholeMessage = m_messageBuffer.toString();
    m_messageBuffer.clear();
    auto shared = socket.getListener();
    long long pointerToSession = (long long)shared.get();

    //OATPP_LOGD(TAG,"Connection %s", shared);

    //OATPP_LOGD(TAG, "onMessage message='%s'", wholeMessage->c_str());
    oatpp::String startPos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    int compare = BOARD_EVENTS_NAMES[BoardEvents::NEW_BOARD].compare(wholeMessage->c_str());
    if (compare == 0)
    {
      Board *board = new Board();
      SessionMap[pointerToSession] = board;
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = board->toFenString().c_str();
      socketResponse->moves = {"A1", "A2", "A3"};
      socketResponse->evaluation = 0;
      socketResponse->aiMoves = {""};
      auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      oatpp::String json = jsonObjectMapper->writeToString(socketResponse);
      socket.sendOneFrameText(json);
    }
    /* Send message in reply */
    //socket.sendOneFrameText( "Hello from oatpp!: " + wholeMessage);
    //socket.sendOneFrameText("{\"fen\": \"hier_fen\" ,\"moves\": [\"a1\",\"a2\"], \"evaluation\" : \"2\", \"aiMoves\": [\"a1\"]}");
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