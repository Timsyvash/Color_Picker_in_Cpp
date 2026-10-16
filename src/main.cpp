#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

ESP8266WebServer server(80);

const char *ssid = "";
const char *password = "";

unsigned long last_time_serial = 0;

uint8_t REDpin = D5;
uint8_t BLUEpin = D6;
uint8_t GREENpin = D7;

short red = 0;
short blue = 0;
short green = 0;

String getContentType(String filename);
bool handleFileRead(String path);

void setup()
{
  pinMode(BLUEpin, OUTPUT);
  pinMode(REDpin, OUTPUT);
  pinMode(GREENpin, OUTPUT);

  Serial.begin(115200);
  Serial.println();

  // Підключення до домашнього роутера
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s ", ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    if (millis() - last_time_serial >= 500)
    {
      last_time_serial = millis();
      Serial.print(".");
    }
  }
  Serial.println(" connected!");

  IPAddress myIP = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(myIP);

  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  server.on("/", []()
            { handleFileRead("/index.html"); });
  server.onNotFound([]()
                    {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found"); });

  server.begin();
  Serial.println("Web server started successfully!");

  analogWrite(REDpin, 0);
  analogWrite(GREENpin, 0);
  analogWrite(BLUEpin, 0);
}

void loop()
{
  server.handleClient();

  if (server.hasArg("red"))
    red = server.arg("red").toInt();
  if (server.hasArg("blue"))
    blue = server.arg("blue").toInt();
  if (server.hasArg("green"))
    green = server.arg("green").toInt();

  analogWrite(REDpin, red);
  analogWrite(BLUEpin, blue);
  analogWrite(GREENpin, green);
}

String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path)
{
  if (path.endsWith("/"))
    path += "index.html";
  String contentType = getContentType(path);

  if (LittleFS.exists(path))
  {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }

  Serial.println("\tFile Not Found on LittleFS");
  return false;
}
