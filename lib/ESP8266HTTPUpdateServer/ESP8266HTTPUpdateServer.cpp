#include <Arduino.h>
#include <Update.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <LittleFS.h>
#include "StreamString.h"
#include "ESP8266HTTPUpdateServer.h"
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include "..\..\src\hwdefs.h"


// taken from here and modified by buzz to add spiffs upload
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPUpdateServer/src/ESP8266HTTPUpdateServer.cpp


/*static const char serverIndex[] PROGMEM =
  R"(<html><body><div id=blah></div><form method='POST' action='' enctype='multipart/form-data'>
                  <input type='file' name='update'>
                  <input type='submit' value='Update'>
               </form>
         </body></html>)";
*/


static const char serverIndex[] PROGMEM =
  R"(<html><body>
     <form method='POST' action='' enctype='multipart/form-data'>
     Firmware:<br>
                  <input type='file' name='firmware'>
                  <input type='submit' value='Update Firmware'>
               </form>
     <form method='POST' action='' enctype='multipart/form-data'>
     Spiffs:<br>
                  <input type='file' name='spiffs'>
                  <input type='submit' value='Update LITTLEFS'>
               </form>
     </body></html>)";


static const char successResponse[] PROGMEM = 
  "<META http-equiv=\"refresh\" content=\"15;URL=/\">Update Success! Rebooting...<br>\nThis may take up-to a full MINUTE to come back on, so please be patient as it does this.<br>\n";

ESP8266HTTPUpdateServer::ESP8266HTTPUpdateServer(bool serial_debug)
{
  _serial_output = serial_debug;
  _server = NULL;
  _username = NULL;
  _password = NULL;
  _authenticated = false;
}

void ESP8266HTTPUpdateServer::setup(ESP8266WebServer *server, const char * path, const char * username, const char * password)
{
    _server = server;
    _username = (char *)username;
    _password = (char *)password;

    // handler for the /update form page
    _server->on(path, HTTP_GET, [&](){
      if(_username != NULL && _password != NULL && !_server->authenticate(_username, _password))
        return _server->requestAuthentication();
      _server->send_P(200, PSTR("text/html"), serverIndex);
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on(path, HTTP_POST, [&](){
      if(!_authenticated)
        return _server->requestAuthentication();
      if (Update.hasError()) {
        _server->send(200, F("text/html"), String(F("Update error: ")) + _updaterError);
      } else {
        _server->client().setNoDelay(true);
        _server->sendHeader("Location","/success.htm");      // Redirect the client to the success page
        _server->send(303);
        //_server->send_P(200, PSTR("text/html"), successResponse);
        delay(100);
        _server->client().stop();
        //ESP.restart();
      }
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = _server->upload();

      if(upload.status == UPLOAD_FILE_START){
        _updaterError = String();

        _authenticated = (_username == NULL || _password == NULL || _server->authenticate(_username, _password));
        if(!_authenticated){
          if (_serial_output)
            //dbgSer.printf("Unauthenticated Update\n");
          return;
        }

        WiFiUDP udp;
        udp.stop();
        //if (_serial_output)
         // dbgSer.printf("Update: %s\n", upload.filename.c_str());
//        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
//        if(!Update.begin(maxSketchSpace)){//start with max available size
//          _setUpdaterError();
//        }

           if (upload.name == F("spiffs")) {
              size_t spiffsSize = LittleFS.totalBytes();
              if (!Update.begin(spiffsSize, U_SPIFFS)){
                if (_serial_output) Update.printError(dbgSer);
              }
            } else {
              uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
              if (!Update.begin(maxSketchSpace, U_FLASH)){//start with max available size
                _setUpdaterError();
              }
            }

//
      } else if(_authenticated && upload.status == UPLOAD_FILE_WRITE && !_updaterError.length()){
        //if (_serial_output) dbgSer.printf(".");
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          _setUpdaterError();
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_END && !_updaterError.length()){
        if(Update.end(true)){ //true to set the size to the current progress
          //if (_serial_output) dbgSer.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          _setUpdaterError();
        }
      } else if(_authenticated && upload.status == UPLOAD_FILE_ABORTED){
        Update.end();
        //if (_serial_output) dbgSer.println("Update was aborted");
      }
      delay(0);
    });
}

void ESP8266HTTPUpdateServer::_setUpdaterError()
{
  if (_serial_output) Update.printError(dbgSer);
  StreamString str;
  Update.printError(str);
  _updaterError = str.c_str();
}
