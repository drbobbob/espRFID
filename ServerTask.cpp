/*
 * ServerTask.cpp
 *      Author: DrBobBob
 */

#include "ServerTask.h"
#include "Arduino.h"
#include <ESP8266WebServer.h>

ServerTask::ServerTask(ESP8266WebServer& _server)
: Task()
, server(_server)
{


}

ServerTask::~ServerTask() {
}

bool ServerTask::canRun(uint32_t now)
{
  return true;
}

void ServerTask::run(uint32_t now)
{
  server.handleClient();
  yield();
}

