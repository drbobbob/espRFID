/*
 * DoorLatchTask.cpp
 *      Author: DrBobBob
 */

#include "DoorLatchTask.h"
#include "Arduino.h"

uint32_t DOOR_OPEN_TIME = 10000;
uint32_t DOOR_BLINK_TIME = 100;

DoorLatchTask::DoorLatchTask(int pin)
: SingleShotTimedTask(0)
, doorPin(pin)
{
  pinMode(doorPin, OUTPUT);
  digitalWrite(doorPin, LOW);
}

DoorLatchTask::~DoorLatchTask() {
}

void DoorLatchTask::run(uint32_t now)
{
  digitalWrite(doorPin, LOW);
}

void DoorLatchTask::openDoor()
{
  openDoorForTime(DOOR_OPEN_TIME);
}

void DoorLatchTask::blinkDoor()
{
  openDoorForTime(DOOR_BLINK_TIME);
}

void DoorLatchTask::openDoorForTime(uint32_t openTime)
{
  digitalWrite(doorPin, HIGH);
  setRunTime(millis() + openTime);
  setRunnable();
}

