/*
 * RFIDReaderTask.cpp
 *      Author: DrBobBob
 */

#include "RFIDReaderTask.h"
#include "Arduino.h"

#include <SoftwareSerial.h>
#include <FS.h>
#include "DoorLatchTask.h"
#include "UpdateACLTask.h"

RFIDReaderTask::RFIDReaderTask(SoftwareSerial& _rfidSerial, DoorLatchTask& _latchTask, UpdateACLTask& _aclTask)
: Task()
, rfidSerial(_rfidSerial)
, latchTask(_latchTask)
, aclTask(_aclTask)
, serialBuffer()
{
  rfidSerial.begin(9600);
}

RFIDReaderTask::~RFIDReaderTask() {
}

bool RFIDReaderTask::canRun(uint32_t now)
{
  rfidSerial.available();
}

void RFIDReaderTask::run(uint32_t now)
{
  while(rfidSerial.available())
  {
    handleChar(rfidSerial.read());
  }
}

void RFIDReaderTask::handleChar(char c)
{
  if(c == 0x02)
  {
    serialBuffer = "";
  }
  else if(c == 0x03)
  {
    validateCard();
  }
  else
  {
    serialBuffer += c;
  }
}

void RFIDReaderTask::validateCard()
{
  bool validated = aclTask.validateCard(serialBuffer);
  if(validated)
  {
    Serial.println("Validated Card");
    latchTask.openDoor();
  }
  else
  {
    Serial.println("Card did not validate");
  }
}

