/*
 * UpdateACLTask.cpp
 *      Author: DrBobBob
 */

#include "UpdateACLTask.h"
#include "Arduino.h"

#include "DoorLatchTask.h"
#include "BlinkPatternTask.h"
#include "HTTPSRedirect.h"
#include "SecretConstants.h"

#include <FS.h>

const int BUTTON_HOLD_TIME = 14000;

// #define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
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


UpdateACLTask::UpdateACLTask(uint32_t interval, DoorLatchTask& _latchTask, BlinkPatternTask& _blinkTask)
: TimedTask(10000)
, retryCount(5)
, nextRetryInterval(10000)
, nextAutomatedTime(interval)
, automatedInterval(interval)
, lastUpdateLog()
, latchTask(_latchTask)
, blinkTask(_blinkTask)
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
  if(downloadACL() || retryCount == 0)
  {
    incRunTime(automatedInterval);
    nextAutomatedTime = runTime;
    retryCount = 5;
  }
  else
  {
    incRunTime(nextRetryInterval);
    nextAutomatedTime = runTime;
    retryCount--;
  }

}

void UpdateACLTask::manualUpdate()
{
  if(downloadACL())
  {
    setRunTime(nextAutomatedTime);
    latchTask.blinkDoor();
  }
  else
  {
    incRunTime(nextRetryInterval); 
    nextAutomatedTime = runTime;
    retryCount--;
  }
}

bool UpdateACLTask::downloadACL()
{
  bool success = false;
  Serial.print("My update interval is ");
  Serial.println(automatedInterval);

  lastUpdateLog = "";
  Serial.println("Updating ACL");
  HTTPSRedirect client(httpsPort);
  DPRINT("Connecting to ");
  DPRINTLN(host);
  lastUpdateLog += "Updating ACL\n";

  if(client.connectRedir(url, host, googleRedirHost))
  {
    Serial.println("Connected, reading stuff");
    DPRINTLN("Connected, reading stuff");
    lastUpdateLog += "Connected, reading stuff\n";
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

      lastUpdateLog += "Read Line: ";
      lastUpdateLog += line;
      lastUpdateLog += '\n';
      DPRINTLN(line);
      if(line == "\r")
      {
        success = true;
        blinkTask.setBlinkCounts(1,1);
        break;
      }
    }
    f.close();
  }
  else
  {
    blinkTask.setBlinkCounts(1,2);
    lastUpdateLog += "Failed at connecting";
    DPRINTLN("Failed at connecting");
  }
  client.stop();
  DPRINTLN("Done with reading");
  lastUpdateLog += "Done with reading\n";
  return success;
}


bool UpdateACLTask::validateCard(const String& card)
{
  String shortCard = card.substring(0, 10);
  bool validated = false;
  if(SPIFFS.exists(aclFilename))
  {
    File f = SPIFFS.open(aclFilename, "r");
    while(f.available())
    {
      String line = f.readStringUntil('\n');
      if(line == shortCard)
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

String UpdateACLTask::getACLLog()
{
  return lastUpdateLog;
}
