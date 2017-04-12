/*
 * RFIDReaderTask.h
 *      Author: DrBobBob
 */

#ifndef RFIDREADERTASK_H_
#define RFIDREADERTASK_H_

#include "Task.h"
#include <Arduino.h>

class SoftwareSerial;
class DoorLatchTask;
class UpdateACLTask;

class RFIDReaderTask: public Task {
public:
  RFIDReaderTask(SoftwareSerial& _rfidSerial, DoorLatchTask& _latchTask, UpdateACLTask& _aclTask);
  virtual ~RFIDReaderTask();

  virtual bool canRun(uint32_t now);
  virtual void run(uint32_t now);

private:
  void handleChar(char c);

  void validateCard();

  SoftwareSerial & rfidSerial;
  DoorLatchTask& latchTask;
  UpdateACLTask& aclTask;
  String serialBuffer;
};

#endif /* BUTTONPRINTER_H_ */
