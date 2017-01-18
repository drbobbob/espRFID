/*
 * UpdateACLTask.cpp
 *      Author: DrBobBob
 */

#include "UpdateACLTask.h"
#include "Arduino.h"

#include "DoorLatchTask.h"
#include "HTTPSRedirect.h"
#include "SecretConstants.h"

#include <FS.h>

const int BUTTON_HOLD_TIME = 14000;

//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif


const int httpsPort = 443;

const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";

String url = String("/macros/s/") + GScriptId + "/exec?";

const char* aclFilename = "acl";


UpdateACLTask::UpdateACLTask(uint32_t interval, DoorLatchTask& _latchTask)
: TimedTask(0)
, nextAutomatedTime(0)
, automatedInterval(interval)
, latchTask(_latchTask)
{
}

UpdateACLTask::~UpdateACLTask() {
}

void UpdateACLTask::run(uint32_t now)
{
  if(isAutomatedUpdate())
  {
    automatedUpdate();
  }
  else
  {
    manualUpdate();
  }
}

void UpdateACLTask::startManualTimer()
{
  setRunTime(millis() + BUTTON_HOLD_TIME);
}

void UpdateACLTask::startShortManualTimer()
{
  setRunTime(millis());
}

void UpdateACLTask::stopManualTimer()
{
  setRunTime(nextAutomatedTime);
}

bool UpdateACLTask::isAutomatedUpdate()
{
  return runTime == nextAutomatedTime;
}

void UpdateACLTask::automatedUpdate()
{
  incRunTime(automatedInterval);
  nextAutomatedTime = runTime;
  downloadACL();
}

void UpdateACLTask::manualUpdate()
{
  setRunTime(nextAutomatedTime);
  downloadACL();
  latchTask.blinkDoor();
}

void UpdateACLTask::downloadACL()
{
  HTTPSRedirect client(httpsPort);
  DPRINT("Connecting to ");
  DPRINTLN(host);

  if(client.connectRedir(url, host, googleRedirHost))
  {
    DPRINTLN("Connected, reading stuff");
    File f = SPIFFS.open(aclFilename, "w");

    while(client.connected())
    {
      String line = client.readStringUntil('\n');
      
      int separatorIndex = line.indexOf('|');
      if(separatorIndex != -1)
      {
        String card = line.substring(separatorIndex + 1);
        card.trim();
        f.write((const uint8_t*)card.c_str(), card.length());
        f.write('\n');
      }
      
      DPRINTLN(line);
      if(line == "\r")
      {
        break;
      }
    }
    f.close();
  }
  else
  {
    DPRINTLN("Failed at connecting");
  }
  client.stop();
  DPRINTLN("Done with reading");
}


bool UpdateACLTask::validateCard(const String& card)
{
  bool validated = false;
  if(SPIFFS.exists(aclFilename))
  {
    File f = SPIFFS.open(aclFilename, "r");
    while(f.available())
    {
      String line = f.readStringUntil('\n');
      if(line == card)
      {
        Serial.println("Validated Card");
        latchTask.openDoor();
        validated = true;
        break;
      }
    }
    f.close();
  }
  return validated;
}

String UpdateACLTask::getACL()
{
  File f = SPIFFS.open(aclFilename, "r");
  String output;
  while(f.available())
  {
    String line = f.readStringUntil('\n');
    output += "\n";
    output += line;
  }
  return output;
}

