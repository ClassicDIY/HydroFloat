#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "Log.h"
// #include "Defines.h"
#include "Tank.h"

using namespace HydroFloat;

// WebServer server(80);

Tank* _tank = new Tank();

// void handleRoot()
// {
//   logd("Handling Root URL");
//   String html = "<html><body>";
//   html += "<h1>ESP32 AP Mode</h1>";
//   html += "<form action='/submit' method='get'>";
//   html += "<label for='value'>Enter a numeric value:</label>";
//   html += "<input type='number' id='value' name='value'>";
//   html += "<input type='submit' value='Submit'>";
//   html += "</form>";
//   html += "<p><a href='update'>Firmware update</a></p>";
//   html += "</body></html>";
//   server.send(200, "text/html", html);
// }

// void handleUpdate() {
//   logd("Handling update URL");
//   String html = "<html><body>";
//   html += "<h1>ESP32 OTA Update</h1>";
//   html += "<form method='POST' action='/doupdate' enctype='multipart/form-data'>";
//   html += "<input type='file' name='firmware'>";
//   html += "<input type='submit' value='Update Firmware'>";
//   html += "</form>";
//   html += "</body></html>";
//   server.send(200, "text/html", html);
// }

// void handleNotFound() {
//   logd("Not found: %s", server.uri().c_str());
//   handleRoot();
// }

// void handleSubmit()
// {
//   if (server.hasArg("value"))
//   {
//     String value = server.arg("value");
//     logd("Received value: %s ", value.c_str());
//     server.send(200, "text/html", "Value received: " + value);
//   }
//   else
//   {
//     server.send(200, "text/html", "No value received");
//   }
// }
// void handleFirmwareUpload() {
//   HTTPUpload& upload = server.upload();

//   if (upload.status == UPLOAD_FILE_START) {
//     logd("Update: %s\n", upload.filename.c_str());
//     if (!Update.begin()) {
//       logd("Update.begin() failed");
//     }
//   } else if (upload.status == UPLOAD_FILE_WRITE) {
//     if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
//       logd("Update.write() failed");
//     }
//   } else if (upload.status == UPLOAD_FILE_END) {
//     if (Update.end(true)) {
//       Serial.printf("Update Success: %u bytes\n", upload.totalSize);
//     } else {
//       logd("Update.end() failed");
//     }
//   }

// }

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }

  // server.onNotFound(handleNotFound);
  // server.on("/", HTTP_GET, handleRoot);
  // server.on("/submit", handleSubmit);
  // server.on("/update", handleUpdate);

  // server.on("/doupdate", HTTP_POST, []() {
  //   server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  //   ESP.restart();
  // }, handleFirmwareUpload);
  // server.begin();
  
  _tank->setup();
  logd("HTTP server started");
}

void loop()
{
  // dnsServer.processNextRequest();
  // server.handleClient();
  _tank->Process();
}