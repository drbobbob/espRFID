/*
 * ReleaseButtonTask.h
 *      Author: DrBobBob
 */

#ifndef RELEASEBUTTONTASK_H_
#define RELEASEBUTTONTASK_H_

#include "ButtonDebounceTask.h"

class DoorLatchTask;
class UpdateACLTask;

class ReleaseButtonTask: public ButtonDebounceTask {
public:
  ReleaseButtonTask(int pin, DoorLatchTask& _latchTask, UpdateACLTask& _updateTask);
  virtual ~ReleaseButtonTask();

  virtual void ButtonPressed();
  virtual void ButtonReleased();

private:
  DoorLatchTask& latchTask;
  UpdateACLTask& updateTask;
};

#endif /* BUTTONPRINTER_H_ */
