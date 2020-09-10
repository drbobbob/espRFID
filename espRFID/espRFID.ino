#include <Arduino.h>

#include <SoftwareSerial.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <FS.h>

#include <TaskScheduler.h>
#include <MyTasks.h>

#include "HTTPSRedirect.h"

#include "ReleaseButtonTask.h"
#include "DoorLatchTask.h"
#include "RFIDReaderTask.h"
#include "UpdateACLTask.h"
#include "ServerTask.h"

#include "SecretConstants.h"

// ESP8266 RFID reader

//Requirements:
// Button for release
// Serial read
// web page for seeing ACL
// Automated refresh of ACL (saved on file system)

//pin definitions
//pins with pullup: 4, 5, 12, 13, 14

const int RELEASE_BUTTON = 14;
const int RELAY_PIN = 5;
const int LED1_PIN = 13;
const int LED2_PIN = 15;
const int RFID_READ_PIN = 12;
const int RFID_WRITE_PIN = 4;

SoftwareSerial rfidSerial(RFID_READ_PIN, RFID_WRITE_PIN);

BlinkPatternTask external1LED(2, HIGH);
DoorLatchTask latchTask(RELAY_PIN);
UpdateACLTask aclTask(1000l*60*60*24, latchTask, external1LED);

RFIDReaderTask readerTask(rfidSerial, latchTask, aclTask);
ReleaseButtonTask releaseTask(RELEASE_BUTTON, latchTask, aclTask);
ServerTask serverTask(aclTask);

Task* tasks[] = {&external1LED, &releaseTask, &latchTask, &readerTask, &aclTask, &serverTask};

TaskScheduler ts(tasks, NUM_TASKS(tasks));

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("I'm booting");
  SPIFFS.begin();

}


void loop() {
  ts.runOnce();
}
