
#ifndef Controller_hpp
#define Controller_hpp

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/network/ConnectionHandler.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class Controller : public oatpp::web::server::api::ApiController
{
private:
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");

public:
  Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper)
  {
  }

public:
  ENDPOINT("GET", "/", root)
  {

    const char *pageTemplate =
        "<html lang='en'>"
        "<head>"
        "<meta charset=utf-8/>"
        "</head>"
        "<body>"
        "<p>Hello Multithreaded WebSocket Server!</p>"
        "</body>"
        "</html>";

    return createResponse(Status::CODE_200, pageTemplate);
  };

  ENDPOINT("GET", "ws", ws, REQUEST(std::shared_ptr<IncomingRequest>, request))
  {
    return oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), websocketConnectionHandler);
  };
};

#include OATPP_CODEGEN_END(ApiController) //<-- codegen end

#endif /* MyController_hpp */
