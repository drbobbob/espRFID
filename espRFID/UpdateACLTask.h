/*
 * UpdateACLTask.h
 *      Author: DrBobBob
 */

#ifndef UPDATEACLTASK_H_
#define UPDATEACLTASK_H_

#include "Task.h"
#include "Arduino.h"

class DoorLatchTask;
class BlinkPatternTask;
class String;

class UpdateACLTask: public TimedTask {
public:
  UpdateACLTask(uint32_t interval, DoorLatchTask& _latchTask, BlinkPatternTask& _blinkTask);
  virtual ~UpdateACLTask();

  virtual void run(uint32_t now);

  void startManualTimer();
  void startShortManualTimer();
  void stopManualTimer();

  bool validateCard(const String& card);

  String getACL();
  String getACLLog();

private:

  void automatedUpdate();
  void manualUpdate();

  bool downloadACL();

  bool isAutomatedUpdate();

  uint32_t retryCount;
  uint32_t nextRetryInterval;
  uint32_t nextAutomatedTime;
  uint32_t automatedInterval;
  String lastUpdateLog;
  DoorLatchTask& latchTask;
  BlinkPatternTask& blinkTask;

};

#endif /* BUTTONPRINTER_H_ */
