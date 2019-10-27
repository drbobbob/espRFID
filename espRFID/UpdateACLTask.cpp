/*
 * UpdateACLTask.cpp
 *      Author: DrBobBob
 */

#include "UpdateACLTask.h"
#include "Arduino.h"

#include "DoorLatchTask.h"
#include "BlinkPatternTask.h"
#include "SecretConstants.h"

#include <ESP8266HTTPClient.h>
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


const char* aclFilename = "acl";


UpdateACLTask::UpdateACLTask(uint32_t interval, DoorLatchTask& _latchTask, BlinkPatternTask& _blinkTask)
: TimedTask(0)
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
  if(downloadACL())
  {
    incRunTime(automatedInterval);
    nextAutomatedTime = runTime;
  }
}

void UpdateACLTask::manualUpdate()
{
  if(downloadACL())
  {
    setRunTime(nextAutomatedTime);
    latchTask.blinkDoor();
  }
}

bool UpdateACLTask::downloadACL()
{
  bool success = false;
  Serial.print("My update interval is ");
  Serial.println(automatedInterval);

  lastUpdateLog = "";
  Serial.println("Updating ACL");
  lastUpdateLog += "Updating ACL\n";

  HTTPClient http;
  http.begin(fullLoginURL, loginFingerprint);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // e.g.
  String msg = String("grant_type=password") + 
               "&client_id=" + clientID + 
               "&client_secret=" + clientSecret + 
               "&username=" + salesforceUsername + 
               "&password=" + salesforcePassword;

  int httpCode = http.POST(msg);
  if(httpCode == 200)
  {
    Serial.println("Login complete");
    lastUpdateLog += "Login complete\n";

    String loginResponse = http.getString();
    int tokenIndex = loginResponse.indexOf("access_token");
    int tokenStart = loginResponse.indexOf("\":\"", tokenIndex) + 3;
    int tokenEnd = loginResponse.indexOf('"', tokenStart);
  
    String token = loginResponse.substring(tokenStart, tokenEnd);
  
    HTTPClient http2;
    http2.begin(fullTagURL, tagFingerprint);
    http2.addHeader("Authorization", String("Bearer ") + token);
    int http2Code = http2.GET();
    if(http2Code == 200)
    {
      Serial.println("Tags acquired");
      lastUpdateLog += "Tags acquired\n";
      String tags = http2.getString();

      File f = SPIFFS.open(aclFilename, "w");

      int startTagIndex = 1;
      int endTagIndex = tags.indexOf('|');
      while(endTagIndex != -1)
      {
        String card = tags.substring(startTagIndex, endTagIndex);

        card.trim();
        f.write((const uint8_t*)card.c_str(), card.length());
        f.write('\n');

        startTagIndex = endTagIndex + 1;
        endTagIndex = tags.indexOf('|', startTagIndex);
      }
      blinkTask.setBlinkCounts(1,1);
      success = true;
    }
    else
    {
      Serial.println("Fail at tag get");
      lastUpdateLog += "Fail at tag get\n";
      blinkTask.setBlinkCounts(1,2);
    }
  }
  else
  {
    Serial.println("failed at auth");
    lastUpdateLog += "failed at auth\n";
    blinkTask.setBlinkCounts(1,3);
  }

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
