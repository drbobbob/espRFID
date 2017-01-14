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
// 

ESP8266WebServer server(80);

const int led = 13;

//pin definitions
//pins with pullup: 4, 5, 12, 13, 14

const int RELEASE_BUTTON = 4;
const int RELAY_PIN = 5;
const int LED1_PIN = 13;
const int LED2_PIN = 15;
const int RFID_READ_PIN = 14;
const int RFID_WRITE_PIN = 12;

SoftwareSerial rfidSerial(RFID_READ_PIN, RFID_WRITE_PIN);

BlinkTask external1LED(LED1_PIN, 500);
DoorLatchTask latchTask(RELAY_PIN);
RFIDReaderTask readerTask(rfidSerial, latchTask);
UpdateACLTask aclTask(1000l*60*60*24, latchTask);
ReleaseButtonTask releaseTask(RELEASE_BUTTON, latchTask, aclTask);
ServerTask serverTask(server);

void handleRoot() {
  digitalWrite(led, 1);

  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleACL()
{
  File f = SPIFFS.open("acl", "r");
  String output;
  output += "The ACL is:";
  while(f.available())
  {
    String line = f.readStringUntil('\n');
    output += "\n";
    output += line;
  }
  server.send(200, "text/plain", output);
}

void handleUpdateACL()
{
  aclTask.startShortManualTimer();
  server.send(200, "text/plain", "Update started, check /acl for the list");
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("doorESP")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/acl", handleACL);
  server.on("/updateAcl", handleUpdateACL);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  // put your main code here, to run repeatedly:

  SPIFFS.begin();

  Task* tasks[] = {&external1LED, &releaseTask, &latchTask, &readerTask, &aclTask, &serverTask};

  TaskScheduler ts(tasks, sizeof(tasks) / sizeof(tasks[0]));

  ts.run();
  
}
