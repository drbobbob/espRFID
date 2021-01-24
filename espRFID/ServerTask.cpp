/*
 * ServerTask.cpp
 *      Author: DrBobBob
 */

#include "ServerTask.h"
#include "Arduino.h"
#include "SecretConstants.h"
#include "UpdateACLTask.h"
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include <functional>


void ServerTask::handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!");
}

void ServerTask::handleACL()
{
  String output;
  output += "The ACL is:";
  output += aclTask.getACL();
  server.send(200, "text/plain", output);
}

void ServerTask::handleACLLog()
{
  server.send(200, "text/plain", aclTask.getACLLog());
}

void ServerTask::handleUpdateACL()
{
  aclTask.startShortManualTimer();
  server.send(200, "text/plain", "Update started, check /acl for the list");
}

void ServerTask::handleNotFound(){
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
}

ServerTask::ServerTask(UpdateACLTask& _aclTask)
: TimedTask(millis())
, aclTask(_aclTask)
, server(80)
, CurrentState(SERVER_START)
{


}

ServerTask::~ServerTask() {
}

void ServerTask::run(uint32_t now)
{
  switch(CurrentState)
  {
    case SERVER_START:
      StartState(now);
      break;
    case SERVER_WAIT_FOR_CONNECTION:
      WaitForConnectionState(now);
      break;
    case SERVER_RUNNING:
      RunningState(now);
      break;
  }
}

void ServerTask::StartState(uint32_t now)
{
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wifi already connected");
  }
  else
  {
    Serial.println("Starting Wifi connection");
    WiFi.begin(ssid, password);
  }
  CurrentState = SERVER_WAIT_FOR_CONNECTION;
  setRunTime(now + 500);
}

void ServerTask::WaitForConnectionState(uint32_t now)
{
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(mdnsName)) {
      Serial.println("MDNS responder started");
    }

    server.on("/", std::bind(&ServerTask::handleRoot, this));
    server.on("/acl", std::bind(&ServerTask::handleACL, this));
    server.on("/updateAcl", std::bind(&ServerTask::handleUpdateACL, this));
    server.on("/aclLog", std::bind(&ServerTask::handleACLLog, this));

    server.onNotFound(std::bind(&ServerTask::handleNotFound, this));

    server.begin();
    Serial.println("HTTP server started");

    MDNS.addService("http", "tcp", 80);

    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    
    CurrentState = SERVER_RUNNING;
    setRunTime(now - 1);
  }
  else
  {
    incRunTime(500);
    yield();
  }
}

void ServerTask::RunningState(uint32_t now)
{
  server.handleClient();
  yield();
  setRunTime(now - 1);
}
