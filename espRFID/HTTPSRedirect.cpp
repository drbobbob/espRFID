/*  HTTPS with follow-redirect
 *  Created by Sujay S. Phadke, 2016
 *  All rights reserved.
 *
 */
// to-do: Use 'F' macro for storing strings
// to-do: remove spurious characters in the message body

#include "HTTPSRedirect.h"

// from LarryD, Arduino forum
//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

HTTPSRedirect::HTTPSRedirect(const int p, const char* fp, bool c)
    : httpsPort(p), redirFingerprint(fp), fpCheck(c){
}

HTTPSRedirect::HTTPSRedirect(const int p)
    : httpsPort(p){
      fpCheck = false;
}

HTTPSRedirect::~HTTPSRedirect(){
}

bool HTTPSRedirect::connectRedir(String& url, const char* host, const char* redirHost){
  return connectRedir(url.c_str(), host, redirHost);
}

bool HTTPSRedirect::connectRedir(const char* url, const char* host, const char* redirHost){

  int redirFlag = false;

  int retryCount = 10;
  while(retryCount > 0 && !connect(host, httpsPort))
  {
    --retryCount;
  }

  // Check if connection to host is alive
  if (!connected()){
    DPRINTLN("Error! Not connected to host.");
    return false;
  }
  // HTTP headers must be terminated with a "\r\n\r\n"
  // http://stackoverflow.com/questions/6686261/what-at-the-bare-minimum-is-required-for-an-http-request
  // http://serverfault.com/questions/163511/what-is-the-mandatory-information-a-http-request-header-must-contain
  String Request = createRequest(url, host);

  DPRINTLN(Request);
  // make request
  print(Request);

  String line;
  String redirUrl;

  DPRINTLN("Detecting re-direction.");
  DPRINTLN(redirHost);

  String httpCodeStr = readStringUntil(' '); // Discard the "HTTP/1.1" string
  httpCodeStr = readStringUntil(' ');
  int httpCodeInt = httpCodeStr.toInt();

  DPRINT("HTTP code return is ");
  DPRINTLN(httpCodeInt);

  if(httpCodeInt >= 300 && httpCodeInt < 400)
  {
    while (connected()) {
      line = readStringUntil('\n');
      DPRINTLN("Read a line");
      DPRINTLN(line);
      if (line == "\r") {
        DPRINTLN("END OF HEADER");
        //DPRINTLN(line);
        break;
      }

      if (find("Location: ") ){
        find((char *)redirHost);
        DPRINTLN("Found re-direction URL!");
        redirUrl = readStringUntil('\n');
        redirFlag = true;
        break;
      }
      /*
      if (finder.findUntil("chunked", "\n\r") ){
        break;
      }*/
    }

    DPRINTLN("Body:\n");
    if (verboseInfo)
      fetchData(true, false);
    else
      flush();

    if (!redirFlag){
      DPRINTLN("No re-direction URL found in header.");
      return false;
    }
  }
  else
  {
    return false;
  }

  DPRINTLN("Redirected URL:");
  DPRINTLN(redirUrl);

  Request = createRequest(redirUrl.c_str(), redirHost);

  DPRINTLN("Connecting to:");
  DPRINTLN(redirHost);

  if (!connect(redirHost, httpsPort)) {
    DPRINTLN("Connection to re-directed host failed!");
    return false;
  }

  if (fpCheck){
    if (verify(redirFingerprint, redirHost)) {
      DPRINTLN("Re-directed host certificate match.");
    } else {
      DPRINTLN("Re-directed host certificate mis-match");
    }
  }

  DPRINTLN("Requesting re-directed URL.");
  DPRINTLN(Request);

  // Make request
  print(Request);

  DPRINTLN("Final Response:");

  fetchData(false, true);
  DPRINTLN("Header done");

  return true;
}

String HTTPSRedirect::createRequest(const char* url, const char* host){
  return String("GET ") + url + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" +
                          "User-Agent: ESP8266\r\n" +
                          (keepAlive ? "" : "Connection: close") +
                          "\r\n\r\n";

}

void HTTPSRedirect::fetchData(bool disp, bool header){
  String line;
  while (connected()) {
    line = readStringUntil('\n');

    if (disp)
      Serial.println(line);

    if (line == "\r") {
      if (disp){
        if (header)
          DPRINTLN("END OF HEADER");
        else
          DPRINTLN("END OF RESPONSE");
          //DPRINTLN(line);
      }
      break;
    }
  }
}
