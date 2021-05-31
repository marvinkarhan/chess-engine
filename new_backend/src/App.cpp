#include "./controller/Controller.hpp"
#include "./AppComponent.hpp"

#include "oatpp/network/Server.hpp"

#include <iostream>
#include "./engine/constants.h"

void run()
{

  /* Register Components in scope of run() method */
  AppComponent components;

  /* Init chess constants */
  initConstants();
  /* Get router component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);

  /* Create MyController and add all of its endpoints to router */
  auto controller = std::make_shared<Controller>();
  controller->addEndpointsToRouter(router);

  /* Get connection handler component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, connectionHandler, "http");

  /* Get connection provider component */
  OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, connectionProvider);

  /* Create server which takes provided TCP connections and passes them to HTTP connection handler */
  oatpp::network::Server server(connectionProvider, connectionHandler);

  /* Priny info about server port */
  OATPP_LOGI("MyApp", "Server running on port %s", connectionProvider->getProperty("port").getData());

  OATPP_LOGI("NoPy++", "\r\n __    __            _______                                \r\n|  \\  |  \\          |       \\               __        __    \r\n| $$\\ | $$  ______  | $$$$$$$\\ __    __    |  \\      |  \\   \r\n| $$$\\| $$ /      \\ | $$__/ $$|  \\  |  \\ __| $$__  __| $$__ \r\n| $$$$\\ $$|  $$$$$$\\| $$    $$| $$  | $$|    $$  \\|    $$  \\\r\n| $$\\$$ $$| $$  | $$| $$$$$$$ | $$  | $$ \\$$$$$$$$ \\$$$$$$$$\r\n| $$ \\$$$$| $$__/ $$| $$      | $$__/ $$   | $$      | $$   \r\n| $$  \\$$$ \\$$    $$| $$       \\$$    $$    \\$$       \\$$   \r\n \\$$   \\$$  \\$$$$$$  \\$$       _\\$$$$$$$                    \r\n                              |  \\__| $$                    \r\n                               \\$$    $$                    \r\n                                \\$$$$$$               ");

  /* Run server */
  server.run();
}

int main(int argc, const char *argv[])
{

  oatpp::base::Environment::init();

  run();

  oatpp::base::Environment::destroy();

  return 0;
}