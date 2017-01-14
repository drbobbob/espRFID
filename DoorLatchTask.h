/*
 * DoorLatchTask.h
 *      Author: DrBobBob
 */

#ifndef DOORLATCHTASK_H_
#define DOORLATCHTASK_H_

#include "Task.h"

class DoorLatchTask: public SingleShotTimedTask {
public:
  DoorLatchTask(int pin);
  virtual ~DoorLatchTask();

  virtual void run(uint32_t now);

  void openDoor();
  void blinkDoor();

private:

  void openDoorForTime(uint32_t openTime);
  int doorPin;
};

#endif /* BUTTONPRINTER_H_ */
