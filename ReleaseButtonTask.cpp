/*
 * ReleaseButtonTask.cpp
 *      Author: DrBobBob
 */

#include "ReleaseButtonTask.h"
#include "Arduino.h"

#include "DoorLatchTask.h"
#include "UpdateACLTask.h"

ReleaseButtonTask::ReleaseButtonTask(int pin, DoorLatchTask& _latchTask, UpdateACLTask& _updateTask)
: ButtonDebounceTask(pin, true)
, latchTask(_latchTask)
, updateTask(_updateTask)
{
  // TODO Auto-generated constructor stub

}

ReleaseButtonTask::~ReleaseButtonTask() {
  // TODO Auto-generated destructor stub
}

void ReleaseButtonTask::ButtonPressed()
{
  latchTask.openDoor();
  updateTask.startManualTimer();
}

void ReleaseButtonTask::ButtonReleased()
{
  updateTask.stopManualTimer();
}

