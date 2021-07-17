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
#include <chrono>
#include <vector>
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

oatpp::String makeEngineMove(Board *userBoard) {
  int eval = 0;
  bool gameOver = userBoard->checkmate() || userBoard->stalemate();
  if (!gameOver)
  {
    auto start = std::chrono::high_resolution_clock::now();
    eval = userBoard->evaluateNextMove();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "\r\n--- total runtime: " << elapsed.count() << " seconds ---" << std::endl;
    cout << "variation: " << toUciString(userBoard->getPV()[0]) << endl;
    userBoard->makeMove(userBoard->getPV()[0]);
    cout << "Made move, eval: " << eval << endl;
  }
  auto socketResponse = SocketResponse::createShared();
  socketResponse->fen = userBoard->toFenString().c_str();
  socketResponse->moves = {};
  for (Move move : MoveList<LEGAL_MOVES>(*userBoard, userBoard->activeSide))
  {
    const string value = toUciString(move);
    socketResponse->moves->push_front(value.c_str());
  }
  socketResponse->evaluation = (float)eval / 100;
  std::cout << "response eval: " << socketResponse->evaluation << std::endl;
  socketResponse->aiMoves = {};
  if (!gameOver)
    for (Move move : userBoard->getPV())
      socketResponse->aiMoves->push_back(toUciString(move).c_str());

  auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
  oatpp::String json = jsonObjectMapper->writeToString(socketResponse);
  return json;
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
      oatpp::Object<NewBoardRequest> request = jsonObjectMapper->readFromString<oatpp::Object<NewBoardRequest>>(wholeMessage);
      cout << "Requested new board, fen: " << request->fen->c_str() << std::endl;
      // Board *board = new Board("r3r1k1/p4pbp/1pp3p1/1q4N1/8/1BP4Q/PP2nPPP/R4K1R b - - 0 1");
      // Board *board = new Board("k7/8/8/8/8/8/1R6/2R4K w - - 0 1");
      Board *board = new Board(request->fen->c_str());
      SessionMap[pointerToSession] = board;
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = board->toFenString().c_str();
      socketResponse->moves = {};
      for (Move move : MoveList<LEGAL_MOVES>(*board, board->activeSide))
      {
        const string value = toUciString(move);
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

      Board *userBoard = SessionMap[pointerToSession];
      userBoard->makeMove(uciToMove(request->move->c_str(), *userBoard));
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = userBoard->toFenString().c_str();
      socketResponse->moves = {};
      socketResponse->evaluation = 0.0;
      socketResponse->aiMoves = {""};
      auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      oatpp::String json = jsonObjectMapper->writeToString(socketResponse);
      socket.sendOneFrameText(json);
    }
    else if (strcmp(emitMessage, BOARD_EVENTS_NAMES[BoardEvents::UNMAKE_MOVE]) == 0)
    {
      cout << "Requested unmake Move " << endl;
      Board *userBoard = SessionMap[pointerToSession];
      if (!userBoard->state->move)
      {
        cout << "no move to unmake: " << userBoard->toFenString() << endl;
        return;
      }
      userBoard->unmakeMove(userBoard->state->move);
      cout << "unmade move" << endl;
      auto socketResponse = SocketResponse::createShared();
      socketResponse->fen = userBoard->toFenString().c_str();
      socketResponse->moves = {};
      for (Move move : MoveList<LEGAL_MOVES>(*userBoard, userBoard->activeSide))
      {
        const string value = toUciString(move);
        socketResponse->moves->push_front(value.c_str());
      }
      socketResponse->evaluation = 0.0;
      socketResponse->aiMoves = {""};
      auto jsonObjectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
      oatpp::String json = jsonObjectMapper->writeToString(socketResponse);
      socket.sendOneFrameText(json);
    }
    else if (strcmp(emitMessage, BOARD_EVENTS_NAMES[BoardEvents::NEW_ENGINE_MOVE]) == 0)
    {
      cout << "Requested swap Board " << endl;
      Board *userBoard = SessionMap[pointerToSession];

      socket.sendOneFrameText(makeEngineMove(userBoard));
    }
    else if (strcmp(emitMessage, BOARD_EVENTS_NAMES[BoardEvents::CHANGE_TIME]) == 0)
    {
      oatpp::Object<ChangeTimeRequest> request = jsonObjectMapper->readFromString<oatpp::Object<ChangeTimeRequest>>(wholeMessage);
      cout << "Requested change time: " << request->time << endl;
      Board *userBoard = SessionMap[pointerToSession];

      userBoard->thinkingTime = request->time;
      // no need to send a response
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