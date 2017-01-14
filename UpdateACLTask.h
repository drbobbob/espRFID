/*
 * UpdateACLTask.h
 *      Author: DrBobBob
 */

#ifndef UPDATEACLTASK_H_
#define UPDATEACLTASK_H_

#include "Task.h"

class DoorLatchTask;

class UpdateACLTask: public TimedTask {
public:
  UpdateACLTask(uint32_t interval, DoorLatchTask& _latchTask);
  virtual ~UpdateACLTask();

  virtual void run(uint32_t now);

  void startManualTimer();
  void startShortManualTimer();
  void stopManualTimer();

private:

  void automatedUpdate();
  void manualUpdate();

  void downloadACL();

  bool isAutomatedUpdate();

  uint32_t nextAutomatedTime;
  uint32_t automatedInterval;
  DoorLatchTask& latchTask;
  
};

#endif /* BUTTONPRINTER_H_ */
