/*
 * ServerTask.h
 *      Author: DrBobBob
 */

#ifndef SERVERTASK_H_
#define SERVERTASK_H_

#include "Task.h"

class ESP8266WebServer;

class ServerTask: public Task {
public:
  ServerTask(ESP8266WebServer& _server);
  virtual ~ServerTask();

  virtual bool canRun(uint32_t now);
  virtual void run(uint32_t now);

  private:
  ESP8266WebServer& server;
};

#endif /* BUTTONPRINTER_H_ */
