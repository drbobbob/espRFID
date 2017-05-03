/*
 * ServerTask.h
 *      Author: DrBobBob
 */

#ifndef SERVERTASK_H_
#define SERVERTASK_H_

#include "Task.h"
#include <ESP8266WebServer.h>

class UpdateACLTask;

class ServerTask: public TimedTask {
public:
  enum ServerState {
    SERVER_START,
    SERVER_WAIT_FOR_CONNECTION,
    SERVER_RUNNING
  };
  ServerTask(UpdateACLTask& _aclTask);
  virtual ~ServerTask();

  virtual void run(uint32_t now);

  private:
  void StartState(uint32_t now);
  void WaitForConnectionState(uint32_t now);
  void RunningState(uint32_t now);

  void handleRoot();
  void handleACL();
  void handleACLLog();
  void handleUpdateACL();
  void handleNotFound();

  UpdateACLTask& aclTask;
  ESP8266WebServer server;
  ServerState CurrentState;
};

#endif /* BUTTONPRINTER_H_ */
